#pragma once
#pragma once
#pragma once
#include <engine/graphics/Graphics.hpp>
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/data_struct/Buffer.hpp>
#include <engine/data_struct/Tuple.hpp>
#include <engine/graphics/vulkan/Queue.hpp>
#include <engine/graphics/vulkan/CommandBuffer.hpp>
#include <engine/graphics/vulkan/ObjectPool.hpp>
#include <engine/graphics/vulkan/Pass.hpp>
#include <engine/os/Window.hpp>
namespace VKInterface
{
	using namespace Collections;
	using namespace OS::Atomic;
	using namespace OS::Async;
	using namespace LockFree;
	class RenderingBackend;
	struct DrawCallInfo
	{
		void const *data;
		void( *dispatch )( RenderingBackend* , void const * );
		uint64_t key;
	};
	class CommandBuffer : public Graphics::CommandBuffer
	{
	public:
		Allocator *allocator;
		Array< DrawCallInfo > draw_calls;
		Pointers::Unique< LinearAllocator > linear_allocator;
		~CommandBuffer() = default;
	};
	class CommandPool : public Graphics::CommandPool
	{
	public:
		Allocator *allocator = Allocator::singleton;
		LocalArray< CommandBuffer , 100 > buffers_per_pass;
		uint pass_handler;
		~CommandPool() = default;
	};
	struct CreationDesc
	{
		void *data;
		void( *dispatch )( RenderingBackend* , void* , uint );
		uint handler;
	};
	class RenderingBackend : public Graphics::RenderingBackend
	{
		friend class Window;
		
	public:
		VK_OBJECT( RenderingBackend );
#ifdef _WIN32
		HDC hdc;
#endif
		friend class OS::Window;
		Pointers::Unique< Thread > thread;
		AtomicFlag working_flag;
		AtomicFlag ready_flag;
		Signal ready_signal = Signal( false );
		Signal render_signal = Signal( false );
		Allocators::Allocator *allocator;
		OS::Window *wnd;
		VK::Device device;
		VK::Instance instance;
		VK::SwapChain swap_chain;
		VK::Memory dev_mem;
		VK::Memory dev_texture_mem;
		VK::Memory host_mem;
		VK::CommandPool cmd_pool;
		VK::CommandBuffer cmd_buf;
		VK::Queue graphics_queue;
		VK::ObjectPool object_pool;
		CommandPool *current_command_pool = nullptr;
		LocalArray< VK::Pass , 1000 > passes;
		//RingBuffer< CommandPool* , 20 > command_queue;
		RingBuffer< CreationDesc , 1000 > creation_queue;
		void mainloop();
	};
}