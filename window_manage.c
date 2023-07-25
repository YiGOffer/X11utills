#define _GNU_SOURCE
#include "window_manage.h"
#include <ctype.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define MAX_PATH_LEN (4096)
#define MAX(X,Y) ((X)>(Y)?(X):(Y))
#define MIN(X,Y) ((X)<(Y)?(X):(Y))

int g_remove = 0 ; // writev中，是否移除
int g_replace_context = 0 ; // 控制在复制行为中，是否把里面的内容给替换了
int g_replace_window = 1 ; // 控制在复制行为中，是否把目标窗口给替换了

// 向上取整
#define UP(x, n) ((( x + n - 1 ) / n ) * n) 

static pthread_rwlock_t g_message_rwlock;//读写锁对象
static int stop = 0 ; // 当前是否已经停止工作了

static void *x11handle = NULL;
static void *libchandle = NULL;

static unsigned long main_window = 0 ;

static unsigned long qt_selection = 0 ;
static unsigned long gdk_selection = 0 ;
static unsigned long chrome_selection = 0 ;

static unsigned long string = 0 ; 
static unsigned long utf8_string = 0 ;
static unsigned long utf8_text =  0;
static unsigned long compund_text = 0 ;
static unsigned long text_plain = 0 ;

#define MAX_VALS (256)
value_string property_vals[MAX_VALS] = {0} ;
value_string type_vals[MAX_VALS] = {0} ;

typedef Display * (*PXOPENDISPLAYFUN)( const char* ) ;
typedef int (*PXCLOSEDISPLAYFUN)( Display * );
typedef Atom  (*PXINTERNATOMFUN)(Display*, const char*, Bool);
typedef char * (*PXGETATOMNAMEFUN)( Display* ,Atom );


unsigned char get_uint8(char *buf, int offset){
    return *(unsigned char  *)(buf + offset) ;
}

char get_int8(char *buf, int offset){
    return *(char *)(buf + offset) ;
}

unsigned short get_uint16(char *buf, int offset){
    return *(unsigned short *)(buf + offset) ;
}

short get_int16(char *buf, int offset){
    return *(short *)(buf + offset) ;
}

unsigned int get_uint32(char *buf, int offset){
    return *(unsigned int *)(buf + offset) ;
}

int get_int32(char *buf, int offset){
    return *(int *)(buf + offset) ;
}

#define INITIAL_CONN 0


