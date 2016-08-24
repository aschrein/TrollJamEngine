#include <stdafx.h>
#include <engine/graphics/ogl/oglinclude.hpp>
#include <engine/graphics/Graphics.hpp>
#include <engine/os/Window.hpp>
#include <engine/assets/FileManager.hpp>
#include <engine/graphics/ogl/RendererGL.hpp>
#include <engine/assets/PipeLine.hpp>
using namespace GL;
#include <GL/wglew.h>
using namespace Assets;
namespace OS
{
	Renderer *Window::createRenderer( Allocators::Allocator *allocator )
	{
		RendererGL *rgl = allocator->alloc< RendererGL >();
		new( rgl ) RendererGL();
		rgl->allocator = allocator;
		rgl->wnd = this;
		rgl->working_flag.set();
		rgl->hdc = hdc;
		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof( PIXELFORMATDESCRIPTOR ) ,
			1 ,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DIRECT3D_ACCELERATED | PFD_DOUBLEBUFFER ,
			PFD_TYPE_RGBA ,
			32 ,
			0 , 0 , 0 , 0 , 0 , 0 ,
			0 ,
			0 ,
			0 ,
			0 , 0 , 0 , 0 ,
			24 ,
			8 ,
			0 ,
			PFD_MAIN_PLANE ,
			0 ,
			0 , 0 , 0
		};
		int pixel_format;
		pixel_format = ChoosePixelFormat( hdc , &pfd );
		SetPixelFormat( hdc , pixel_format , &pfd );
		rgl->oglcontext = wglCreateContext( hdc );
		rgl->thread = Thread::create(
			[ = ]()
		{
			wglMakeCurrent( hdc , rgl->oglcontext );
			glewExperimental = GL_TRUE;
			auto err = glewInit();
			if( GLEW_OK != err )
			{
				std::cout << "glew error:" << glewGetErrorString( err );
			}
			wglSwapIntervalEXT( 1 );
			rgl->mainloop();
			wglDeleteContext( rgl->oglcontext );
		} , Allocator::singleton
		);
		return rgl;
	}
}
namespace Graphics
{
	uint Renderer::createVertexBuffer( AttributeBufferDesc const *attribute_buffer_desc )
	{
		RendererGL* thisgl = ( RendererGL* )this;
		return thisgl->pushCreationQueue( { attribute_buffer_desc , CreationType::ATTRIBUTE_BUFFER } );
	}
	uint Renderer::createIndexBuffer( IndexBufferDesc *index_buffer_desc )
	{
		RendererGL* thisgl = ( RendererGL* )this;
		return thisgl->pushCreationQueue( { index_buffer_desc , CreationType::INDEX_BUFFER } );
	}
	uint Renderer::createTexture( TextureDesc const *bitmap )
	{
		RendererGL* thisgl = ( RendererGL* )this;
		return thisgl->pushCreationQueue( { bitmap , CreationType::TEXTURE } );
	}
	bool Renderer::isReady()
	{
		RendererGL* thisgl = ( RendererGL* )this;
		return thisgl->ready_flag.isSet();
	}
	void Renderer::wait()
	{
		RendererGL* thisgl = ( RendererGL* )this;
		//if( !ready_flag.isSet() )
		{
			thisgl->ready_signal.wait();
		}
		thisgl->ready_signal.reset();
	}
	Allocators::Allocator *Renderer::getAuxiliaryAllocator()
	{
		RendererGL* thisgl = ( RendererGL* )this;
		return thisgl->swap_auxiliary_allocator;
	}
	void Renderer::render()
	{
		RendererGL* thisgl = ( RendererGL* )this;
		thisgl->render_signal.signal();
	}
	void Renderer::pushCommand( CommandBuffer cmd_buffer )
	{
		RendererGL* thisgl = ( RendererGL* )this;
		thisgl->command_queue.push( cmd_buffer );
	}
}
namespace GL
{
	uint RendererGL::pushCreationQueue( CreationDesc desc )
	{
		uint new_handler;
		switch( desc.type )
		{
		case CreationType::INDEX_BUFFER:
		case CreationType::ATTRIBUTE_BUFFER:
			new_handler = buffers.getSize();
			buffers.push( Buffer() );
			break;
		case CreationType::TEXTURE:
			new_handler = textures.getSize();
			textures.push( TextureView() );
			break;
		}
		desc.handler = new_handler;
		creation_queue.push( desc );
		return new_handler;
	}
	void RendererGL::mainloop()
	{
		Unique< FileConsumer > local_file_consumer( new FileConsumer() );
		GL::Program program;
		String vert_filename = "shaders/simple.vs";
		String frag_filename = "shaders/simple.fs";
		//PipeLine< GL::Program > program_pipeline( );
		FileManager::singleton->loadFile( { frag_filename , vert_filename } , local_file_consumer.get() );
		Shared< FileImage > frag_file , vert_file;
		int files = 2;
		while( files )
		{
			FileEvent file_event = local_file_consumer->popEvent( true ).getValue();
			if( file_event.filename == vert_filename )
			{
				vert_file = std::move( file_event.file_result.getValue() );
				files--;
			} else if( file_event.filename == frag_filename )
			{
				frag_file = std::move( file_event.file_result.getValue() );
				files--;
			}
		}
		program = GL::Program::create( ( const char* )frag_file->getView().getRaw() , ( const char* )vert_file->getView().getRaw() , true ).getValue();
		TextureDesc desc;
		BitMap2D bitmap;
		byte data[] = { 0x0000ff };
		bitmap.data = data;
		bitmap.height = 1;
		bitmap.width = 1;
		bitmap.pixel_mapping = PixelMapping::RGB;
		bitmap.pixel_type = PixelType::BYTE;
		desc.bitmap = bitmap;
		desc.mag_filter = MAGFilter::NEAREST;
		desc.min_filter = MINFilter::NONE;
		desc.x_regime = WrapRegime::CLAMP;
		desc.y_regime = WrapRegime::CLAMP;
		TextureView black_texture = Texture2D::create( &desc );
		glEnable( GL_DEPTH_TEST );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );
		Result< CommandBuffer > command_res;
		i2 window_size;
		goto wait_section;

