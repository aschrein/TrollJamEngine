#pragma once
#include <engine/util/defines.hpp>
#include <engine/math/vec.hpp>
#include <engine/math/mat.hpp>
#include <engine/assets/Mesh.hpp>
#include <engine/mem/Pointers.hpp>
#include <engine/data_struct/RingBuffer.hpp>
#include <engine/graphics/Shader.hpp>
#include <engine/graphics/Buffers.hpp>
#include <engine/graphics/Textures.h>
namespace Graphics
{
	using namespace Assets::Mesh;
	using namespace Math;
	
	enum class FillType : uint
	{
		FILL , FILL_BACK , FILL_FRONT , WIRE
	};
	enum class PrimitiveType : uint
	{
		TRIANGLES , LINES , PATCHES , QUADS , POINTS
	};
	struct DrawMeshInfo
	{
		LocalArray< BufferRef , 10 > vertex_buffers;
		LocalArray< BufferRef , 10 > uniform_buffers;
		IndexType index_type;
		BufferRef index_buffer;
		uint start_index;
		uint index_count;
		uint start_instance;
		uint instance_count;
		uint vertex_offset;
		PrimitiveType primitive_type;
		LocalArray< TextureRef , 4 > material_textures;
		float normalized_distance_from_camera;
	};
	class CommandBuffer
	{
	public:
		void drawIndexed( DrawMeshInfo const *info );
		void presentTarget( uint rt_hndl );
		void fillBuffer( uint dst_buffer , void const *data , uint size );
		void fence();
		void *allocate( uint size );
	};
	class CommandPool
	{
	public:
		CommandBuffer *createCommandBuffer();
	};
	class RenderingBackend;
	class UniqueHandler
	{
	private:
		uint handler;
		RenderingBackend *queue;
		void( RenderingBackend::*deleter )( uint );
	public:
		UniqueHandler() = default;
		UniqueHandler( UniqueHandler  const & ) = delete;
		UniqueHandler &operator=( UniqueHandler  const & ) = delete;
		UniqueHandler( UniqueHandler &&p )
		{
			*this = std::move( p );
		}
		UniqueHandler &operator=( UniqueHandler &&p )
		{
			handler = p.handler;
			queue = p.queue;
			deleter = p.deleter;
			p.handler = 0;
			p.queue = 0;
			p.deleter = 0;
			return *this;
		}
		uint const &operator*() const
		{
			return handler;
		}
		void release()
		{
			if( deleter )
			{
				( *queue.*deleter )( handler );
			}
			handler = 0;
			queue = 0;
			deleter = 0;
		}
		~UniqueHandler()
		{
			release();
		}
	};
	struct PointLightPass
	{
		uint frame_buffer_size;
	};
	enum class BlendType
	{
		ONE , SRC_ALPHA , ONE_MINUS_SRC_ALPHA , ZERO
	};
	enum class DepthTestType
	{
		LESS , EQUAL , LEQUAL , NONE
	};
	enum class FrontFace
	{
		CLOCKWISE , COUNTER_CLOCKWISE
	};
	struct PassCreateInfo
	{
		LocalArray< uint , 10 > render_targets;
		uint depth_stencil_target;
		bool use_depth_stencil;
		DepthTestType depth_test_type;
		PrimitiveType primitive_type;
		BlendType src_blend_type;
		BlendType dst_blend_type;
		uint shader_handler;
		bool use_blend;
		bool wireframe;
		float line_width;
		bool cull_face;
		FrontFace front_face;
		uint4 viewport_rect;
		uint vertex_layout;
		LocalArray< AttributeInfo , 10 > vertex_attribute_layout;
		LocalArray< uint , 10 > vertex_buffer_binding_strides;
		LocalArray< UniformInfo , 10 > uniform_layout;
	};
	class RenderingBackend
	{
		friend class Windows;
	private:
		
	public:
		NONMOVABLE( RenderingBackend );
		bool isReady();
		void wait();
		CommandPool *createCommandPool( uint pass_handler );
		void render( CommandPool *cmd_pool );

		uint createBuffer( BufferCreateInfo info );
		uint createTexture( TextureCreateInfo info );
		uint createTextureView( TextureViewCreateInfo info );
		uint createRenderTarget( RenderTargetCreateInfo info );
		uint createDepthStencilTarget( DepthStencilTargetCreateInfo info );
		uint createPass( PassCreateInfo info );
		uint createSampler( SamplerCreateInfo info );
		uint createShader( ShaderCreateInfo info );

		/*void freeVertexBuffer( uint hndl );
		void freeIndexBuffer( uint hndl );
		void freeTexture( uint hndl );
		void freeTextureView( uint hndl );
		void freeSampler( uint hndl );
		void freePass( uint hndl );
		void freeRenderTarget( uint hndl );*/
	};
}
