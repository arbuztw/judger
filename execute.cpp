#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include "judger.h"
#define OUTPUT_FILE "output.txt"

int pid;
int valid_syscall[1024];

void getSyscallTable()
{
	FILE *f = fopen("syscall.txt", "r");
	int i;
	for (i = 0; i < 342; i++)
		fscanf(f, "%d", &valid_syscall[i]);
	fclose(f);
}

int checkSyscall()
{
	struct user_regs_struct regs;
	int syscall;
	ptrace(PTRACE_GETREGS, pid, NULL, &regs);
	syscall = regs.orig_eax;
	//printf("%d\n", valid_syscall[syscall]);
	/*if (!valid_syscall[syscall])
		return RET_RF;*/
/*	else
	{
			 
	}
*/
	return regs.orig_eax;
}

int getMemory() //KBytes
{
	char buf[1024];
	sprintf(buf, "/proc/%d/statm", pid);
	FILE *f = fopen(buf, "r");
	int r;
	for (int i = 0; i < 6; i++)
		fscanf(f, "%d", &r);
	fclose(f);
	return r * getpagesize() / 1024;
}

int main(int argc, char *argv[])
{
	int tdcount, tlimit, mlimit;
	char exename[1024], inputfile[1024];
	struct rlimit r;
	
	if (argc < 6)
	{
		printf("Usage: [id] [probid] [input] [time limit] [memory limit]\n");
		exit(RET_SE);
	}

	tlimit = atoi(argv[4]);
	mlimit = atoi(argv[5]);

	sprintf(exename, "./%s", argv[1]);
	strcpy(inputfile, argv[3]);

	getSyscallTable();

	if ((pid = fork()) == 0)
	{
		//signal(SIGXCPU, func);
		chdir("chroot");
		r.rlim_cur = r.rlim_max = 3;
		setrlimit(RLIMIT_CPU, &r);
		setuid(99);
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execl(exename, exename, NULL);
	}
	else
	{
		int stat, maxmem = 0, tmpmem;
		struct rusage rinfo;
		//struct user_regs_struct regs;
		for (;;)
		{
			wait4(pid, &stat, 0, &rinfo);
			if (WIFEXITED(stat))
			{
				//puts("Exited!");
				break;
			}
			else if (WIFSTOPPED(stat))
			{
				switch (WSTOPSIG(stat))
				{
					case SIGTRAP:
						checkSyscall();
						break;
				}
			}
			tmpmem = getMemory();
			if (tmpmem > maxmem) maxmem = tmpmem;
			ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
		}
		printf("Memory: %d KB\n", maxmem);
	}

	
	return 0;
}
