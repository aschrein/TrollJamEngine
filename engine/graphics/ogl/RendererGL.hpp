#pragma once
#pragma once
#include <engine/graphics/common/RendererImpl.hpp>
#include <engine/graphics/Renderer.hpp>
#include <engine/graphics/ogl/TextureGL.hpp>
#include <engine/graphics/ogl/ProgramGL.hpp>
#include <engine/graphics/ogl/BufferGL.hpp>
namespace GL
{
	using namespace Graphics;
	class RendererGL : public RendererImpl , public Renderer
	{
	public:
#ifdef _WIN32
		HGLRC oglcontext;
		HDC hdc;
#endif
		Array< TextureView > textures;
		Array< Buffer > buffers;
		Array< Program > programs;
		uint pushCreationQueue( CreationDesc desc );
		void mainloop();
	};
}