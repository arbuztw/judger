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
int maxmem;
struct rusage rinfo;

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

int getRuntime() //MS
{
	double r;
	r = (rinfo.ru_utime.tv_sec + rinfo.ru_stime.tv_sec) * 1000;
	r += (double)(rinfo.ru_utime.tv_usec + rinfo.ru_stime.tv_usec) / 1000;
	return (int)r;
}

void timer(int sig)
{
	kill(pid, SIGUSR1);
	alarm(1);
}


void final_result(int r)
{
	printf("Runtime: %d MS\n", getRuntime());
	printf("Memory: %d KB\n", maxmem);
	exit(r);
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
		setregid(99, 99);
		setreuid(99, 99);
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execl(exename, exename, NULL);
		exit(0);
	}
	
	signal(SIGALRM, timer);
	alarm(1);

	int stat, tmpmem, sig;
	for (;;)
	{
		wait4(pid, &stat, 0, &rinfo);
		if (WIFEXITED(stat))
		{
			puts("exited!\n");
			break;
		}
		else if (WIFSTOPPED(stat))
		{
			sig = WSTOPSIG(stat);
			if (sig == SIGTRAP)
			{
					if (checkSyscall() == RET_RF)
					{
						ptrace(PTRACE_KILL, pid, NULL, NULL);
						final_result(RET_RF);
					}
			}
			else if (sig == SIGUSR1)
			{
			}
			else
				printf("Stopped due to signal: %d\n", sig);
		}
		else if (WIFSIGNALED(stat))
		{
			//Runtime Error
			printf("Runtime Error. Received signal: %d\n", WTERMSIG(stat));
			final_result(RET_RE);
			break;
		}
		tmpmem = getMemory();
		if (tmpmem > maxmem) maxmem = tmpmem;

		if (maxmem > mlimit)
			final_result(RET_MLE);
		if (getRuntime() > tlimit)
		{
			ptrace(PTRACE_KILL, pid, NULL, NULL);
			final_result(RET_TLE);
		}
		ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
	}
	final_result(RET_AC);
	
	return 0;
}
