#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "../libpcsxcore/system.h"

// Implementation of system dependant functions.

void SysPrintf(const char *fmt, ...) {
	va_list list;
	char msg[512];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

  static char linestart = 1;
  int l = strlen(msg);

  printf(linestart ? " * %s" : "%s", msg);

  if (l > 0 && msg[l - 1] == '\n') {
    linestart = 1;
  } else {
    linestart = 0;
  }
}
