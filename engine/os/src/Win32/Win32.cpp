#include <stdafx.h>
#include <engine/os/Input.hpp>
#include <engine/os/Window.hpp>
using namespace OS::InputState;
using namespace OS;
#include <windows.h>
#include <iostream>
#include <intrin.h>

#include <engine/os/Files.hpp>
namespace OS
{
	namespace Files
	{
		Options::Result< FileImage > load( String filename , Allocator *allocator )
		{
			FILE *file = NULL;
			fopen_s( &file , filename.getChars() , "rb" );
			if( file )
			{
				fseek( file , 0 , SEEK_END );
				int size = ftell( file );
				rewind( file );
				byte *data = allocator->alloc< byte >( size + 1 );
				fread( data , 1 , size , file );
				fclose( file );
				( ( byte* )data )[ size ] = '\0';
				return Options::Result< FileImage >( FileImage( data , allocator , size ) );
			} else
			{
				return Options::Result< FileImage >();
			}
		}
		void save( String filename , void const *data , uint length )
		{
			FILE *file = NULL;
			fopen_s( &file , filename.getChars() , "wb" );
			fwrite( data , 1 , length , file );
			fclose( file );
		}
		void append( String filename , void const *data , uint length )
		{
			FILE *file = NULL;
			fopen_s( &file , filename.getChars() , "ab" );
			fwrite( data , 1 , length , file );
			fclose( file );
		}
	}
	void exec( String command )
	{
		system( command.getChars() );
	}
}

/*#else
uint64_t rdtsc()
{
	unsigned int lo , hi;
	__asm__ __volatile__( "rdtsc" : "=a" ( lo ) , "=d" ( hi ) );
	return ( ( uint64_t )hi << 32 ) | lo;
}
#endif*/

LRESULT CALLBACK WndProc( HWND hWnd , UINT message , WPARAM wParam , LPARAM lParam )
{
	switch( message )
	{
	case WM_DESTROY: case WM_CLOSE:
		PostQuitMessage( 0 );
		break;
	default:
		return DefWindowProc( hWnd , message , wParam , lParam );
	}
	return 0;
}
struct WindowContext
{
	HWND hwnd;
	HDC hdc;
	HGLRC oglcontext;
};
Window::~Window()
{
	ReleaseDC( hwnd , hdc );
	DeleteDC( hdc );
}
void Window::run()
{
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = GetModuleHandle( NULL );
	this->hinstance = wc.hInstance;
	wc.hbrBackground = ( HBRUSH )( COLOR_WINDOW );
	wc.lpszClassName = "minwindowsapp";
	RegisterClass( &wc );
	hwnd = CreateWindow( wc.lpszClassName , "Minimal Windows Application" ,
		WS_POPUP | WS_VISIBLE , param.x , param.y , param.width , param.height , NULL , NULL , wc.hInstance , NULL );
	hdc = GetDC( hwnd );
	
	
	
	/*int new_pixel_format;
	BOOL is_new_pixel_format_valid;
	UINT valid_formats_count;
	int piAttribIList[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 24,
		WGL_ALPHA_BITS_ARB,8,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
		WGL_SAMPLES_ARB, 4,
		0 };
	is_new_pixel_format_valid = wglChoosePixelFormatARB( hdc , piAttribIList , NULL , 1 , &new_pixel_format , &valid_formats_count );
	if( is_new_pixel_format_valid && valid_formats_count > 0 )
	{
		SetPixelFormat( hdc , new_pixel_format , &pfd );
		auto temp_context = wglCreateContext( hdc );
		wglMakeCurrent( hdc , NULL );
		wglDeleteContext( oglcontext );
		oglcontext = temp_context;
		wglMakeCurrent( hdc , oglcontext );
		err = glewInit();
		if( GLEW_OK != err )
		{
			std::cout << "glew error:" << glewGetErrorString( err );
		}
		glEnable( GL_MULTISAMPLE_ARB );
	}*/
	if( param.init_func )
	{
		param.init_func();
	}
	OS::Async::Timer timer;
	State input_state;
	Allocator::zero( &input_state );
	while( true )
	{
		MSG msg = { 0 };
		//PeekMessage( &msg , hwnd , 0 , 0 , PM_REMOVE );
		GetMessage( &msg , hwnd , 0 , 0 );
		TranslateMessage( &msg );
		DispatchMessage( &msg );
		timer.updateTime();
		switch( msg.message )
		{
		case WM_QUIT:
			ShowWindow( hwnd , SW_HIDE );
			DestroyWindow( hwnd );
			break;
		case WM_MOUSEMOVE:
		{
			int nx = LOWORD( msg.lParam );
			int ny = HIWORD( msg.lParam );
			input_state.ptr_pos.x = nx;
			input_state.ptr_pos.y = ny;
			if( param.input_handler != nullptr )
			{
				param.input_handler( EventType::PTR_MOVE , &input_state );
			}
		}
		break;
		case WM_LBUTTONDOWN:
		{
			input_state.ptr_key_state[ 0 ] = true;
			if( param.input_handler != nullptr )
			{
				param.input_handler( EventType::PTR_DOWN , &input_state );
			}
		}
		break;
		case WM_LBUTTONUP:
		{
			input_state.ptr_key_state[ 0 ] = false;
			if( param.input_handler != nullptr )
			{
				param.input_handler( EventType::PTR_UP , &input_state );
			}
		}
		break;
		case WM_MOUSEWHEEL:
		{
			auto zDelta = GET_WHEEL_DELTA_WPARAM( msg.wParam );
			input_state.ptr_pos.z += zDelta;
			if( param.input_handler != nullptr )
			{
				param.input_handler( EventType::PTR_MOVE , &input_state );
			}
		}
		break;
		case WM_KEYDOWN:
		{
			//OS::IO::debugLogln( "WM_KEYDOWN" );
			auto key_code = msg.wParam;
			input_state.keyboard_state[ key_code ] = true;
			if( param.input_handler != nullptr )
			{
				param.input_handler( EventType::KEY_DOWN , &input_state );
			}
		}
		break;
		case WM_KEYUP:
		{
			//OS::IO::debugLogln( "WM_KEYUP" );
			auto key_code = msg.wParam;
			input_state.keyboard_state[ key_code ] = false;
			if( param.input_handler != nullptr )
			{
				param.input_handler( EventType::KEY_UP , &input_state );
			}
		}
		break;
		case WM_SIZE:
		{
			if( param.resize_func )
			{
				RECT wndrect;
				GetWindowRect( hwnd , &wndrect );
				param.width = wndrect.right - wndrect.left;
				param.height = wndrect.bottom - wndrect.top;
				param.x = wndrect.left;
				param.y = wndrect.top;
				param.resize_func( param.x , param.y , param.width , param.height );
			}
		}
		break;
		}
	}
	if( param.release_func )
	{
		param.release_func();
	}
}
int2 Window::getSize() const
{
	return{ param.width , param.height };
}
void Window::setPosition( int2 const &pos )
{
	param.x = pos.x;
	param.y = pos.y;
	SetWindowPos( hwnd ,
	HWND_TOP ,
	param.x ,
	param.y ,
	param.width , param.height ,
	SWP_SHOWWINDOW );
}
void Window::setSize( int2 const &size )
{
	param.width = size.x;
	param.height = size.y;
	SetWindowPos( hwnd ,
		HWND_TOP ,
		param.x ,
		param.y ,
		param.width , param.height ,
		SWP_SHOWWINDOW );
}
