#pragma once
#include <engine/util/defines.hpp>
#include <engine/math/vec.hpp>
#include <engine/math/mat.hpp>
#include <engine/assets/Mesh.hpp>
#include <engine/mem/Pointers.hpp>
#include <engine/data_struct/RingBuffer.hpp>
#include <engine/graphics/Shader.hpp>
#include <engine/graphics/Buffers.hpp>
#include <engine/graphics/Shader.hpp>
#include <engine/graphics/Textures.h>
namespace Graphics
{
	using namespace Assets::Mesh;
	using namespace Math;
	
	enum class FillType : uint
	{
		FILL , FILL_BACK , FILL_FRONT , WIRE
	};
	struct PointLightInfo
	{
		float3 position;
		float3 color;
		float linear_falloff;
		float quadratic_falloff;
		uint shadow_render_target;
		bool cast_shadows;
		uint mask_texture_view;
		bool use_mask;
	};
	struct ConeLightInfo
	{
		float3 position;
		float3 direction;
		float3 color;
		float linear_falloff;
		float quadratic_falloff;
		uint shadow_render_target;
		bool cast_shadows;
		uint mask_texture_view;
		bool use_mask;
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
		void putPointLight( PointLightInfo const *info );
		void putConeLight( ConeLightInfo const *info );
		void drawIndexed( DrawMeshInfo const *info );
		void fillBuffer( uint buf_handler , void const *data , uint size );
		void fence();
		void *allocate( uint size );
	};
	class CommandPool
	{
		CommandBuffer *createCommandBuffer( uint pass_id );
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
	struct PassInfo
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
		Pointers::Unique< CommandPool > createCommandPool();
		void render( Pointers::Unique< CommandPool > &&cmd_pool );

		uint createBuffer( BufferInfo info );
		uint createTexture( TextureInfo info );
		uint createTextureView( TextureViewInfo info );
		uint createRenderTarget( RenderTargetInfo info );
		uint createDepthStencilTarget( DepthStencilTargetInfo info );
		uint createPass( PassInfo info );
		uint createSampler( SamplerInfo info );
		uint createSader( Shader info );

		void freeVertexBuffer( uint hndl );
		void freeIndexBuffer( uint hndl );
		void freeTexture( uint hndl );
		void freeTextureView( uint hndl );
		void freeSampler( uint hndl );
		void freePass( uint hndl );
		void freeRenderTarget( uint hndl );
	};
}
