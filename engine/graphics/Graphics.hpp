#pragma once
#include <engine/util/defines.hpp>
#include <engine/math/vec.hpp>
#include <engine/math/mat.hpp>
#include <engine/os/Async.hpp>
#include <engine/assets/Mesh.hpp>
#include <engine/data_struct/Optional.hpp>
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
		WrapRegime x_regime;
		WrapRegime y_regime;
		WrapRegime z_regime;
	};
	struct RenderTargetInfo
	{
		RenderTargetType type;
		ComponentMapping component_mapping;
		bool sampled;
		uint width;
		uint height;
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
	enum class MaterialType : uint
	{
		DEFAULT
	};
	struct Material
	{
		uint albedo_texture_view;
		uint normal_texture_view;
		uint roughness_texture_view;
		uint metalness_texture_view;
		MaterialType type;
		bool has_albedo_texture;
		bool has_normal_texture;
		bool has_specular__texture;
	};
	enum class PrimitiveType : uint
	{
		TRIANGLES , LINES , PATCHES , QUADS , POINTS
	};
	enum class IndexType : uint
	{
		UINT32 , UINT16 , UINT8
	};
	enum class AttributeSlot : uint
	{
		POSITION , NORMAL , TANGENT , BINORMAL , TEXCOORD , BONEINDICES , BONEWEIGHTS
	};
	struct DrawMeshInfo
	{
		uint *vertex_buffer_handle;
		uint vertex_buffer_count;
		uint index_buffer_handle;
		uint index_layout_handle;
		uint start_index;
		uint count;
		PrimitiveType primitive_type;
		f4x4 transform;
		qf *skeletal_transform;
		uint bone_count;
		Material *material;
	};
	struct Attribute
	{
		AttributeSlot slot;
		uint offset;
		uint elem_count;
		PlainFieldType src_type;
		bool normalized;
		uint stride;
		uint buffer_index;
	};
	struct VertexLayoutInfo
	{
		Attribute *attributes;
		uint attributes_count;
	};
	struct BufferInfo
	{
		void *data;
		uint size;
		Usage usage;
	};
	struct TextureViewInfo
	{
		uint texture_handler;
		ComponentSwizzle swizzle[ 4 ];
	};
	class CommandBuffer
	{
		friend class CommandBufferPool;
	private:
		NONMOVABLE( CommandBuffer );
	public:
		void drawIndexed( DrawMeshInfo const *info );
		void fillBuffer( uint buf_handler , void const *data , uint size );
		void fillImage( uint img_handler , uint layer , void const *data , uint size );
		void fence();
		void *allocate( uint size );
	};
	class CommandBufferPool
	{
		friend class RenderingBackend;
	private:
		NONMOVABLE( CommandBufferPool );
	public:
		CommandBuffer *createCommandBuffer();
	};
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
	class RenderingBackend
	{
		friend class Windows;
	private:
		
	public:
		NONMOVABLE( RenderingBackend );
		bool isReady();
		void wait();
		void render();
		void pushCommand( CommandBufferPool *cmd );
		uint getSwapBuffersCount() const;
		CommandBufferPool createCommandPool();

		uint createVertexBuffer( BufferInfo const *info );
		uint createIndexBuffer( BufferInfo const *info );
		uint createVertexLayout( VertexLayoutInfo const *info );
		uint createTexture( TextureInfo const *info );
		uint createTextureView( TextureViewInfo const *info );
		uint createRenderTarget( RenderTargetInfo const *info );
		uint createSampler( SamplerInfo const *info );
		void freeVertexBuffer( uint hndl );
		void freeIndexBuffer( uint hndl );
		void freeTexture( uint hndl );
		void freeTextureView( uint hndl );
		void freeVertexLayout( uint hndl );
		void freeSampler( uint hndl );
	};
}
