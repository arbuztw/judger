#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "judger.h"
#define BUFSZ 1024

int isBlank(char str[])
{
	int i;
	for (i = 0; str[i]; i++)
		if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
			return 0;
	return 1;
}

void trim(char str[])
{
	int n = strlen(str), i;

	for (i = n - 1; i >= 0; i--)
		if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
			break;
	str[i+1] = 0;
}

void usage()
{
	printf("\
Usage: compare [options] file1 file2\n\
Options:\n\
  -s	Ignore spaces at the end of the line\n\
  -l	Ignore all blank lines\n\
  -e	Ignore blank lines ONLY at the end of the file\n");
}



int main(int argc, char *argv[])
{
	int ed1, ed2, sflg = 0, lflg = 0, eflg = 0;
	char ch;
	char s1[BUFSZ], s2[BUFSZ];
	FILE *f1, *f2;

	opterr = 0;
	while (~(ch = getopt(argc, argv, "sle")))
		switch (ch)
		{
			case 's': sflg = 1; break;
			case 'l': lflg = 1; break;
			case 'e': eflg = 1; break;
		}

	if (argc - optind < 2) 
	{
		usage();
		return RET_SE;
	}

	f1 = fopen(argv[optind], "r");
	f2 = fopen(argv[optind+1], "r");

	for (;;)
	{
		ed1 = !fgets(s1, BUFSZ, f1);
		ed2 = !fgets(s2, BUFSZ, f2);
		
		if (lflg)
		{
			while (!ed1 && isBlank(s1))
				ed1 = !fgets(s1, BUFSZ, f1);
			while (!ed2 && isBlank(s2))
				ed2 = !fgets(s2, BUFSZ, f2);
		}

		if (ed1 && ed2) break;

		if (sflg)
		{
			trim(s1);
			trim(s2);
		}

		if (eflg && (ed1 && !isBlank(s2) || ed2 && !isBlank(s1)) ||
			!eflg && (strcmp(s1, s2) || ed1 || ed2))
			return RET_WA;
	}

	
	return 0;
}
