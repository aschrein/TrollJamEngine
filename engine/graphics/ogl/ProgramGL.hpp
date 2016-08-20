#pragma once
#include <engine/graphics/ogl/oglinclude.hpp>
#include <engine/util/defines.hpp>
#include <engine/data_struct/Optional.hpp>
#include <engine/os/log.hpp>
#include <engine/graphics/Graphics.hpp>
namespace GL
{
	using namespace Options;
	using namespace Graphics;
	struct Program
	{
		uint prog = 0;
		uint vert = 0;
		uint frag = 0;
		void bind()
		{
			glUseProgram( prog );
		}
		void release()
		{
			if( prog )
			{
				glDeleteShader( vert );
				glDeleteShader( frag );
				glDeleteProgram( prog );
				vert = 0;
				frag = 0;
				prog = 0;
			}
		}
		static void bindDefault()
		{
			glUseProgram( 0 );
		}
		static Result< Program > create( char const *frag_text , char const *vert_text , bool bind_std = false )
		{
			auto compile = []( uint type , char const *raw ) -> Result< uint >
			{
				uint out = glCreateShader( type );
				glShaderSource( out , 1 , &raw , NULL );
				glCompileShader( out );
				GLint compiled;
				glGetShaderiv( out , GL_COMPILE_STATUS , &compiled );
				if( !compiled )
				{
					GLint length;
					glGetShaderiv( out , GL_INFO_LOG_LENGTH , &length );
					char *log = new char[ length ];
					glGetShaderInfoLog( out , length , &length , log );
					OS::IO::debugLogln( log );
					glDeleteShader( out );
					delete[] log;
					return Result< uint >();
				}
				return Result< uint >( out );
			};
			auto frag_result = compile( GL_FRAGMENT_SHADER , frag_text );
			if( !frag_result.isPresent() )
			{
				return Result< Program >();
			}
			auto vert_result = compile( GL_VERTEX_SHADER , vert_text );
			if( !vert_result.isPresent() )
			{
				return Result< Program >();
			}
			uint vert = vert_result.getValue();
			uint frag = frag_result.getValue();
			uint prog = glCreateProgram();
			if( bind_std )
			{
				glBindAttribLocation( prog , ( uint )AttributeSlot::POSITION , "v_position" );
				glBindAttribLocation( prog , ( uint )AttributeSlot::NORMAL , "v_normal" );
			}
			Program out;
			out.frag = frag;
			out.vert = vert;
			out.prog = prog;
			glAttachShader( prog , frag );
			glAttachShader( prog , vert );
			glLinkProgram( prog );
			GLint compiled;
			glGetProgramiv( prog , GL_LINK_STATUS , &compiled );
			if( !compiled )
			{
				GLint length;
				glGetProgramiv( prog , GL_INFO_LOG_LENGTH , &length );
				char *log = new char[ length ];
				glGetProgramInfoLog( prog , length , &length , &log[ 0 ] );
				OS::IO::debugLogln( log );
				glDeleteShader( vert );
				glDeleteShader( frag );
				glDeleteProgram( prog );
				delete[] log;
				return Result< Program >();
			}
			
			return Result< Program >( std::move( out ) );
		}
	};
}