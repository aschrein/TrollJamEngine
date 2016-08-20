#pragma once
#include <engine/os/Window.hpp>
namespace Graphics
{
	enum class CreationType
	{
		ATTRIBUTE_BUFFER , INDEX_BUFFER , TEXTURE , RENDER_TARGET
	};
	struct CreationDesc
	{
		void const *data;
		CreationType type;
		uint handler;
	};
	class RendererImpl
	{
	public:
		friend class OS::Window;
		Unique< Thread > thread;
		AtomicFlag working_flag;
		AtomicFlag ready_flag;
		Signal ready_signal = Signal( false );
		Signal render_signal = Signal( false );
		Allocators::Allocator *allocator;
		Allocators::LinearAllocator *auxiliary_allocator = new Allocators::LinearAllocator( 4000 );
		Allocators::LinearAllocator *swap_auxiliary_allocator = new Allocators::LinearAllocator( 4000 );
		OS::Window *wnd;
		RingBuffer< CommandBuffer , 20 > command_queue;
		RingBuffer< CreationDesc , 200 > creation_queue;
	};
}