#include <env.h>
#include <pmap.h>
#include <printf.h>

/* Overview:
 *  Implement simple round-robin scheduling.
 *  Search through 'envs' for a runnable environment ,
 *  in circular fashion statrting after the previously running env,
 *  and switch to the first such environment found.
 *
 * Hints:
 *  The variable which is for counting should be defined as 'static'.
 */
extern struct Env_list env_sched_list[2];
extern struct Env *curenv;
extern char *KERNEL_SP;
void sched_yield(void)
{
    //struct Env *e;
    static struct Env *cur = NULL;
    static int num = 0;// 当前正在遍历的链表
    static int count = 0;// 当前进程剩余执行次数
    int IsChange = 0;   //Lab6需要用到，其他不同管
    //printf("继续调用sched.c\n");
    while( (!cur) || count <= 0 || (cur && cur -> env_status != ENV_RUNNABLE)) {
        count = 0;
        if(cur != NULL) {
            LIST_REMOVE(cur, env_sched_link);
            if(cur -> env_status != ENV_FREE)
                LIST_INSERT_HEAD(&env_sched_list[num^1], cur, env_sched_link);
        }
        if(LIST_EMPTY(&env_sched_list[num])) num ^= 1;
        if(LIST_EMPTY(&env_sched_list[num])) {
            //printf("\nsched.c/sched_yield:NO envs to run\n");
            continue ;
        }
        cur = LIST_FIRST(&env_sched_list[num]);
        if(cur != NULL) count = cur -> env_pri;
        IsChange = 1;
        //printf("%x %d\n", cur, cur -> env_status);
    }
    if(IsChange) cur -> env_runs ++ ;   //Lab6
    -- count;
    env_run(cur);
}
