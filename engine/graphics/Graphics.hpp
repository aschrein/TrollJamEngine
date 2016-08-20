#pragma once
#include <engine/util/defines.hpp>
#include <engine/math/vec.hpp>
#include <engine/math/mat.hpp>
#include <engine/os/Async.hpp>
#include <engine/assets/Mesh.hpp>
#include <engine/assets/BitMap.hpp>
#include <engine/data_struct/Optional.hpp>
#include <engine/mem/Pointers.hpp>
#include <engine/data_struct/RingBuffer.hpp>
namespace Graphics
{
	using namespace Assets::Mesh;
	using namespace OS::Async;
	using namespace Math;
	using namespace Options;
	using namespace Pointers;
	using namespace LockFree;
	struct Color
	{
		union
		{
			uint base;
			struct
			{
				byte r , g , b , a;
			};
		};
	};
	struct Rect2D
	{
		i2 pos , size;
		bool isIn( i2 const &p ) const
		{
			return p.x > pos.x && p.y > pos.y && p.x < pos.x + size.x && p.y < pos.y + size.y;
		}
	};
	enum class CommandType
	{
		DRAW_INDEXED_MESH , SET_VIEW_PROJ , CLEAR_COLOR , SET_RENDER_TARGET , FENCE , CLEAR_DEPTH , SET_VIEWPORT , BUFFER_APPEND_DATA , BUFFER_SET_DATA
	};
	struct Command
	{
		CommandType type;
		uint64_t key;
		void const *data;
	};
	struct CommandBuffer
	{
		Allocator *allocator;
		Command *base = nullptr;
		uint pos = 0;
		uint limit = 0;
		uint current_fence_length = 0;
		Command *current_fence = nullptr;
		CommandBuffer( uint max_cmd_count , Allocator *allocator ) :
			allocator( allocator ) ,
			base( ( Command * )allocator->alloc( max_cmd_count * sizeof( Command ) ) ) ,
			limit( max_cmd_count )
		{
			current_fence = ( Command* )( base );
			push( { CommandType::FENCE } );
		}
		CommandBuffer() = default;
		//NONMOVABLE( CommandBuffer );
		void push( Command const &command )
		{
			base[ pos++ ] = command;
			current_fence_length++;
		}
		Command const &operator[]( uint i ) const
		{
			return base[ i ];
		}
		void fence()
		{
			current_fence->key = current_fence_length;
			current_fence = ( Command* )( base + pos );
			current_fence_length = 0;
		}
		void *allocate( uint size )
		{
			return allocator->alloc( size );
		}
		int getSize() const
		{
			return pos;
		}
	};
	enum class FillType : uint
	{
		FILL , FILL_BACK , FILL_FRONT , WIRE
	};
	enum class MaterialType : uint
	{
		DEFAULT
	};
	struct Material
	{
		uint albedo;
		uint normal;
		uint specular;
		MaterialType type;
		bool has_albedo_texture;
		bool has_normal_texture;
		bool has_specular__texture;
	};
	enum class PrimitiveType : uint
	{
		TRIANGLES , LINES , PATCHES , QUADS
	};
	enum class IndexType : uint
	{
		UINT32 , UINT16 , UINT8
	};
	enum class AttributeSlot : uint
	{
		POSITION , NORMAL , TANGENT , BINORMAL , TEXCOORD
	};
	struct DrawMeshDesc
	{
		uint *vertex_buffer_handle;
		uint vertex_buffers_count;
		uint index_buffer_handle;
		uint start_index;
		uint count;
		PrimitiveType primitive_type;
		FillType fill_type;
		f4x4 *transform;
		Material *material;
	};
	struct Attribute
	{
		AttributeSlot slot;
		uint offset;
		uint elem_count;
		PlainFieldType src_type;
		bool normalized;
		uint stride;
	};
	enum class BufferUsage
	{
		STATIC_DRAW , DYNAMIC_DRAW
	};
	struct AttributeBufferDesc
	{
		void *data;
		BufferUsage usage;
		uint size;
		Attribute *attributes;
		uint attributes_count;
	};
	struct IndexBufferDesc
	{
		void *data;
		uint size;
		BufferUsage usage;
		IndexType index_type;
	};
}
