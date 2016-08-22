#pragma once
#include <engine/graphics/Graphics.hpp>
#include <engine/os/Atomic.hpp>
namespace OS
{
	class Window;
}
namespace Graphics
{
	using namespace OS::Atomic;
	class Renderer
	{
	public:
		bool isReady();
		void wait();
		Allocators::Allocator *getAuxiliaryAllocator();
		void render();
		void pushCommand( CommandBuffer cmd_buffer );
		uint createVertexBuffer( AttributeBufferDesc const *attribute_buffer_desc );
		uint createIndexBuffer( IndexBufferDesc *index_buffer_desc );
		uint createTexture( TextureDesc const *bitmap );
		i2 getScreenSize();
		void freeVertexBuffer( uint vb );
		void freeIndexBuffer( uint ib );
		void freeTexture( uint tx );
		//~Renderer();
	};
}