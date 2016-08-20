#include <stdafx.h>
#include <ogl/oglinclude.hpp>
#include <view/Graphics.hpp>
#include <os/Window.hpp>
#include <ogl/TextureGL.hpp>
#include <ogl/ProgramGL.hpp>
#include <ogl/BufferGL.hpp>
#include <assets/FileManager.hpp>
#include <view/Renderer.hpp>
#include <view/Mesh.hpp>

namespace Graphics
{
	using namespace Options;
	using namespace GL;
	static Array< TextureView > textures;
	static Array< Buffer > buffers;
	static Array< Program > programs;
	//static Array< 
	uint Renderer::pushCreationQueue( CreationDesc desc )
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
	void Renderer::mainloop()
	{
		Unique< FileConsumer > local_file_consumer( new FileConsumer() );
		GL::Program program;
		String vert_filename = "shaders/simple.vs";
		String frag_filename = "shaders/simple.fs";
		FileManager::singleton->loadFile( { frag_filename , vert_filename } , local_file_consumer.get() );
		FileImage frag_file , vert_file;
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
		program = GL::Program::create( ( const char* )frag_file.getRaw() , ( const char* )vert_file.getRaw() , true ).getValue();
		glEnable( GL_DEPTH_TEST );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );
		Result< CommandBuffer > command_res;
		i2 window_size;
		goto wait_section;

		while( working_flag.isSet() )
		{
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
			glFinish();
			ready_flag.set();
			ready_signal.signal();
			render_signal.wait();
			render_signal.reset();
			//OS::IO::debugLogln( "render thread woked up" );
			ready_flag.reset();
		}
	}
}