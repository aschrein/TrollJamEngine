#pragma once
#include <engine/graphics/ogl/oglinclude.hpp>
#include <engine/data_struct/Buffer.hpp>
#include <malloc.h>
#include <string.h>
#include <engine/graphics/Graphics.hpp>
#include <engine/graphics/ogl/AttributeArray.hpp>
namespace GL
{
	using namespace Graphics;
	struct Buffer
	{
		static const int STEP = 10;
		uint bo;
		uint target;
		int real_size = 0;
		int size = 0;
		uint usage;
		bool bound = false;
		AttributeArray attrib_array;
		uint index_type;
		uint index_size;
		static Buffer createVBO( BufferUsage usage , Attribute const *in_attributes , uint count )
		{
			Buffer out;
			out.attrib_array = AttributeArray( in_attributes , count );
			out.target = GL_ARRAY_BUFFER;
			glCreateBuffers( 1 , &out.bo );
			out.usage = usage == BufferUsage::DYNAMIC_DRAW ? GL_STREAM_DRAW : GL_STATIC_DRAW;
			return out;
		}
		static Buffer createIBO( BufferUsage usage , IndexType index_type )
		{
			Buffer out;
			out.index_size =
				index_type == IndexType::UINT32 ?
				4 :
				index_type == IndexType::UINT16 ?
				2 :
				1;
			out.index_type =
				index_type == IndexType::UINT32 ?
				GL_UNSIGNED_INT :
				index_type == IndexType::UINT16 ?
				GL_UNSIGNED_SHORT :
				GL_UNSIGNED_BYTE;
			out.target = GL_ELEMENT_ARRAY_BUFFER;
			glCreateBuffers( 1 , &out.bo );
			out.usage = usage == BufferUsage::DYNAMIC_DRAW ? GL_STREAM_DRAW : GL_STATIC_DRAW;
			return out;
		}
		uint gethandler()
		{
			return bo;
		}
		void clear()
		{
			resize( 0 );
		}
		void resize( int new_size )
		{
			if( !bound )
			{
				bind();
			}
			if( new_size > real_size )
			{
				int new_real_size = new_size + STEP;
				if( size > 0 )
				{
					void *tmp = malloc( new_real_size );
					auto map = this->map();
					memcpy( tmp , map.getRaw() , size );
					unmap();
					free( tmp );
					glBufferData( target , new_real_size , tmp , usage );
				} else
				{
					glBufferData( target , new_real_size , NULL , usage );
				}
				real_size = new_real_size;
			} else if( new_size < real_size - STEP )
			{
				if( new_size > 0 )
				{
					void *tmp = malloc( new_size );
					auto map = this->map();
					memcpy( tmp , map.getRaw() , size );
					unmap();
					free( tmp );
					glBufferData( target , new_size , tmp , usage );
				} else
				{
					glBufferData( target , new_size , NULL , usage );
				}
				real_size = new_size;
			}
			size = new_size;
		}
		void bind()
		{
			glBindBuffer( target , bo );
			attrib_array.bind();
			bound = true;
		}
		void unbind()
		{
			glBindBuffer( target , bo );
			attrib_array.unbind();
			bound = false;
		}
		Collections::BufferView map( bool invalidate = false )
		{
			if( !bound )
			{
				bind();
			}
			void *ptr;
			if( invalidate )
			{
				ptr = glMapBufferRange( target , 0 , size , GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT );
			} else
			{
				ptr = glMapBufferRange( target , 0 , size , GL_MAP_WRITE_BIT | GL_MAP_READ_BIT );
			}
			return Collections::BufferView( ptr , size );
		}
		void unmap()
		{
			glUnmapBuffer( target );
		}
		void release()
		{
			if( bo )
			{
				glDeleteBuffers( 1 , &bo );
				bo = 0;
			}
		}
	};
}