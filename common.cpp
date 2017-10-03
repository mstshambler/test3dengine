#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// debug function
void debug(const char *fmt, ...) {
	char text[10240];
	va_list		ap;
	FILE *fp;

	if (fmt == NULL)
		return;

	va_start(ap, fmt);
	    vsprintf(text, fmt, ap);
	va_end(ap);

	fp = fopen("debug.txt","a+");
	if (fp) {
		fprintf(fp, "%s\n",text);
		fclose(fp);
	}
}
