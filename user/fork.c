// implement fork from user space

#include "lib.h"
#include <mmu.h>
#include <env.h>


/* ----------------- help functions ---------------- */

/* Overview:
 * 	Copy `len` bytes from `src` to `dst`.
 *
 * Pre-Condition:
 * 	`src` and `dst` can't be NULL. Also, the `src` area 
 * 	 shouldn't overlap the `dest`, otherwise the behavior of this 
 * 	 function is undefined.
 */
void user_bcopy(const void *src, void *dst, size_t len)
{
	void *max;

	//writef("~~~~~~~~~~~~~~~~ src:%x dst:%x len:%x\n",(int)src,(int)dst,len);
	max = dst + len;

	// copy machine words while possible
	if (((int)src % 4 == 0) && ((int)dst % 4 == 0)) {
		while (dst + 3 < max) {
			*(int *)dst = *(int *)src;
			dst += 4;
			src += 4;
		}
	}

	// finish remaining 0-3 bytes
	while (dst < max) {
		*(char *)dst = *(char *)src;
		dst += 1;
		src += 1;
	}

	//for(;;);
}

/* Overview:
 * 	Sets the first n bytes of the block of memory 
 * pointed by `v` to zero.
 * 
 * Pre-Condition:
 * 	`v` must be valid.
 *
 * Post-Condition:
 * 	the content of the space(from `v` to `v`+ n) 
 * will be set to zero.
 */
void user_bzero(void *v, u_int n)
{
	char *p;
	int m;

	p = v;
	m = n;

	while (--m >= 0) {
		*p++ = 0;
	}
}
/*--------------------------------------------------------------*/

/* Overview:
 * 	Custom page fault handler - if faulting page is copy-on-write,
 * map in our own private writable copy.
 * 
 * Pre-Condition:
 * 	`va` is the address which leads to a TLBS exception.
 *
 * Post-Condition:
 *  Launch a user_panic if `va` is not a copy-on-write page.
 * Otherwise, this handler should map a private writable copy of 
 * the faulting page at correct address.
 */
static void
pgfault(u_int va)
{
	u_int *tmp;
	//	writef("fork.c:pgfault():\t va:%x\n",va);
	va = ROUNDDOWN(va, BY2PG);
	tmp = (u_int *)USTACKTOP;// 设置临时地址，这个地址开始是invalid memory。这个地址好像没用过？
	/* 首先要判断是写时复制页面 */
	
    u_int perm = (*vpt)[VPN(va)] & 0xfff;
	//u_int srcid = syscall_getenvid();
	if(!(perm & PTE_COW)) {
		user_panic("Error at fork.c/pgfault. perm has no PTE_COW\n"); 
		return ;
	}
	//map the new page at a temporary place
	/* 分配一个新的页面到临时位置tmp */
	if( syscall_mem_alloc(0, tmp, PTE_V | PTE_R) < 0 ) {
		user_panic("Error at fork.c/pgfault. syscall_mem_alloc failed\n");
		return ;
	}

	//copy the content
	/* 将要复制的内容拷贝到刚分配的页(临时位置) */ 
	user_bcopy((void *)va, (void *)tmp, BY2PG);

    //map the page on the appropriate place
	/*让va也映射到刚刚分配的页上 */
	if( syscall_mem_map(0, tmp, 0, va, PTE_V | PTE_R) < 0){
		user_panic("Error at fork.c/pgfault. syscall_mem_map failed\n");
		return ;
	}
	

    //unmap the temporary place 
	/* 取消临时位置的映射 */
	if( syscall_mem_unmap(0, tmp) < 0){
		user_panic("Error at fork.c/pgfault. syscall_mem_unmap failed\n");
		return ;
	}
}

/* Overview:
 * 	Map our virtual page `pn` (address pn*BY2PG) into the target `envid`
 * at the same virtual address. 
 *
 * Post-Condition:
 *  if the page is writable or copy-on-write, the new mapping must be 
 * created copy on write and then our mapping must be marked 
 * copy on write as well. In another word, both of the new mapping and
 * our mapping should be copy-on-write if the page is writable or 
 * copy-on-write.
 * 
 * Hint:
 * 	PTE_LIBRARY indicates that the page is shared between processes.
 * A page with PTE_LIBRARY may have PTE_R at the same time. You
 * should process it correctly.
 */
