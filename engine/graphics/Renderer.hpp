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
		friend class OS::Window;
	protected:
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
#ifdef _WIN32
		HGLRC oglcontext;
		HDC hdc;
#endif
		NONMOVABLE( Renderer );
		uint pushCreationQueue( CreationDesc desc );
	public:
		bool isReady()
		{
			return ready_flag.isSet();
		}
		void wait()
		{
			//if( !ready_flag.isSet() )
			{
				ready_signal.wait();
			}
			ready_signal.reset();
		}
		Allocators::Allocator *getAuxiliaryAllocator()
		{
			return swap_auxiliary_allocator;
		}
		void render()
		{
			render_signal.signal();
		}
		void pushCommand( CommandBuffer cmd_buffer )
		{
			command_queue.push( cmd_buffer );
		}
		uint createVertexBuffer( AttributeBufferDesc const *attribute_buffer_desc )
		{
			return pushCreationQueue( { attribute_buffer_desc , CreationType::ATTRIBUTE_BUFFER } );
		}
		uint createIndexBuffer( IndexBufferDesc *index_buffer_desc )
		{
			return pushCreationQueue( { index_buffer_desc , CreationType::INDEX_BUFFER } );
		}
		uint createTexture( TextureDesc const *bitmap )
		{
			return pushCreationQueue( { bitmap , CreationType::TEXTURE } );
		}
		i2 getScreenSize();
		void freeVertexBuffer( uint vb );
		void freeVertexLayout( uint vl );
		void freeIndexBuffer( uint ib );
		void freeTexture( uint tx );
		void mainloop();
		~Renderer()
		{
		}
		static void operator delete( void* ptr )
		{
			Renderer* r = ( Renderer* )ptr;
			r->working_flag.reset();
			r->render();
			r->wait();
			r->~Renderer();
			delete r->auxiliary_allocator;
			delete r->swap_auxiliary_allocator;
			r->allocator->free( r );
			r->allocator->zero( r , 1 );
		}
	};
}