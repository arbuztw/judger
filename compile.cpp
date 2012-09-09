#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <jsoncpp/json.h>
#include "judger.h"

const int compile_time_limit = 5; //second
const int compile_file_limit = 35; //kb
int pid;

const char *getCompileCommand(char *id, int lang)
{
	Json::Reader reader;
	Json::Value val;
	FILE *f = fopen("compile.json", "r+");
	char buf[1024], json[50000];
	std::string cmd;
	int found;
	
	
	while (fgets(buf, 1024, f))
		strcat(json, buf);
	
	fclose(f);
	
	if (reader.parse(json, val))
	{
		if (lang >= 0 && lang < val.size())
		{
			cmd = val[lang]["command"].asString();
			found = cmd.find("${id}");
			while (found != -1)
			{
				cmd.replace(found, 5, id);
				found = cmd.find("${id}", found+5);
			}
			return cmd.c_str();
		}
	}
	return NULL;
}

void timeout(int sig) { kill(pid, SIGUSR1); }

int main(int argc, char *argv[])
{
	int stat;
	const char *cmd;
	if (argc < 3)
	{
		printf("Usage: compile [id] [lang]\n");
		exit(RET_SE);
	}
	cmd =	getCompileCommand(argv[1], atoi(argv[2]));
	if (!cmd) exit(RET_SE);
	//signal(SIGALRM, timeout);
	//alarm(compile_time_limit);
	if ((pid = fork()) == 0)
	{
		struct rlimit rlim;
		/*rlim.rlim_cur = compile_file_limit * 1024;
		rlim.rlim_max = compile_file_limit * 1024;
		setrlimit(RLIMIT_FSIZE, &rlim);
		putenv("LC_ALL=en_US.UTF-8");*/
		rlim.rlim_cur = rlim.rlim_max = compile_time_limit;
		setrlimit(RLIMIT_CPU, &rlim);
		freopen("ce.txt", "w", stderr);
		chdir("chroot");
		setpgid(0, getpid());
		execl("/bin/sh", "sh", "-c", cmd, NULL);
	}
	else
	{
		wait4(pid, &stat, 0, NULL);
		if (WIFEXITED(stat))
		{
			if (WEXITSTATUS(stat))
				exit(RET_CE);
		}
		else if (WIFSIGNALED(stat))
		{
			kill(-pid, SIGTERM);
			if (WTERMSIG(stat) == SIGXFSZ)
				exit(RET_CE);
		}
	}
	return 0;
}
