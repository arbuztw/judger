#include <stdio.h>
#include <string.h>
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

int main(int argc, char *argv[])
{
	if (argc < 3) return 0;

	int outed, ansed, sflg = 1, lflg = 1, eflg = 1;
	char s1[BUFSZ], s2[BUFSZ];
	FILE *fout, *fans;

	fout = fopen(argv[1], "r");
	fans = fopen(argv[2], "r");

	for (;;)
	{
		outed = !fgets(s1, BUFSZ, fout);
		ansed = !fgets(s2, BUFSZ, fans);
		
		if (lflg)
		{
			while (!outed && isBlank(s1))
				outed = !fgets(s1, BUFSZ, fout);
			while (!ansed && isBlank(s2))
				ansed = !fgets(s2, BUFSZ, fans);
		}

		if (outed && ansed) break;

		if (sflg)
		{
			trim(s1);
			trim(s2);
		}

		if (eflg && (outed && !isBlank(s2) || ansed && !isBlank(s1)) ||
			!eflg && strcmp(s1, s2))
			return RET_WA;
	}

	
	return 0;
}
