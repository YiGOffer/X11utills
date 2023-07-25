// complie: gcc main.cpp -o main -lX11
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

int main(int argc, char **argv){
    Display *d;
    char *endptr ;
    Window w;

    if(argc != 2) {
        printf("Usage: %s <pid>\n", argv[0]) ;
        exit(1) ;
    }

    w = (Window)strtol(argv[1], &endptr, 16) ;

    d = XOpenDisplay(NULL); // 连接Xserver
    if (d == NULL)
    {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    XWindowAttributes xgwa = {0} ;
    XGetWindowAttributes(d, w, &xgwa);

    printf("root window: %p, x: %d, y: %d, width: %d, height: %d\n", (void *)xgwa.root,
                                                                        xgwa.x,
                                                                        xgwa.y,
                                                                        xgwa.width,
                                                                        xgwa.height) ;

    return 0;
}
