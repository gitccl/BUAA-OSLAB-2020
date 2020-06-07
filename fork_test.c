#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
int main(void){
	int var;
	pid_t pid; 
	printf("Before fork.\n");
	pid = fork();
	printf("After fork.\n");
	if(pid==0){
		printf("son.");
	}else{
		sleep(2);
		printf("father.");
	}
	printf("pid:%d\n",getpid());
	return 0;
}