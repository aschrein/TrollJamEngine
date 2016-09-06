#pragma once
#include <engine/util/defines.hpp>
#include <engine/math/vec.hpp>
#include <engine/components/Reflection.hpp>
namespace Graphics
{
	using namespace Math;
	enum class Usage
	{
		STATIC , DYNAMIC
	};
	enum class BufferTarget
	{
		VERTEX_BUFFER , INDEX_BUFFER , UNIFORM_BUFFER
	};
	struct BufferInfo
	{
		void *data;
		uint size;
		Usage usage;
		BufferTarget target;
	};
	enum class IndexType : uint
	{
		UINT32 , UINT16
	};
	enum class AttributeSlot : uint
	{
		POSITION , NORMAL , TANGENT , BINORMAL , TEXCOORD , BONEINDICES , BONEWEIGHTS , MODEL_TRANSFORM , SKELETAL_TRANSFORM , VIEW_PROJ
	};
	struct BufferRef
	{
		uint handler;
		uint offset;
	};
	struct AttributeInfo
	{
		AttributeSlot slot;
		uint offset;
		uint elem_count;
		PlainFieldType src_type;
		bool normalized;
		uint buffer_index;
		bool per_instance;
	};
	struct UniformInfo
	{
		AttributeSlot slot;
		uint offset;
		uint elem_count;
		PlainFieldType src_type;
		bool normalized;
		uint buffer_index;
	};
}