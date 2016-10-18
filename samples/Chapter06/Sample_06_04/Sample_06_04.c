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
int AppDisplay( void );
int AppRelease( void );

LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
HINSTANCE g_hInst;
LPSTR lpszClass="OVG PG: Sample 06_04";

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
PAINTSTRUCT ps;
HDC hdc;

    switch (message)
    {
        case WM_CREATE:
            appDc = GetDC( hWnd );

            if ( AppInit( hWnd, appDc ) == 0 )
                return 0;
            break;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            AppDisplay();
            eglSwapBuffers( display, surface ); // Display
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            AppRelease();

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

extern const unsigned long cimg_wine64x64[];

int AppDisplay( void )
{
    VGPath path;
    VGImage imageWine;
    VGPaint fillPaint;
    VGubyte segments[] = { VG_MOVE_TO_ABS, VG_LINE_TO_ABS, VG_LINE_TO_ABS,
    VG_LINE_TO_ABS, VG_LINE_TO_ABS, VG_CLOSE_PATH };
    VGfloat coords[] = { 120.0f, 260.0f, 61.2f, 79.1f, 215.1f, 190.9f,
                          24.8f, 190.9f, 178.8f, 79.1f };
    VGfloat clear[4] = { .9f, .9f, .9f, 1.0f };
    VGfloat tileFillColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

    imageWine = vgCreateImage( VG_sRGBA_8888, 64, 64,VG_IMAGE_QUALITY_BETTER );
    vgImageSubData( imageWine, cimg_wine64x64, 64*4, VG_sRGBA_8888,0,0,64,64 );

    vgSetfv( VG_CLEAR_COLOR, 4, clear );
    vgClear( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
    fillPaint = vgCreatePaint();
    path = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,1.0f, 0.0f, 0, 0,
    VG_PATH_CAPABILITY_ALL);
    vgAppendPathData( path, 6, segments, coords );

    vgSeti( VG_FILL_RULE, VG_NON_ZERO ); // CHANGE

    vgPaintPattern( fillPaint, imageWine );
    vgSetParameteri( fillPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_PATTERN );
    vgSetParameteri( fillPaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_FILL); // (a)
    //vgSetParameteri( fillPaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_REPEAT); // (b)
    //vgSetParameteri( fillPaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_REFLECT);// (c)
    //vgSetParameteri( fillPaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_PAD ); // (d)
    vgSetfv( VG_TILE_FILL_COLOR, 4, tileFillColor );
    vgSetPaint( fillPaint, VG_FILL_PATH );
    vgSeti( VG_MATRIX_MODE, VG_MATRIX_FILL_PAINT_TO_USER );
    vgTranslate (90.0f,130.0f);

    vgDrawPath( path, VG_FILL_PATH);
    vgDestroyPath( path );
    vgDestroyPaint( fillPaint );

    return 1;
}