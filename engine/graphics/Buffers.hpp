#pragma once
#include <engine/util/defines.hpp>
#include <engine/math/vec.hpp>
namespace Graphics
{
	using namespace Math;
	enum class Usage
	{
		STATIC , DYNAMIC
	};
	enum class BufferTarget
	{
		VERTEX_BUFFER , INDEX_BUFFER
	};
	struct BufferCreateInfo
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
	struct BufferRef
	{
		uint buffer_handler;
		uint offset;
	};
}