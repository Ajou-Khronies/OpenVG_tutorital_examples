#include <windows.h>
#include <egl/egl.h>
#include <vg/openvg.h>
#include <vg/vgu.h>
#include "..\..\..\resource\resource.h"

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 640

HWND hWnd;
HDC appDc;

EGLDisplay display;
EGLSurface surface;
EGLContext context;

int AppInit( NativeWindowType native_window, NativeDisplayType native_display );
int AppSampleInit( void );
int AppDisplay( int );
int AppRelease( void );

LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
HINSTANCE g_hInst;
LPSTR lpszClass="OVG PG: Sample 11_05";

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    MSG Message;
    WNDCLASS WndClass;
    DWORD dwStyle;
    RECT rect;

    g_hInst = hInstance;

    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
    WndClass.hIcon = LoadImage( hInstance, 
                                MAKEINTRESOURCE(IDI_ICON2), 
								IMAGE_ICON, 
								GetSystemMetrics(SM_CXSMICON), 
								GetSystemMetrics(SM_CYSMICON), 
								LR_DEFAULTCOLOR );
    WndClass.hInstance = hInstance;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.lpszClassName = lpszClass;
    WndClass.lpszMenuName = NULL;
    WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

    RegisterClass(&WndClass);

    rect.left = 0;
    rect.top = 0;
    rect.right = SCREEN_WIDTH;
    rect.bottom = SCREEN_HEIGHT;

    dwStyle = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU );

    AdjustWindowRect(&rect, dwStyle, FALSE);

    hWnd=CreateWindow(lpszClass, lpszClass, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, rect.right-rect.left, rect.bottom-rect.top, NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd,nCmdShow);

    while(GetMessage(&Message,0,0,0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int rot = 0;
    static HANDLE hTimer;

    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_CREATE:
            appDc = GetDC( hWnd );

            if ( AppInit( hWnd, appDc ) == 0 )
                return 0; 

            AppSampleInit();

            hTimer = (HANDLE) SetTimer(hWnd, 1, 100, NULL);
            break;

        case WM_TIMER:
            switch(wParam)
            {
                case 1:
                    InvalidateRect(hWnd,NULL, FALSE);
                    break;
            }
            break;

        case WM_KEYDOWN:
            if( wParam == 32) // Space Bar
            {
                rot++;
                rot %= 360;                
            }
            break;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            AppDisplay(rot);
            eglSwapBuffers( display, surface ); // Display
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            AppRelease();

            KillTimer(hWnd, 1);

            ReleaseDC( hWnd, appDc );
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
int AppInit( NativeWindowType native_window, NativeDisplayType native_display )
{
    EGLint config_list[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    EGLint attrib_list[] = {
        EGL_WIDTH, 480,
        EGL_HEIGHT, 640,
        EGL_NONE
    };

    EGLConfig config;
    EGLint num_config;

    display = eglGetDisplay( native_display );

    if (display == EGL_NO_DISPLAY)
        return 0;

    if (eglInitialize(display, NULL, NULL) == EGL_FALSE || eglGetError() != EGL_SUCCESS)
        return 0;

    eglBindAPI(EGL_OPENVG_API);
    eglChooseConfig(display, config_list, &config, 1, &num_config);
    surface = eglCreateWindowSurface( display, config, native_window, NULL );

    if ( surface == EGL_NO_SURFACE || eglGetError() != EGL_SUCCESS )
        return 0;

    context = eglCreateContext( display, config, NULL, NULL );
    if ( context == EGL_NO_CONTEXT || eglGetError() != EGL_SUCCESS )
        return 0;

    if ( eglMakeCurrent( display, surface, surface, context ) == EGL_FALSE || eglGetError() != EGL_SUCCESS )
        return 0;

    return 1;
}

int AppRelease( void )
{
    eglMakeCurrent(display, EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT );

    eglDestroyContext( display, context );
    context = NULL;

    eglDestroySurface( display, surface );
    surface = NULL;

    eglTerminate( display );
    display = NULL;

    return 1;
}

VGPath Spath, Epath ;
VGPaint fillPaint, strokePaint;
VGfloat fillColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
VGfloat strokeColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

VGubyte segments[] = { VG_MOVE_TO_ABS, VG_LINE_TO_ABS, VG_LINE_TO_ABS,
                       VG_LINE_TO_ABS, VG_LINE_TO_ABS, VG_CLOSE_PATH };
VGfloat Scoords[] = { 120.0f, 260.0f, 61.2f, 79.1f, 215.1f, 190.9f,
                        24.8f, 190.9f, 178.8f, 79.1f };
VGfloat Ecoords[] = { 120.0f, 260.0f, 24.8f, 190.9f, 61.2f, 79.1f,
                       178.8f, 79.1f, 215.1f, 190.9f};
VGfloat clearColor[] = {1.0f, 1.0f, 1.0f, 1.0f}; // To show Mask

int AppSampleInit()
{
    vgSetfv( VG_CLEAR_COLOR, 4, clearColor );
    Spath = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,1.0f, 0.0f, 0, 0,
            VG_PATH_CAPABILITY_ALL);
    vgAppendPathData( Spath, 6, segments, Scoords );
    Epath = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,1.0f, 0.0f, 0, 0,
            VG_PATH_CAPABILITY_ALL);
    vgAppendPathData( Epath, 6, segments, Ecoords );
    vgSeti( VG_FILL_RULE, VG_EVEN_ODD ); // OR VG_NON_ZERO
    vgSetf( VG_STROKE_LINE_WIDTH, 8.0f );
    fillPaint = vgCreatePaint();
    strokePaint = vgCreatePaint();
    vgSetPaint(fillPaint, VG_FILL_PATH );
    vgSetPaint(strokePaint, VG_STROKE_PATH );

    return 1;
}
int AppDisplay(int rot)
{
    VGPath path;
    path = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,1.0f, 0.0f, 0, 0,
            VG_PATH_CAPABILITY_ALL);
    vgClear( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
    vgInterpolatePath(path, Spath, Epath,(float) rot/360.0 );
    strokeColor[0] = fillColor[2] = (float) rot/360.0;
    vgSetParameterfv( fillPaint, VG_PAINT_COLOR, 4, fillColor);
    vgSetParameterfv( strokePaint, VG_PAINT_COLOR, 4, strokeColor);
    vgDrawPath(path, VG_FILL_PATH | VG_STROKE_PATH );
    vgDestroyPath( path );

    return 1;
}
int AppDone()
{
    vgDestroyPath( Spath );
    vgDestroyPath( Epath );
    vgDestroyPaint( fillPaint );
    vgDestroyPaint( strokePaint );

    return 1;
}