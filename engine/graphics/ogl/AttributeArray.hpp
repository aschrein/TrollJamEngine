#pragma once
#include <engine/graphics/ogl/oglinclude.hpp>
#include <engine/graphics/Graphics.hpp>
#include <engine/components/Reflection.hpp>
namespace GL
{
	using namespace Graphics;
	struct VertexAttributeGL
	{
		uint location;
		uint offset;
		uint elem_count;
		uint src_type;
		int normalized;
		uint stride;
	};
	struct AttributeArray
	{
		VertexAttributeGL attributes[ 10 ];
		uint count = 0;
		uint getGLType( PlainFieldType type )
		{
			switch( type )
			{
			case PlainFieldType::FLOAT32:
				return GL_FLOAT;
			case PlainFieldType::INT32:
				return GL_INT;
			case PlainFieldType::INT16:
				return GL_SHORT;
			case PlainFieldType::INT8:
				return GL_BYTE;
			case PlainFieldType::UINT16:
				return GL_UNSIGNED_SHORT;
			case PlainFieldType::UINT32:
				return GL_UNSIGNED_INT;
			case PlainFieldType::UINT8:
				return GL_UNSIGNED_BYTE;
			}
		}
		AttributeArray() = default;
		AttributeArray( Attribute const *in_attributes , uint count )
		{
			ito( count )
			{
				attributes[ i ] =
				{
					( uint )in_attributes[ i ].slot ,
					in_attributes[ i ].offset ,
					in_attributes[ i ].elem_count ,
					getGLType( in_attributes[ i ].src_type ) ,
					in_attributes[ i ].normalized ? GL_TRUE : GL_FALSE ,
					in_attributes[ i ].stride
				};
			}
			this->count = count;
		}
		void bind()
		{
			ito( count )
			{
				glEnableVertexAttribArray( attributes[ i ].location );
				glVertexAttribPointer( attributes[ i ].location , attributes[ i ].elem_count , attributes[ i ].src_type , attributes[ i ].normalized , attributes[ i ].stride , ( void * )attributes[ i ].offset );
			}
		}
		void unbind()
		{
			ito( count )
			{
				glDisableVertexAttribArray( attributes[ i ].location );
			}
		}
		uint getAttributeCount()
		{
			return count;
		}
		~AttributeArray()
		{

		}
	};
}