		while( working_flag.isSet() )
		{
			//glFinish();
			window_size = wnd->getSize();
			glViewport( 0 , 0 , window_size.x , window_size.y );
			{
				Result< CreationDesc > res;
				while( ( res = creation_queue.pop() ).isPresent() )
				{
					auto p = res.getValue();
					switch( p.type )
					{
					case CreationType::ATTRIBUTE_BUFFER:
					{
						AttributeBufferDesc const *attribute_buffer_desc = ( AttributeBufferDesc const * )p.data;
						Buffer &buffer = buffers[ p.handler ];
						buffer = Buffer::createVBO( attribute_buffer_desc->usage , attribute_buffer_desc->attributes , attribute_buffer_desc->attributes_count );
						buffer.resize( attribute_buffer_desc->size );
						auto map = buffers[ p.handler ].map();
						//float *tmp = ( float* )p.data;
						memcpy( map.getRaw() , attribute_buffer_desc->data , attribute_buffer_desc->size );
						buffer.unmap();
						buffer.unbind();
						checkError( "CreationType::VERTEX_BUFFER" );
					}
					break;
					case CreationType::INDEX_BUFFER:
					{
						IndexBufferDesc const *index_buffer_desc = ( IndexBufferDesc const * )p.data;
						Buffer &buffer = buffers[ p.handler ];
						buffer = Buffer::createIBO( index_buffer_desc->usage , index_buffer_desc->index_type );
						buffer.resize( index_buffer_desc->size );
						//int *tmp = ( int* )p.data;
						auto map = buffer.map();
						memcpy( map.getRaw() , index_buffer_desc->data , index_buffer_desc->size );
						buffer.unmap();
						buffer.unbind();
						checkError( "CreationType::INDEX_BUFFER" );
					}
					break;
					case CreationType::TEXTURE:
					{
						glEnable( GL_TEXTURE_2D );
						TextureDesc const *desc = ( TextureDesc const * )p.data;
						textures[ p.handler ] = GL::Texture2D::create( desc );
						checkError( "CreationType::INDEX_BUFFER" );
					}
					break;
					}
				}
			}
			
			while( ( command_res = command_queue.pop() ).isPresent() )
			{
				auto command_buffer = command_res.getValue();
				//OS::IO::debugLogln( "render thread poped command buffer" );
				//OS::IO::debugLogln( command_buffer.getSize() );
				ito( command_buffer.getSize() )
				{
					Command const &cmd = command_buffer[ i ];
					switch( cmd.type )
					{
					case CommandType::CLEAR_COLOR:
						glClearColor( 0.0f , 0.0f , 0.0f , 1.0f );
						glClear( GL_COLOR_BUFFER_BIT );
						break;
					case CommandType::CLEAR_DEPTH:
						glClearDepth( 1.0f );
						//glClearStencil( 0 );
						glClear( GL_DEPTH_BUFFER_BIT );
						break;
					case CommandType::SET_VIEW_PROJ:
					{
						program.bind();
						f4x4 *view_proj = ( f4x4 * )cmd.data;
						glUniformMatrix4fv( glGetUniformLocation( program.prog , "view_proj" ) , 1 , GL_TRUE , ( float const * )cmd.data );
						Program::bindDefault();
						checkError( "CommandType::SET_VIEW_PROJ" );
					}
					break;
					case CommandType::DRAW_INDEXED_MESH:
					{
						DrawMeshDesc *desc = ( DrawMeshDesc* )cmd.data;
						program.bind();
						glUniformMatrix4fv( glGetUniformLocation( program.prog , "model" ) , 1 , GL_TRUE , ( float const * )desc->transform );
						Material *material = desc->material;
						if( material && material->has_albedo_texture )
						{
							textures[ desc->material->albedo ].bind( 0 , glGetUniformLocation( program.prog , "albedo" ) );
						}
						auto &ibo = buffers[ desc->index_buffer_handle ];
						ito( desc->vertex_buffers_count )
						{
							auto &vbo = buffers[ desc->vertex_buffer_handle[ i ] ];
							vbo.bind();
						}
						ibo.bind();
						glClearStencil( 0 );
						glClear( GL_STENCIL_BUFFER_BIT );
						glEnable( GL_STENCIL_TEST );
						glStencilFunc( GL_ALWAYS , 1 , -1 );
						glStencilOp( GL_KEEP , GL_KEEP , GL_REPLACE );
						glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
						glDrawElements(
							desc->primitive_type == PrimitiveType::TRIANGLES ?
							GL_TRIANGLES :
							GL_QUADS ,
							desc->count ,
							ibo.index_type
							, ( void* )( desc->start_index * ibo.index_size ) );
						glStencilFunc( GL_NOTEQUAL , 1 , -1 );
						glStencilOp( GL_KEEP , GL_KEEP , GL_REPLACE );
						glLineWidth( 4 );
						glPolygonMode( GL_FRONT_AND_BACK , GL_LINE );
						black_texture.bind( 0 , glGetUniformLocation( program.prog , "albedo" ) );
						glDrawElements(
							desc->primitive_type == PrimitiveType::TRIANGLES ?
							GL_TRIANGLES :
							GL_QUADS ,
							desc->count ,
							ibo.index_type
							, ( void* )( desc->start_index * ibo.index_size ) );
						//glDisableVertexAttribArray( 0 );
						checkError( "CommandType::DRAW_INDEXED" );
					}
					break;
					}
				}
			}
			SwapBuffers( hdc );
wait_section:
			auto tmp = auxiliary_allocator;
			//InterlockedExchangePointer( (  void * volatile * )&auxiliary_allocator , swap_auxiliary_allocator );
			//InterlockedExchangePointer( ( void * volatile * )&swap_auxiliary_allocator , tmp );
			auxiliary_allocator = swap_auxiliary_allocator;
			swap_auxiliary_allocator = tmp;
			swap_auxiliary_allocator->reset();
			//OS::IO::debugLogln( "render thread swaped command buffer" );
			
			ready_flag.set();
			ready_signal.signal();
			render_signal.wait();
			render_signal.reset();
			//OS::IO::debugLogln( "render thread woked up" );
			ready_flag.reset();
		}
	}
}