#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libcsoup.h"
#include "rename.h"


char	*testcases = "\
[Test001]\n\
MKFILE: 'Plan your trip.txt'\n\
RENAME: renamex 'Plan your trip.txt' 'my trip.txt'\n\
VERIFY: 'my trip.txt'\n\
DELETE: 'Plan your trip.txt' 'my trip.txt'\n\
\n\
[Test002]\n\
MKFILE: 'Plan your trip.txt'\n\
MKDIR: tripper\n\
RENAME: renamex 'Plan your trip.txt' tripper\n\
VERIFY: tripper/'Plan your trip.txt'\n\
DELETE: tripper/'Plan your trip.txt'\n\
";


int main(int argc, char **argv)
{
	FILE	*fin, *ftmp;
	char	buf[8192];
	int	i, flags = 0;

	if (argc < 2) {
		puts(testcases);
		return 0;
	}
	if ((fin = fopen(argv[1], "r")) == NULL) {
		perror(argv[1]);
		return -1;
	}
	while (fgets(buf, sizeof(buf), fin)) {
		buf[strlen(buf)-1] = 0;
		if (flags == 0) {
			if (buf[0] == '[') {
				flags = 1;
				printf("Testing %s:  ", buf);
			}
			continue;
		}
		
		if (buf[0] == '[') {
			flags = 1;
			printf("Testing %s:  ", buf);
		} else if (!strncmp(buf, "MKFILE:", 7)) {
			sprintf(buf, "echo >");
			buf[6] = ' ';
			system(buf);
		} else if (!strncmp(buf, "MKDIR:", 6)) {
			sprintf(buf, "mkdir");
			buf[5] = ' ';
			system(buf);
		} else if (!strncmp(buf, "RENAME:", 7)) {
			system(&buf[7]);
		} else if (!strncmp(buf, "VERIFY:", 7)) {
			for (i = 7; isspace(buf[i]); i++);
			if ((ftmp = fopen(&buf[i], "r")) == NULL) {
				printf("failed\n");
			} else {
				printf("ok\n");
				fclose(ftmp);
			}
		} else if (!strncmp(buf, "DELETE:", 7)) {
			sprintf(buf, "rm -f");
			buf[5] = buf[6] = ' ';
			system(buf);
		} else if (!strncmp(buf, "RMDIR:", 6)) {
			sprintf(buf, "rmdir");
			buf[5] = ' ';
			system(buf);
		}
	}
	fclose(fin);
	return 0;
}

