#include <stdio.h>
#include <stdarg.h>

int __printf_chk(int flag, const char* fmt, ...) {
	va_list ap;
	int ret;
	va_start(ap, fmt);
	ret = vfprintf(stdout, fmt, ap);
	va_end(ap);
	return ret;
}
