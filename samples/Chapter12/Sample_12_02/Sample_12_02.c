#include <windows.h>
#include <egl/egl.h>
#include <vg/openvg.h>
#include <vg/vgu.h>

#include "font.h"

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

void TextFont_Init( void );
void TextFont_Release( void );
void TextFont_Draw( int argc, void *argv );

VGfloat clearColor[4] = { 1.0f, 1.0f, 1.0f, 0.8f };
VGPath path[128];
VGFont font;
VGPaint paint;
VGfloat glyphOrigin[2] = { 0.0f, 0.0f };
VGfloat escapement[2] = { 0.0f, 0.0f };
VGubyte name[6] = "ALEX";

LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
HINSTANCE g_hInst;
LPSTR lpszClass="OVG PG: Sample 12_02";

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
    WndClass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
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
    TextFont_Init();
    TextFont_Draw( 0, NULL );
    TextFont_Release();

    return 1;
}

void TextFont_Init( void )
{
    int i;
	VGfloat linearGradient[4] = {0.0f, 0.0f, 450.0f, 0.0f};
    VGfloat rampStops[25] = {0.00f, 1.0f, 1.0f, 1.0f, 1.0f,
                             0.25f, 1.0f, 0.0f, 0.0f, 1.0f,
                             0.50f, 0.0f, 1.0f, 0.0f, 1.0f,
							 0.75f, 1.0f, 0.0f, 1.0f, 1.0f,
                             1.00f, 0.0f, 0.0f, 0.0f, 1.0f};

    vgSetfv( VG_CLEAR_COLOR, 4, clearColor );
    vgClear( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
    
    for( i = 0; i < FONTCOUNT ; i++)
    {
        path[font_name[i]] = vgCreatePath( VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1, 0, 0, 0, VG_PATH_CAPABILITY_ALL );
        vgAppendPathData( path[font_name[i]], count_name[i], cmd_name[i], coord_name[i] );
    }

    font = vgCreateFont(0);

    vgSeti( VG_MATRIX_MODE, VG_MATRIX_GLYPH_USER_TO_SURFACE );

	paint = vgCreatePaint();
	vgSetPaint( paint, VG_FILL_PATH );
    vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
    vgSetParameterfv(paint, VG_PAINT_LINEAR_GRADIENT, 4, linearGradient);
    vgSetParameterfv(paint, VG_PAINT_COLOR_RAMP_STOPS, 25, rampStops);
    vgSetParameteri(paint, VG_PAINT_COLOR_RAMP_SPREAD_MODE, VG_COLOR_RAMP_SPREAD_PAD);
}

void TextFont_Release( void )
{  
    int i;

    for( i = 0; name[i] != '\0'; i++ )
        vgClearGlyph(font, name[i]);

    vgDestroyFont(font);

    for( i = 0 ; i < FONTCOUNT ; i++ )
        vgDestroyPath( path[font_name[i]] );
}

void TextFont_Draw( int argc, void *argv )
{
    VGint i = 0, j = 0;

	vgSeti( VG_MATRIX_MODE, VG_MATRIX_GLYPH_USER_TO_SURFACE );
    vgLoadIdentity();
    vgTranslate( 10.0f, 140.0f );
    vgScale( 0.3f, 0.3f );

    while( name[i] != '\0' )
    {
        for( j = 0; j < FONTCOUNT ; j++)
        {
            if(name[i] == font_name[j])
                break;
        }
        escapement[0] = font_distance[j];

        vgSetGlyphToPath( font, name[i], path[name[i]], VG_TRUE, glyphOrigin, escapement );
        vgDestroyPath( path[name[i]] );

		vgDrawGlyph( font, name[i], VG_FILL_PATH, VG_TRUE );    
        i++;
    }
}