static const value_string opcode_vals[] = {
      { INITIAL_CONN,                   "Initial connection request" },
      { X_CreateWindow,                 "CreateWindow" },
      { X_ChangeWindowAttributes,       "ChangeWindowAttributes" },
      { X_GetWindowAttributes,          "GetWindowAttributes" },
      { X_DestroyWindow,                "DestroyWindow" },
      { X_DestroySubwindows,            "DestroySubwindows" },
      { X_ChangeSaveSet,                "ChangeSaveSet" },
      { X_ReparentWindow,               "ReparentWindow" },
      { X_MapWindow,                    "MapWindow" },
      { X_MapSubwindows,                "MapSubwindows" },
      { X_UnmapWindow,                  "UnmapWindow" },
      { X_UnmapSubwindows,              "UnmapSubwindows" },
      { X_ConfigureWindow,              "ConfigureWindow" },
      { X_CirculateWindow,              "CirculateWindow" },
      { X_GetGeometry,                  "GetGeometry" },
      { X_QueryTree,                    "QueryTree" },
      { X_InternAtom,                   "InternAtom" },
      { X_GetAtomName,                  "GetAtomName" },
      { X_ChangeProperty,               "ChangeProperty" },
      { X_DeleteProperty,               "DeleteProperty" },
      { X_GetProperty,                  "GetProperty" },
      { X_ListProperties,               "ListProperties" },
      { X_SetSelectionOwner,            "SetSelectionOwner" },
      { X_GetSelectionOwner,            "GetSelectionOwner" },
      { X_ConvertSelection,             "ConvertSelection" },
      { X_SendEvent,                    "SendEvent" },
      { X_GrabPointer,                  "GrabPointer" },
      { X_UngrabPointer,                "UngrabPointer" },
      { X_GrabButton,                   "GrabButton" },
      { X_UngrabButton,                 "UngrabButton" },
      { X_ChangeActivePointerGrab,      "ChangeActivePointerGrab" },
      { X_GrabKeyboard,                 "GrabKeyboard" },
      { X_UngrabKeyboard,               "UngrabKeyboard" },
      { X_GrabKey,                      "GrabKey" },
      { X_UngrabKey,                    "UngrabKey" },
      { X_AllowEvents,                  "AllowEvents" },
      { X_GrabServer,                   "GrabServer" },
      { X_UngrabServer,                 "UngrabServer" },
      { X_QueryPointer,                 "QueryPointer" },
      { X_GetMotionEvents,              "GetMotionEvents" },
      { X_TranslateCoords,              "TranslateCoordinates" },
      { X_WarpPointer,                  "WarpPointer" },
      { X_SetInputFocus,                "SetInputFocus" },
      { X_GetInputFocus,                "GetInputFocus" },
      { X_QueryKeymap,                  "QueryKeymap" },
      { X_OpenFont,                     "OpenFont" },
      { X_CloseFont,                    "CloseFont" },
      { X_QueryFont,                    "QueryFont" },
      { X_QueryTextExtents,             "QueryTextExtents" },
      { X_ListFonts,                    "ListFonts" },
      { X_ListFontsWithInfo,            "ListFontsWithInfo" },
      { X_SetFontPath,                  "SetFontPath" },
      { X_GetFontPath,                  "GetFontPath" },
      { X_CreatePixmap,                 "CreatePixmap" },
      { X_FreePixmap,                   "FreePixmap" },
      { X_CreateGC,                     "CreateGC" },
      { X_ChangeGC,                     "ChangeGC" },
      { X_CopyGC,                       "CopyGC" },
      { X_SetDashes,                    "SetDashes" },
      { X_SetClipRectangles,            "SetClipRectangles" },
      { X_FreeGC,                       "FreeGC" },
      { X_ClearArea,                    "ClearArea" },
      { X_CopyArea,                     "CopyArea" },
      { X_CopyPlane,                    "CopyPlane" },
      { X_PolyPoint,                    "PolyPoint" },
      { X_PolyLine,                     "PolyLine" },
      { X_PolySegment,                  "PolySegment" },
      { X_PolyRectangle,                "PolyRectangle" },
      { X_PolyArc,                      "PolyArc" },
      { X_FillPoly,                     "FillPoly" },
      { X_PolyFillRectangle,            "PolyFillRectangle" },
      { X_PolyFillArc,                  "PolyFillArc" },
      { X_PutImage,                     "PutImage" },
      { X_GetImage,                     "GetImage" },
      { X_PolyText8,                    "PolyText8" },
      { X_PolyText16,                   "PolyText16" },
      { X_ImageText8,                   "ImageText8" },
      { X_ImageText16,                  "ImageText16" },
      { X_CreateColormap,               "CreateColormap" },
      { X_FreeColormap,                 "FreeColormap" },
      { X_CopyColormapAndFree,          "CopyColormapAndFree" },
      { X_InstallColormap,              "InstallColormap" },
      { X_UninstallColormap,            "UninstallColormap" },
      { X_ListInstalledColormaps,       "ListInstalledColormaps" },
      { X_AllocColor,                   "AllocColor" },
      { X_AllocNamedColor,              "AllocNamedColor" },
      { X_AllocColorCells,              "AllocColorCells" },
      { X_AllocColorPlanes,             "AllocColorPlanes" },
      { X_FreeColors,                   "FreeColors" },
      { X_StoreColors,                  "StoreColors" },
      { X_StoreNamedColor,              "StoreNamedColor" },
      { X_QueryColors,                  "QueryColors" },
      { X_LookupColor,                  "LookupColor" },
      { X_CreateCursor,                 "CreateCursor" },
      { X_CreateGlyphCursor,            "CreateGlyphCursor" },
      { X_FreeCursor,                   "FreeCursor" },
      { X_RecolorCursor,                "RecolorCursor" },
      { X_QueryBestSize,                "QueryBestSize" },
      { X_QueryExtension,               "QueryExtension" },
      { X_ListExtensions,               "ListExtensions" },
      { X_ChangeKeyboardMapping,        "ChangeKeyboardMapping" },
      { X_GetKeyboardMapping,           "GetKeyboardMapping" },
      { X_ChangeKeyboardControl,        "ChangeKeyboardControl" },
      { X_GetKeyboardControl,           "GetKeyboardControl" },
      { X_Bell,                         "Bell" },
      { X_ChangePointerControl,         "ChangePointerControl" },
      { X_GetPointerControl,            "GetPointerControl" },
      { X_SetScreenSaver,               "SetScreenSaver" },
      { X_GetScreenSaver,               "GetScreenSaver" },
      { X_ChangeHosts,                  "ChangeHosts" },
      { X_ListHosts,                    "ListHosts" },
      { X_SetAccessControl,             "SetAccessControl" },
      { X_SetCloseDownMode,             "SetCloseDownMode" },
      { X_KillClient,                   "KillClient" },
      { X_RotateProperties,             "RotateProperties" },
      { X_ForceScreenSaver,             "ForceScreenSaver" },
      { X_SetPointerMapping,            "SetPointerMapping" },
      { X_GetPointerMapping,            "GetPointerMapping" },
      { X_SetModifierMapping,           "SetModifierMapping" },
      { X_GetModifierMapping,           "GetModifierMapping" },
      { X_NoOperation,                  "NoOperation" },
      { 0,                              NULL }
};

const char * value_to_str(const value_string *vs, int value){
    if(NULL == vs) return NULL ;

    for(int i = 0; vs[i].value != 0; ++i){
        if(vs[i].value == value){
            return vs[i].string ;
        }
    }
    return NULL ;
}

int add_to_value_string(value_string *vs, int value, const char *str) {
    if(NULL == vs){
        return -1 ;
    }

    for(int i = 0; i < MAX_VALS - 1; ++i){
        if(0 == vs[i].value) {
            vs[i].value = value ;
            vs[i].string = str ;
            return 0 ;
        }
    }
    return -1 ;
}


/*
 * @brief: 获取指定窗口所属的主机名
 * @return:     返回主机名长度
 *    @retval ==0   成功
 *    @retval <0  失败
 */
int get_host_name(Display* display,Window win,char* namebuf, int buflen){
    XTextProperty textData; 
    int len = 0 ;

    int ret = XGetWMClientMachine(display,
                                    win,
                                    &textData);

    if (ret) {
        len = strlen((char *)textData.value) ;
        len = len >= buflen ? buflen - 1 : len ;
        strncpy(namebuf, (char *)textData.value, len) ;
    }

    return len > 0 ? 0 : -1 ;

}

/*
 * @brief: 设置指定窗口所属的主机名
 * @return:     返回主机名长度
 *    @retval ==0   成功
 *    @retval <0  失败
 */
int set_host_name(Display* display,Window win,char* host_name){
    XTextProperty txt_prop; 

    XStringListToTextProperty(&host_name, 1, &txt_prop);
    XSetWMClientMachine(display,
                                    win,
                                    &txt_prop);

    return 0 ;
}