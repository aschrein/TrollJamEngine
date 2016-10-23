#pragma once
#pragma once
#pragma once
#include <engine/graphics/Graphics.hpp>
#include <engine/data_struct/Buffer.hpp>
#include <engine/data_struct/Tuple.hpp>
#include <engine/graphics/vulkan/CommandBuffer.hpp>
#include <engine/graphics/vulkan/Device.hpp>
#include <engine/graphics/vulkan/ObjectPool.hpp>
#include <engine/os/Window.hpp>
namespace VKInterface
{
	using namespace Collections;
	using namespace OS::Atomic;
	using namespace OS::Async;
	using namespace LockFree;
	class RenderingBackend;
	struct GraphicsState
	{
		f4x4 view_proj;
		float3 camera_pos;
		int current_pipeline;
	};
	struct Command
	{
		void( *dispatch )( RenderingBackend * , GraphicsState & , VK::CommandBuffer const & ,
			VK::CommandBuffer const & , uint , void * );
		void *data;
	};
	struct CommandQueue : public Graphics::CommandQueue
	{
		VK::Device *device;
		LinearAllocator temp_allocator;
		LocalArray< Command , 1000 > commands;
		~CommandQueue() = default;
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
		LocalArray< CommandQueue* , 10 > current_command_queues;
		LocalArray< CommandQueue* , 10 > swap_command_queues;
		VK::Device device;
		struct
		{
			VkRenderPass pass;
			VkFramebuffer frame_buffer;
		} opaque_pass;
		VK::ObjectsBlob objects_pool;
		VK::ObjectsCounter objects_counter;
		void mainloop();
		void release();
	};
}