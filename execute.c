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
#include "syscall.h"
#define OUTPUT_FILE "output.txt"

int pid;

int checkSyscall()
{
	struct user_regs_struct regs;
	int syscall;
	ptrace(PTRACE_GETREGS, pid, NULL, &regs);
	#ifdef __i386__
	syscall = regs.orig_eax;
	#else
	syscall = regs.orig_rax;
	#endif

	//printf("syscall: %d\n", syscall);

	if (avail_syscall[syscall] == 0)
	{
		printf("Invalid syscall: %d\n", syscall);
		return RET_RF;
	}
	else if (avail_syscall[syscall] > 0)
		avail_syscall[syscall]--;
	return RET_AC;
}

int getMemory() //KBytes
{
	char buf[1024];
	sprintf(buf, "/proc/%d/statm", pid);
	FILE *f = fopen(buf, "r");
	int r, i;
	for (i = 0; i < 6; i++)
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


	if ((pid = fork()) == 0)
	{
		freopen("input.txt", "r", stdin);
		chdir("sandbox");
		chroot(".");
		freopen("output.txt", "w", stdout);
		r.rlim_cur = r.rlim_max = 3;
		setrlimit(RLIMIT_CPU, &r);
		setregid(99, 99);
		setreuid(99, 99);
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execl(exename, exename, NULL);
	}
	else
	{
		int stat, maxmem = 0, tmpmem;
		struct rusage rinfo;
		for (;;)
		{
			wait4(pid, &stat, 0, &rinfo);
			if (WIFEXITED(stat))
				break;
			else if (WIFSTOPPED(stat))
			{
				switch (WSTOPSIG(stat))
				{
					case SIGTRAP:
						if (checkSyscall() == RET_RF)
						{
							ptrace(PTRACE_KILL, pid, NULL, NULL);
							exit(RET_RF);
						}
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
