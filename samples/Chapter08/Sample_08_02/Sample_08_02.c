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
LPSTR lpszClass="OVG PG: Sample 08_02";

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

int AppDisplay( void )
{
    VGPath path;
	int i;
    VGPaint fillPaint, strokePaint;
    VGfloat linearGradient[4] = {61.0f, 79.0f, 215.0f, 190.0f};
    VGfloat rampStops[20] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                             0.3f, 1.0f, 0.0f, 0.0f, 1.0f,
                             0.6f, 0.0f, 1.0f, 0.0f, 1.0f,
                             1.0f, 0.0f, 0.0f, 1.0f, 1.0f};
    VGubyte segments[] = { VG_MOVE_TO_ABS, VG_LINE_TO_ABS, VG_LINE_TO_ABS,
                           VG_LINE_TO_ABS, VG_LINE_TO_ABS, VG_CLOSE_PATH };
    VGfloat coords[] = { 120.0f, 260.0f, 61.2f, 79.1f, 215.1f, 190.9f,
                          24.8f, 190.9f, 178.8f, 79.1f };
    VGfloat clear[4] = { .5f, .5f, .5f, 1.0f };
    VGfloat fillColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
    VGfloat strokeColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    vgSetfv( VG_CLEAR_COLOR, 4, clear );
    vgSetf( VG_STROKE_LINE_WIDTH, 8.0f );
    vgClear( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
    fillPaint = vgCreatePaint();
    strokePaint = vgCreatePaint();
    path = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,1.0f, 0.0f, 0, 0,
    VG_PATH_CAPABILITY_ALL);
    vgAppendPathData( path, 6, segments, coords );
    vgSeti( VG_FILL_RULE, VG_EVEN_ODD ); // OR VG_NON_ZERO
    vgSetPaint(fillPaint, VG_FILL_PATH );
    vgSetParameterfv( fillPaint, VG_PAINT_COLOR, 4, fillColor);
    vgSetParameteri(strokePaint, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
    vgSetParameterfv(strokePaint, VG_PAINT_LINEAR_GRADIENT, 4, linearGradient);
    vgSetParameterfv(strokePaint, VG_PAINT_COLOR_RAMP_STOPS, 20, rampStops);
    vgSetParameteri(strokePaint, VG_PAINT_COLOR_RAMP_SPREAD_MODE, VG_COLOR_RAMP_SPREAD_REFLECT);
    vgSetPaint( strokePaint, VG_STROKE_PATH );
    vgSeti( VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgLoadIdentity();
    vgScale(0.5,0.5);
	for ( i = 0 ; i < 10 ; i++)
	{
		vgTranslate(20.0,10.0);
        vgDrawPath( path, (VG_FILL_PATH | VG_STROKE_PATH) );
	}

	vgLoadIdentity();
	vgTranslate(0.0, 100.0);
    vgScale(0.5,0.5);
	for ( i = 0 ; i < 10 ; i++)
	{
		vgSeti( VG_MATRIX_MODE, VG_MATRIX_STROKE_PAINT_TO_USER);
		vgTranslate(20.0,10.0);
        vgSeti( VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgTranslate(20.0,10.0);
        vgDrawPath( path, (VG_FILL_PATH | VG_STROKE_PATH) );
	}
    vgDestroyPath( path );
    vgDestroyPaint( fillPaint );
    vgDestroyPaint( strokePaint );

    return 1;
}