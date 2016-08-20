#pragma once
#include <engine/util/defines.hpp>
#include <GL/glew.h>
#include <gl/GL.h>
#include <engine/os/log.hpp>
#pragma comment( lib , "opengl32.lib" )
#pragma comment( lib , "glew32.lib" )
static void checkError( char const *mark = "anonimous mark" )
{
	int error_code;
	do
	{
		error_code = glGetError();
		if( error_code != GL_NO_ERROR )
		{
			OS::IO::debugLogln( error_code , ":" , glewGetErrorString( error_code ) , " at " , mark );
		}
	} while( error_code != GL_NO_ERROR );
}