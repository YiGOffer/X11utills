#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define MAKE_INDENT(a) {for(int j = 0 ; j < a ; j++){printf("\t");}}
#define MAX_PROPERTY_VALUE_LEN 4096 
#define WINDOW_NAME_LEN 1024

unsigned int Indent_Table = 0;

void make_sub_line_indent(unsigned int indent_deep){
    int count ;
    for (count = 0;count < indent_deep ;count++){
        if(Indent_Table & (1<<count)){
            printf("│");
        }
        printf("\t");
    }
};


char *get_property (Display *disp, Window win,
    Atom xa_prop_type, const char *prop_name, unsigned long *size) {
    Atom xa_prop_name;
    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes_after;
    unsigned long tmp_size;
    unsigned char *ret_prop;
    char *ret;

    xa_prop_name = XInternAtom(disp, prop_name, False);

    /* MAX_PROPERTY_VALUE_LEN / 4 explanation (XGetWindowProperty manpage):
     *
     * long_length = Specifies the length in 32-bit multiples of the
     *               data to be retrieved.
     */
    if (XGetWindowProperty(disp, win, xa_prop_name, 0, MAX_PROPERTY_VALUE_LEN / 4, False,
            xa_prop_type, &xa_ret_type, &ret_format,
            &ret_nitems, &ret_bytes_after, &ret_prop) != Success) {
        //printf("Cannot get %s property.\n", prop_name) ;
        return NULL;
    }

    if (xa_ret_type != xa_prop_type) {
        //printf("Invalid type of %s property.\n", prop_name);
        XFree(ret_prop);
        return NULL;
    }

    /* null terminate the result to make string handling easier */
    tmp_size = (ret_format / (32 / sizeof(long))) * ret_nitems;
    ret = (char *)malloc(tmp_size + 1);
    memcpy(ret, ret_prop, tmp_size);
    ret[tmp_size] = '\0';

    if (size) {
        *size = tmp_size;
    }

    XFree(ret_prop);
    return ret;
}

unsigned long get_pid_by_window(Display* display,Window win)
{
    unsigned long pid = 0 ;
    unsigned long *ppidbuf = NULL ;

    ppidbuf = (unsigned long *)get_property(display, win,
            XA_CARDINAL, "_NET_WM_PID", NULL) ;

    if(NULL != ppidbuf) {
        pid = *ppidbuf ;
        free(ppidbuf) ;
    }
    return pid ;
}

int get_window_name(Display* display,Window windowID,char* ret_name){
    char* name = NULL;
    Atom property_type;
    Atom ret_type;
    int ret;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes;
    unsigned char* ret_buf;

    property_type = XInternAtom(display,"WM_CLIENT_MACHINE",0);


    ret = XGetWindowProperty(display,windowID,property_type,0,WINDOW_NAME_LEN,0,XA_STRING,&ret_type,&ret_format,&ret_nitems,&ret_bytes,&ret_buf);
    if(ret != 0){
        return -1;
        printf("get name fail\n");
    }

    if( ret_buf != NULL){
        sprintf(ret_name,"%s | ",ret_buf);
        XFree(ret_buf);
    }

    property_type = XInternAtom(display,"_NET_WM_NAME",0);

    ret = XGetWindowProperty(display,windowID,property_type,0,WINDOW_NAME_LEN,0,XInternAtom(display,"UTF8_STRING",0),&ret_type,&ret_format,&ret_nitems,&ret_bytes,&ret_buf);

    if (ret_buf != NULL)
    {
        sprintf(ret_name," [ %s ]",ret_buf);
        XFree(ret_buf);
    }


    return 0;
}


int print_all_sub_window(Display* display,Window windowID,unsigned int indent_deep){

    int i;
    int ret;
    //unsigned int control = 1;
    int Indent_Deep;
    Window root_wd,parent_wd; 
    Window* child_wd_list = NULL;
    unsigned int childw_count = 0;
    char window_name[WINDOW_NAME_LEN];

    Indent_Deep = indent_deep + 1;

    XQueryTree(display,windowID,&root_wd,&parent_wd,&child_wd_list,&childw_count);


    if(child_wd_list != NULL && childw_count > 0){
        Indent_Table |= (1 << Indent_Deep);
        make_sub_line_indent(Indent_Deep);
        printf("│\n");
        for(i = 0; i < childw_count; ++i ){
            if(i == childw_count - 1){
                Indent_Table &= ~( 1 << Indent_Deep );

                make_sub_line_indent(Indent_Deep);
                printf("└──── Child Window : %p ", (void *)child_wd_list[i]);
                memset(window_name,0,WINDOW_NAME_LEN);
                ret = get_window_name(display,child_wd_list[i],window_name);
                if(ret != 0){
                    printf("\n");
                }else{
                    printf("%s \n", window_name);
                }


            }else{
                unsigned long pid = 0 ;
                make_sub_line_indent(Indent_Deep);
                printf("├──── Child Window : %p ", (void *)child_wd_list[i]);
                memset(window_name,0,WINDOW_NAME_LEN);
                ret = get_window_name(display,child_wd_list[i],window_name);
                if(ret != 0){
                    printf(" ");
                }else{
                    printf("%s ", window_name);
                }

                pid = get_pid_by_window(display, child_wd_list[i]) ;
                if(pid != 0) {
                    printf("[%lu]\n", pid) ;
                } else {
                    printf("\n") ;
                }
            }

            print_all_sub_window(display,child_wd_list[i],Indent_Deep);

        }
    }


    if(child_wd_list != NULL){
        free(child_wd_list);
    }


    return 0;
}




int main(int argc, char* argv[])
{
    Display* display = XOpenDisplay(NULL);
    XGCValues values;
    XWindowAttributes wattr;
    XSetWindowAttributes attr;
    XVisualInfo vinfo;
    Window root_wd,parent_wd; 
    Window main_wd;
    Window* child_wd_list = NULL;
    unsigned int childw_count = 0;

    if(argc != 2 ){
        printf("Usage %s <WINDID> , 0 for all windows\n",argv[0]);
        return -1;
    }

    main_wd = (Window)atol(argv[1]);
    if(main_wd == 0){
        main_wd = DefaultRootWindow(display);
    }

    XQueryTree(display,main_wd,&root_wd,&parent_wd,&child_wd_list,&childw_count);

    if(child_wd_list != NULL){
        free(child_wd_list);
    }
    printf("Root Window : %p\n",(void*)root_wd);
    printf("--------------------------\n");
    printf("Main Window : %p\n",(void*)main_wd);


    print_all_sub_window(display,main_wd,0);

    return 0;
}