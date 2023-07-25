#ifndef WINDOW_MANAGE
#define WINDOW_MANAGE
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>

typedef struct {int value; const char * string;} value_string;

const char * value_to_str(const value_string *vs, int value) ;

int get_host_name(Display* display,Window win,char* namebuf, int buflen) ;
int set_host_name(Display* display,Window win,char* host_name) ;

#endif
