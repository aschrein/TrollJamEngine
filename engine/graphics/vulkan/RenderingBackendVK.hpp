#pragma once
#pragma once
#pragma once
#include <engine/graphics/Graphics.hpp>
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/data_struct/Buffer.hpp>
#include <engine/os/Window.hpp>
namespace VK
{
	using namespace Graphics;
	using namespace Collections;
	enum class CreationType
	{
		VERTEX_BUFFER , INDEX_BUFFER , TEXTURE , TEXTURE_VIEW , SAMPLER , RENDER_TARGET
	};
	struct CreationDesc
	{
		void const *data;
		CreationType type;
		uint handler;
	};
	class RenderingBackendVK : public RenderingBackend
	{
		friend class Window;
		
	public:
		VK_OBJECT( RenderingBackendVK );
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
		//RingBuffer< CommandBufferPool* , 20 > command_queue;
		RingBuffer< CreationDesc , 1000 > creation_queue;
		uint pushCreationQueue( CreationDesc desc );
		void mainloop();
	};
}