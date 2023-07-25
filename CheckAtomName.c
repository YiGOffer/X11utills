// gcc -o xclipget xclipget.c -lX11
#include "window_manage.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <X11/Xlib.h>

int main(int argc, char **argv)
{
    Window win;
    Display *display = XOpenDisplay(NULL);
    char *name = NULL;

    if (NULL == display)
    {
        return -1;
    }

    if(argc != 2) 
        return 0 ;

    if(0 == strcmp(argv[1], "list")) {
        unsigned long atom = 1 ;

        printf("List Atom:\n") ;

        for (; atom < 1024 ; atom++){
            name = XGetAtomName(display,atom);

            if (NULL == name) {
                break ;
            }

            printf("\t%d: %s\n", (int)atom, name) ;
            XFree(name);
        }
    } else {
        Atom atom = atol(argv[1]);
        
        name = XGetAtomName(display, atom);

        if (name)
        {
            printf("Atom : %lu , Name : %s\n", atom, name);
            XFree(name);
        }
    }

#if 0
  if(argc == 3) {
    if(0 == strcmp(argv[1], "set")) {
      win = atol(argv[2]) ;
      set_host_name(display, win, "COM") ;
      printf("set COM, window: %d\n", win) ;
    }
    else if(0 == strcmp(argv[1], "get")) {
      char host_name[256] = {0} ;
      win = atol(argv[2]) ;
      get_host_name(display, win, host_name, sizeof(host_name)) ;
      printf("host name: %s, window: %d\n", host_name, win) ;
    }
    else {
      printf("null\n") ;
    }
  }
#endif

    XCloseDisplay(display);
    return 0;
}