static void
duppage(u_int envid, u_int pn)
{
	/*target 是 envid*/
	u_int addr;
	u_int perm;
	addr = pn * BY2PG;
	perm = (*vpt)[pn] & 0xfff;// 取出权限位
	u_int srcid = syscall_getenvid();	
	if(!(perm & PTE_R) || !(perm & PTE_V)) {
		/* 只读页面或者无效页面,权限不变 */
		int ret = syscall_mem_map(0, addr, envid, addr, perm);
        if(ret < 0) {
            user_panic("In fork.c/duppage. syscall_mem_map failed on 只读页面或者无效页面,权限不变\n");
        }
	}else if(perm&PTE_LIBRARY){
		/* 共享页面，保持共享 */
		int ret = syscall_mem_map(0, addr, envid, addr, perm);
        if(ret < 0) {
            user_panic("In fork.c/duppage. syscall_mem_map failed on 共享页面，保持共享\n");
        }
	}else if(perm&PTE_COW){
		/* 非共享页面，写时复制页面 */
		int ret = syscall_mem_map(0, addr, envid, addr, perm);
        if(ret < 0) {
            user_panic("In fork.c/duppage. syscall_mem_map failed on 非共享页面，写时复制页面\n");
        }
	}else{
		/*可读页面，父进程和子进程都要设置写时复制*/
		perm |= PTE_COW;
		int ret = syscall_mem_map(0, addr, envid, addr, perm); // 子进程
        if(ret < 0) {
            user_panic("In fork.c/duppage. syscall_mem_map failed on 可读页面, 子进程要设置写时复制\n");
        }
		ret = syscall_mem_map(0, addr, 0, addr, perm); // 父进程
        if(ret < 0) {
            user_panic("In fork.c/duppage. syscall_mem_map failed on 可读页面, 父进程要设置写时复制\n");
        }
	}
    //writef("duppage success!\n");
	//	user_panic("duppage not implemented");
}

/* Overview:
 * 	User-level fork. Create a child and then copy our address space
 * and page fault handler setup to the child.
 *
 * Hint: use vpd, vpt, and duppage.
 * Hint: remember to fix "env" in the child process!
 * Note: `set_pgfault_handler`(user/pgfault.c) is different from 
 *       `syscall_set_pgfault_handler`. 
 */
extern void __asm_pgfault_handler(void);
int
fork(void)
{
    //writef("Start fork.c/fork\n");
	// Your code here.
	/* 只会被父进程执行的部分从这里开始 */
	u_int newenvid;
	extern struct Env *envs;
	extern struct Env *env;
	u_int i;
	u_int parent_id = syscall_getenvid();
    
    //writef("In fork.c/fork, getenvid(parent_id): %d success!\n", parent_id);
	//The parent installs pgfault using set_pgfault_handler
	/* 父进程为自身分配了异常处理栈 */
	set_pgfault_handler(pgfault);
	//writef("In fork.c/fork, set_pgfault_handler for father success!\n");
	//alloc a new alloc
	newenvid = syscall_env_alloc(); // 获取子进程的envid
	/* 只会被父进程执行的部分结束，接下来的代码父子进程会共同执行*/
    //writef("In fork.c/fork, 获取子进程的envid success! envid : %d\n", newenvid);
	u_int envid;
	if(newenvid == 0) {
		/*这个只有子进程会进入这个if语句来执行*/
        //writef("******Start. This is Son's env**********\n");
		envid = syscall_getenvid(); //通过系统调用来获取envid
        //writef("******通过系统调用来获取自己的envid: %d ENVX(envid): %d **********\n", envid, ENVX(envid));
        /*为啥无法使用envid2env*/
        env = &envs[ENVX(envid)];
        //user_panic("******通过系统调用来获取自己的envid: **********\n");
		env -> env_parent_id = parent_id; //刚刚设置的parent_id
        //writef("******End. This is Son's env**********\n");
		return 0;
	}
    //writef("***********Start fork.c/fork Father's env**********\n");
	/*这个newenvid 是子进程的envid*/
	/*1. 遍历父进程地址空间，进行 duppage*/
    //writef("Start duppage\n");
	for( i = 0; i < VPN(USTACKTOP) ; ++ i )
	{
		if( ((*vpd)[i >> 10]) && ((*vpt)[i]) ) {
			duppage(newenvid, i);
		}
	}
    //writef("End duppage\n");
	//The parent installs pgfault using set_pgfault_handler
	/*2. 为子进程分配异常处理栈(异常处理栈每个进程都要有单独的) */
    //writef("为子进程分配异常处理栈\n");
	if (syscall_mem_alloc(newenvid, UXSTACKTOP - BY2PG, PTE_V | PTE_R) < 0 ) {
		writef("Error at fork.c/fork. syscall_mem_alloc for Son_env failed\n");
		return -1;
	}
    //writef("为子进程分配异常处理栈结束\n");
	/* 3. 设置子进程的处理函数，确保缺页中断可以正常执行。*/
    //writef("设置子进程的处理函数\n");
	if(syscall_set_pgfault_handler(newenvid, __asm_pgfault_handler, UXSTACKTOP) < 0) {
		writef("Error at fork.c/fork. syscall_set_pgfault_handler for Son_env failed\n");
		return -1;
	}
    //writef("设置子进程的处理函数结束\n");
	/*注意在开始的地方，父进程已经调用set_pgfault_handler,在为自身分配异常处理栈之后
	 * 同时也设置了 __pgfault_handler = pgfault。 子进程保留了父进程的大部分内容。因此子进程不需要再次设置。
	 */
	/* 4.设置子进程的运行状态 */
	syscall_set_env_status(newenvid, ENV_RUNNABLE);
    //writef("End fork.c/fork Father's env\n");
	return newenvid;//返回子进程的envid
}

// Challenge!
int
sfork(void)
{
	user_panic("sfork not implemented");
	return -E_INVAL;
}
