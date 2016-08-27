#pragma once
#pragma once
#pragma once
#include <engine/graphics/common/RendererImpl.hpp>
#include <engine/graphics/Renderer.hpp>
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/data_struct/Buffer.hpp>
namespace VK
{
	using namespace Graphics;
	using namespace Collections;
	class RendererVK : public RendererImpl , public Renderer
	{
	public:
#ifdef _WIN32
		HDC hdc;
#endif
		uint pushCreationQueue( CreationDesc desc );
		void mainloop();
	};
}