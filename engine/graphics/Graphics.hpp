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
#include <engine/graphics/Material.hpp>
#include <engine/graphics/Camera.hpp>
#undef TRANSPARENT
#undef OPAQUE
namespace Graphics
{
	using namespace Assets::Mesh;
	using namespace Math;
	/*struct PointLight
	{
		float3 pos;
		float3 color;
		float linear_falloff;
		float quadratic_fallof;
		bool cast_shadows;
	};*/
	struct DrawMeshInfo
	{
		BufferRef vertex_buffer;
		IndexType index_type;
		BufferRef index_buffer;
		uint start_index;
		uint index_count;
		uint pipeline_handle;
		float scale;
		qf rotation;
		/*
		uint material_id;
		LocalArray< Pair< uint , TextureSlot > , 4 > material_textures;
		f4x4 model_transform;
		dqf *bone_transform;
		uint bone_count;
		float3 aabb_pos;
		float3 aabb_size;*/
	};
	struct PipelineCreateInfo
	{
		uint stride;
		LocalArray< AttributeInfo , 10 > attributes;
	};
	class CommandQueue
	{
	public:
		void drawIndexed( DrawMeshInfo const &info );
		//void fillBuffer( uint dst_buffer , void const *data , uint offset , uint size );
		uint createBuffer( BufferCreateInfo const &info );
		uint createPipeline( PipelineCreateInfo const & );// , MaterialCreateInfo const & );
		uint createTexture( TextureCreateInfo const &info );
		//uint createMaterial( MaterialCreateInfo const *info );
		/*
		
		void freeBuffer( uint hndl );
		void freeMaterial( uint hndl );
		void freeTexture( uint hndl );*/
	};
	class RenderingBackend
	{
		friend class Windows;
	private:
		
	public:
		NONMOVABLE( RenderingBackend );
		bool isIdle();
		void waitIdle();
		void setViewCamera( Camera const &camera );
		CommandQueue *createCommandQueue();
		void submitCommandQueue( CommandQueue * );
		void render();
	};
}
