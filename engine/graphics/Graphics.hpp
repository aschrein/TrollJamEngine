#pragma once
#include <engine/util/defines.hpp>
#include <engine/math/vec.hpp>
#include <engine/math/mat.hpp>
#include <engine/assets/Mesh.hpp>
#include <engine/mem/Pointers.hpp>
#include <engine/data_struct/RingBuffer.hpp>
namespace Graphics
{
	using namespace Assets::Mesh;
	using namespace OS::Async;
	using namespace Math;
	using namespace Options;
	using namespace Pointers;
	using namespace LockFree;
	enum class Usage
	{
		STATIC , DYNAMIC
	};
	enum class ComponentType : uint
	{
		FLOAT32 , INT , SHORT , BYTE , UNORM8 , UNORM16 , FIVE
	};
	enum class ComponentFormat
	{
		RGB , RGBA , BGR , BGRA , R
	};
	enum class ComponentSwizzle : uint
	{
		R , G , B , A , ONE , ZERO
	};
	enum class RenderTargetType
	{
		COLOR , DEPTH , DEPTH_STENCIL
	};
	enum class DepthFormat
	{
		DEPTH32_UINT
	};
	//default is RGBA
	struct ComponentMapping
	{
		ComponentFormat format;
		ComponentType type = ComponentType::BYTE;
	};
	enum class ImageCompression
	{
		NONE
	};
	static uint getBpp( ComponentMapping mapping )
	{
		bool used[ 4 ] = { false };
		if( mapping.type == ComponentType::FIVE )
		{
			return 2;
		}
		static uint _sizes[] =
		{
			4 , 4 , 2 , 1 , 1 , 2
		};
		static uint _comp[] =
		{
			3 , 4 , 3 , 4 , 1
		};
		return _comp[ ( uint )mapping.format ] * _sizes[ ( uint )mapping.type ];
	}
	//strict linear layout and power of two size
	struct BitMap2D
	{
		void const *data;
		uint size;
		uint width;
		uint height;
		uint depth = 1;
		uint layers_count = 1;
		uint mipmaps_count = 1;
		ImageCompression compression = ImageCompression::NONE;
		ComponentMapping component_mapping;
		uint getBpp() const
		{
			return Graphics::getBpp( component_mapping );
		}
	};
	enum class Filter : int
	{
		NEAREST , LINEAR
	};
	enum class WrapRegime
	{
		CLAMP , REPEAT , MIRROR
	};
	struct SamplerInfo
	{
		Filter mag_filter;
		Filter min_filter;
		bool use_mipmap;
		uint anisotropy_level;
		WrapRegime u_regime;
		WrapRegime v_regime;
		WrapRegime w_regime;
	};
	struct RenderTargetInfo
	{
		ComponentMapping component_mapping;
		uint2 size;
	};
	struct DepthStencilTargetInfo
	{
		uint2 size;
	};
	struct TextureInfo
	{
		BitMap2D bitmap;
		Usage usage;
	};
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
	enum class MaterialTextureTarget
	{
		ALBEDO , NORMAL , METALNESS , ROUGHNESS
	};
	struct MaterialTexture
	{
		uint texture_view;
		uint sampler;
		MaterialTextureTarget target;
	};
	enum class PrimitiveType : uint
	{
		TRIANGLES , LINES , PATCHES , QUADS , POINTS
	};
	enum class IndexType : uint
	{
		UINT32 , UINT16
	};
	enum class AttributeSlot : uint
	{
		POSITION , NORMAL , TANGENT , BINORMAL , TEXCOORD , BONEINDICES , BONEWEIGHTS
	};
	struct DrawMeshInfo
	{
		LocalArray< uint , 10 > vertex_buffer_handles;
		LocalArray< uint , 10 > vertex_buffer_offsets;
		IndexType index_type;
		uint index_buffer_handle;
		uint index_buffer_offset;
		uint start_index;
		uint index_count;
		uint start_instance;
		uint instance_count;
		uint vertex_offset;
		PrimitiveType primitive_type;
		LocalArray< MaterialTexture , 4 > material_textures;
		uint16_t distance_from_camera;
	};
	struct AttributeInfo
	{
		AttributeSlot slot;
		uint offset;
		uint elem_count;
		PlainFieldType src_type;
		bool normalized;
		uint buffer_index;
		bool per_instance;
	};
	enum class UniformSlot
	{
		MODEL_TRANSFORM , SKELETAL_TRANSFORM
	};
	struct UniformInfo
	{
		UniformSlot slot;
		uint offset;
		uint size;
		uint buffer_index;
	};
	enum class BufferTarget
	{
		VERTEX_BUFFER , INDEX_BUFFER , UNIFORM_BUFFER
	};
	struct BufferInfo
	{
		void *data;
		uint size;
		Usage usage;
		BufferTarget target;
	};
	struct TextureViewInfo
	{
		uint texture_handler;
		ComponentSwizzle swizzle[ 4 ];
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
	struct MaterialInfo
	{
		LocalArray< uint , 10 >
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
		uint material_handler;
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

		void freeVertexBuffer( uint hndl );
		void freeIndexBuffer( uint hndl );
		void freeTexture( uint hndl );
		void freeTextureView( uint hndl );
		void freeSampler( uint hndl );
		void freePass( uint hndl );
		void freeRenderTarget( uint hndl );
	};
}
