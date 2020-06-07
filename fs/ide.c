/*
 * operations on IDE disk.
 */

#include "fs.h"
#include "lib.h"
#include <mmu.h>

// Overview:
// 	read data from IDE disk. First issue a read request through
// 	disk register and then copy data from disk buffer
// 	(512 bytes, a sector) to destination array.
//
// Parameters:
//	diskno: disk number.
// 	secno: start sector number.
// 	dst: destination for data read from IDE disk.
// 	nsecs: the number of sectors to read.
//
// Post-Condition:
// 	If error occurred during read the IDE disk, panic. 
// 	
// Hint: use syscalls to access device registers and buffers
void
ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs)
{
	// 0x200: the size of a sector: 512 bytes.
	int offset_begin = secno * 0x200;
	int offset_end = offset_begin + nsecs * 0x200;
	int offset = 0, curoffset;
	while (offset_begin + offset < offset_end) {
		curoffset = offset_begin + offset;
		/* 1. 选择磁盘号, 即是往地址写值, 注意是4个字节 */
		if( syscall_write_dev(&diskno, 0x13000010, 4) < 0) {
			user_panic("Error in ide.c/ide_read 1\n");
		}
		/* 2. 设置磁盘读取位置的偏移offset */
		if( syscall_write_dev(&curoffset, 0x13000000, 4) < 0) {
			user_panic("Error in ide.c/ide_read 2\n");
		}
		/* 3. 写入0，表示开始读取, 注意是一个字节 */
		//char tmp = 0;
		int temp = 0;
		if( syscall_write_dev(&temp, 0x13000020, 4) < 0) {
			user_panic("Error in ide.c/ide_read 3\n");
		}
		/* 4. 读取磁盘操作结果*/
		int val;
		if( syscall_read_dev(&val, 0x13000030, 4) < 0){
			user_panic("Error in ide.c/ide_read 4\n");
		}
		if( val == 0 ) {//失败了
			user_panic("Error in ide.c/ide_read 4 val = 0\n");
		}
		/*5. 读出缓冲区里的内容 */
		if( syscall_read_dev(dst, 0x13004000, 512) < 0){
			user_panic("Error in ide.c/ide_read 5\n");
		}
		offset += 512;
		dst += 512;
		//offset_begin += 512;
	}
}


// Overview:
// 	write data to IDE disk.
//
// Parameters:
//	diskno: disk number.
//	secno: start sector number.
// 	src: the source data to write into IDE disk.
//	nsecs: the number of sectors to write.
//
// Post-Condition:
//	If error occurred during read the IDE disk, panic.
//	
// Hint: use syscalls to access device registers and buffers
void
ide_write(u_int diskno, u_int secno, void *src, u_int nsecs)
{
    // Your code here
	int offset_begin = secno * 512;
	int offset_end = offset_begin + nsecs*512;
	// int offset = ;
	writef("diskno: %d\n", diskno);
	while (offset_begin <  offset_end) {
		/* 1. 将数据写入缓冲区*/
		if(syscall_write_dev(src, 0x13004000, 512) < 0) {
			user_panic("Error in ide.c/ide_write 1\n");
		}
		/* 2. 选择磁盘号*/
		if(syscall_write_dev(&diskno, 0x13000010, 4) < 0) {
			user_panic("Error in ide.c/ide_write 2\n");
		} 
		/* 3. 设置磁盘写入位置的偏移 */
		if(syscall_write_dev(&offset_begin, 0x13000000, 4) < 0) {
			user_panic("Error in ide.c/ide_write 3\n");
		}
		/* 4. 写入1, 表示开始把缓冲区内容写入磁盘 */
		//char tmp = 1;
		int temp = 1;
		if(syscall_write_dev(&temp, 0x13000020, 4) < 0) {
			user_panic("Error in ide.c/ide_write 4\n");
		}
		/* 5. 读取磁盘操作结果 */
		int val;
		if( syscall_read_dev(&val, 0x13000030, 4) < 0 ){
			user_panic("Error in ide.c/ide_write 5\n");
		}
		if(val == 0) {
			user_panic("Error in ide.c/ide_write 5 val = 0\n");
		}
		src += 512;
		offset_begin += 512;
	}
}

