#pragma once
#include <engine/data_struct/Array.hpp>
#include <engine/graphics/vulkan/Unique.hpp>
#include <engine/graphics/vulkan/Instance.hpp>
#include <engine/graphics/vulkan/Images.hpp>
#include <engine/graphics/vulkan/Pass.hpp>
#include <engine/graphics/vulkan/Pipeline.hpp>
#include <engine/graphics/vulkan/Buffers.hpp>
#include <engine/graphics/vulkan/Device.hpp>
namespace VK
{
	using namespace Collections;
	template< typename T >
	class ObjectPool
	{
	private:
		Array< T > objects;
		Array< uint > free_objects;
	public:
		ObjectPool( Allocator *allocator = Allocator::singleton )
		{
			objects.setAllocator( allocator );
			free_objects.setAllocator( allocator );
		}
		uint push( T const &val )
		{
			auto tmp = val;
			return push( std::move( tmp ) );
		}
		uint push( T &&val )
		{
			if( !free_objects.isEmpty() )
			{
				uint index = free_objects.pop();
				objects[ index ] = std::move( val );
				return index;
			}
			objects.push( std::move( val ) );
			return objects.getSize() - 1;
		}
		T free( int i )
		{
			auto val = std::move( objects[ i ] );
			free_objects.push( i );
			return val;
		}
		T const &operator[]( uint i ) const
		{
			return objects[ i ];
		}
		T &operator[]( uint i )
		{
			return objects[ i ];
		}
	};
	struct EmptyCreateInfo
	{

	};
	struct ShaderModuleCreateInfo
	{
		void const *data;
		uint size;
		VkShaderStageFlagBits stage_flag;
	};
	struct ShaderModuleDesc
	{
		VkShaderModule handle;
		VkShaderStageFlagBits stage_flag;
		LocalArray< uint , 10 > dependant_pipelines;
	};
	struct Texture
	{
		uint image_index;
		uint image_view_index;
		uint sampler_index;
	};
	struct PipelineCreateInfo
	{
		uint renderpass;
		uint subpass;
		LocalArray< uint , 5 > stages;
		LocalArray< DescriptorSetInfo , 5 > desc_info;
		LocalArray< VkPushConstantRange , 5 > push_constants;
		LocalArray< VkVertexInputBindingDescription , 10 > attrib_binding_info;
		LocalArray< VkVertexInputAttributeDescription , 10 > attrib_info;
		VkPrimitiveTopology topology;
		LocalArray< VkPipelineColorBlendAttachmentState , 5 > color_blend_state_info;
	};
	struct PipelineDesc
	{
		PipelineCreateInfo create_info;
		Pipeline pipeline;
	};
	struct ComputePipelineCreateInfo
	{
		uint stage;
		LocalArray< DescriptorSetInfo , 5 > desc_info;
		LocalArray< VkPushConstantRange , 5 > push_constants;
	};
	struct ComputePipelineDesc
	{
		ComputePipelineCreateInfo create_info;
		ComputePipeline pipeline;
	};
	struct ObjectsBlob
	{
		ObjectPool< VkSemaphore > semaphore_pool;
		ObjectPool< ShaderModuleDesc > shader_module_pool;
		ObjectPool< VkSampler > samplers_pool;
		ObjectPool< VkFramebuffer > frame_buffers_pool;
		ObjectPool< VkRenderPass > pass_pool;
		ObjectPool< Image > image_pool;
		ObjectPool< ImageView > image_view_pool;
		ObjectPool< PipelineDesc > pipelines_pool;
		ObjectPool< Buffer > buffer_pool;
		ObjectPool< ComputePipelineDesc > compute_pipelines_pool;
	};
	struct ObjectsCounter
	{
		uint semaphore_counter;
		uint shader_module_counter;
		uint samplers_counter;
		uint frame_buffers_counter;
		uint pass_counter;
		uint image_counter;
		uint image_view_counter;
		uint pipelines_counter;
		uint buffer_counter;
		uint compute_pipelines_counter;
	};
	template< typename T >
	struct HandlerBase {};
	template<>
	struct HandlerBase< ShaderModuleDesc >
	{
		static constexpr uint INDEX = 1;
		typedef ShaderModuleCreateInfo CreateInfo;
		uint index;
		static ShaderModuleDesc create( Device const &dev , ObjectsBlob &blob , ShaderModuleCreateInfo const &create_info )
		{
			VkShaderModuleCreateInfo shader_module_create_info;
			Allocator::zero( &shader_module_create_info );
			shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_module_create_info.codeSize = create_info.size;
			shader_module_create_info.pCode = ( const uint32_t* )create_info.data;
			return{ vkNew( dev.getHandle() , shader_module_create_info ) , create_info.stage_flag ,{} };
		}
		uint release( Device const &dev , ShaderModuleDesc const &desc )
		{
			vkFree( dev.getHandle() , desc.handle );
		}
	};
	template<>
	struct HandlerBase< VkSemaphore >
	{
		static constexpr uint INDEX = 0;
		typedef EmptyCreateInfo CreateInfo;
		uint index;
		static VkSemaphore create( Device const &dev , ObjectsBlob &blob , EmptyCreateInfo const &create_info )
		{
			VkSemaphoreCreateInfo semaphore_create_info;
			Allocator::zero( &semaphore_create_info );
			semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphore_create_info.pNext = NULL;
			semaphore_create_info.flags = 0;
			return Factory< VkSemaphore >::create( dev.getHandle() , semaphore_create_info );
		}
		uint release( Device const &dev , VkSemaphore const &desc )
		{
			vkFree( dev.getHandle() , desc );
		}
	};
	template<>
	struct HandlerBase< VkSampler >
	{
		static constexpr uint INDEX = 2;
		typedef Graphics::SamplerCreateInfo CreateInfo;
		uint index;
		static VkSampler create( Device const &dev , ObjectsBlob &blob , CreateInfo const &info )
		{
			VkSamplerCreateInfo sampler;
			Allocator::zero( &sampler );
			sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			sampler.magFilter = getVK( info.mag_filter );
			sampler.minFilter = getVK( info.min_filter );
			sampler.mipmapMode = info.use_mipmap ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler.addressModeU = getVK( info.u_regime );
			sampler.addressModeV = getVK( info.v_regime );
			sampler.addressModeW = getVK( info.w_regime );
			sampler.compareOp = VK_COMPARE_OP_NEVER;
			sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			return vkNew( dev.getHandle() , sampler );
		}
		uint release( Device const &dev , ShaderModuleDesc const &desc )
		{
			vkFree( dev.getHandle() , desc.handle );
		}
	};
	struct FrameBufferCreateInfo
	{
		VkRenderPass render_pass;
		LocalArray< VkImageView , 5 > attachments_views;
		uint2 size;
	};
	template<>
	struct HandlerBase< VkFramebuffer >
	{
		static constexpr uint INDEX = 3;
		typedef FrameBufferCreateInfo CreateInfo;
		uint index;
		static VkFramebuffer create( Device const &dev , ObjectsBlob &blob , CreateInfo const &info )
		{
			VkFramebufferCreateInfo frame_buffer_create_info;
			Allocator::zero( &frame_buffer_create_info );
			frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frame_buffer_create_info.renderPass = info.render_pass;
			frame_buffer_create_info.attachmentCount = info.attachments_views.size;
			frame_buffer_create_info.pAttachments = &info.attachments_views[ 0 ];
			frame_buffer_create_info.width = info.size.x;
			frame_buffer_create_info.height = info.size.y;
			frame_buffer_create_info.layers = 1;
			return vkNew( dev.getHandle() , frame_buffer_create_info );
		}
		uint release( Device const &dev , ShaderModuleDesc const &desc )
		{
			vkFree( dev.getHandle() , desc.handle );
		}
	};
	struct ImageCreateInfo
	{
		uint width;
		uint height;
		uint mip_levels; 
		uint layers;
		VkImageLayout initial_layout;
		VkFormat format;
		VkImageUsageFlags usage_flags;
	};
	template<>
	struct HandlerBase< Image >
	{
		static constexpr uint INDEX = 5;
		typedef ImageCreateInfo CreateInfo;
		uint index;
		static Image create( Device const &dev , ObjectsBlob &blob , CreateInfo const &info )
		{
			VkImageCreateInfo image_info;
			Allocator::zero( &image_info );
			image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_info.pNext = NULL;
			image_info.imageType = VK_IMAGE_TYPE_2D;
			image_info.format = info.format;
			image_info.extent = { info.width, info.height, 1 };
			image_info.mipLevels = info.mip_levels;
			image_info.arrayLayers = info.layers;
			image_info.samples = VK_SAMPLE_COUNT_1_BIT;
			image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
			image_info.usage = info.usage_flags;
			image_info.flags = 0;
			image_info.initialLayout = info.initial_layout;

			VkMemoryRequirements mem_req;

			VkResult err;
			Image out;
			out.handle = Factory< VkImage >::create( dev.getHandle() , image_info );
			vkGetImageMemoryRequirements( dev.getHandle() , out.handle , &mem_req );
			out.depth = 1;
			out.format = info.format;
			out.height = info.height;
			out.width = info.width;
			out.layout = info.initial_layout;
			out.layers = info.layers;
			out.mip_levels = info.mip_levels;
			out.memory_offset = dev.getMemory( MemoryType::DEV_TEXTURE ).allocate( mem_req.size );
			out.memory_size = mem_req.size;
			out.usage = info.usage_flags;
			vkBindImageMemory( dev.getHandle() , out.handle , dev.getMemory( MemoryType::DEV_TEXTURE ).handle , out.memory_offset );
			return out;
		}
		uint release( Device const &dev , Image const &desc )
		{
			auto &mem = dev.getMemory( MemoryType::DEV_TEXTURE );
			mem.free( desc.memory_offset );
			vkFree( dev.getHandle() , desc.handle );
		}
	};
	struct ImageViewCreateInfo
	{
		Image const &image;
		VkComponentMapping mapping = { VK_COMPONENT_SWIZZLE_R , VK_COMPONENT_SWIZZLE_G , VK_COMPONENT_SWIZZLE_B , VK_COMPONENT_SWIZZLE_A };
		VkImageSubresourceRange subresource_range = { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 };
	};
	template<>
	struct HandlerBase< ImageView >
	{
		static constexpr uint INDEX = 6;
		typedef ImageViewCreateInfo CreateInfo;
		uint index;
		static ImageView create( Device const &dev , ObjectsBlob &blob , CreateInfo const &info )
		{
			VkImageViewCreateInfo image_view_info;
			Allocator::zero( &image_view_info );
			image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_info.format = info.image.format;
			image_view_info.components = info.mapping;
			image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_info.subresourceRange = info.subresource_range;
			image_view_info.image = info.image.handle;
			ImageView image_view;
			image_view.handle = vkNew( dev.getHandle() , image_view_info );
			image_view.range = image_view_info.subresourceRange;
			image_view.format = info.image.format;
			image_view.mapping = info.mapping;
			return image_view;
		}
		uint release( Device const &dev , ImageView const &desc )
		{
			vkFree( dev.getHandle() , desc.handle );
		}
	};
	template<>
	struct HandlerBase< VkRenderPass >
	{
		static constexpr uint INDEX = 4;
		typedef PassInfo CreateInfo;
		uint index;
		static VkRenderPass create( Device const &dev , ObjectsBlob &blob , CreateInfo const &info )
		{
			LocalArray< VkSubpassDescription , 10 > subpasses = {};
			for( auto &sinfo : info.subpasses )
			{
				VkSubpassDescription subpass_desc;
				Allocator::zero( &subpass_desc );
				subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass_desc.colorAttachmentCount = sinfo.color_attachments.size;
				subpass_desc.pColorAttachments = &sinfo.color_attachments[ 0 ];
				if( sinfo.use_depth_stencil )
				{
					subpass_desc.pDepthStencilAttachment = &sinfo.depth_stencil_attachment;
				}
				subpasses.push( subpass_desc );
			}
			VkRenderPassCreateInfo render_pass_info;
			Allocator::zero( &render_pass_info );
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			render_pass_info.attachmentCount = info.attachments.size;
			render_pass_info.pAttachments = &info.attachments[ 0 ];
			render_pass_info.subpassCount = subpasses.size;
			render_pass_info.pSubpasses = &subpasses[ 0 ];

			auto out = vkNew( dev.getHandle() , render_pass_info );
		}
		uint release( Device const &dev , VkRenderPass const &desc )
		{
			vkFree( dev.getHandle() , desc );
		}
	};
	template<>
	struct HandlerBase< PipelineDesc >
	{
		static constexpr uint INDEX = 7;
		typedef PipelineCreateInfo CreateInfo;
		uint index;
		static PipelineDesc create( Device const &dev , ObjectsBlob &blob , CreateInfo const &info )
		{
			LocalArray< Pair< VkShaderStageFlagBits , VkShaderModule > , 5 > stages = {};
			for( auto stage_index : info.stages )
			{
				auto &stage = blob.shader_module_pool[ stage_index ];
				stages.push( { stage.stage_flag , stage.handle } );
			}
			return{ info , vkCreate(
				dev.getHandle() , blob.pass_pool[ info.renderpass ] , info.subpass , stages ,
				info.desc_info , info.push_constants , info.attrib_binding_info , info.attrib_info ,
				info.topology , info.color_blend_state_info
			) };
		}
		uint release( Device const &dev , PipelineDesc const &desc )
		{
			vkFree( dev.getHandle() , desc.pipeline.handle );
		}
	};
	template<>
	struct HandlerBase< ComputePipelineDesc >
	{
		static constexpr uint INDEX = 9;
		typedef ComputePipelineCreateInfo CreateInfo;
		uint index;
		static ComputePipelineDesc create( Device const &dev , ObjectsBlob &blob , CreateInfo const &info )
		{
			auto &stage = blob.shader_module_pool[ info.stage ];
			return ComputePipelineDesc{ info , vkCreate(
				dev.getHandle() , stage.handle ,
				info.desc_info , info.push_constants
			) };
		}
		uint release( Device const &dev , ComputePipelineDesc const &desc )
		{
			vkFree( dev.getHandle() , desc.pipeline.handle );
		}
	};
	struct BufferCreateInfo
	{
		VkBufferUsageFlags usage;
		MemoryType mem_type;
		uint size;
	};
	template<>
	struct HandlerBase< Buffer >
	{
		static constexpr uint INDEX = 8;
		typedef BufferCreateInfo CreateInfo;
		uint index;
		static Buffer create( Device const &dev , ObjectsBlob &blob , CreateInfo const &info )
		{
			Buffer out;
			VkBufferCreateInfo create_info;
			Allocator::zero( &create_info );
			create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			create_info.size = info.size;
			create_info.usage = info.usage;
			out.handle = Factory< VkBuffer >::create( dev.getHandle() , create_info );
			VkMemoryRequirements mem_req;
			vkGetBufferMemoryRequirements( dev.getHandle() , out.handle , &mem_req );
			out.size = info.size;
			out.usage = info.usage;
			out.mem_type = info.mem_type;

			out.offset = dev.getMemory( info.mem_type ).allocate( mem_req.size );
			vkBindBufferMemory( dev.getHandle() , out.handle , dev.getMemory( info.mem_type ).handle , out.offset );
			return out;
		}
		uint release( Device const &dev , Buffer const &desc )
		{
			auto &mem = dev.getMemory( desc.mem_type );
			mem.free( desc.offset );
			vkFree( dev.getHandle() , desc.handle );
		}
	};
	template< typename T >
	struct Handle : public HandlerBase< T >
	{
		typedef typename HandlerBase< T >::CreateInfo CreateInfo;
		static Pair< Handle , T > create( Device const &dev , ObjectsBlob &blob , ObjectsCounter &counter , CreateInfo const &create_info )
		{
			reinterpret_cast< ObjectPool< T > * >( &blob )[ HandlerBase< T >::INDEX ].push( {} );
			return create( dev , blob , allocate( counter ) , create_info );
		}
		static Pair< Handle , T > create( Device const &dev , ObjectsBlob &blob , uint index , CreateInfo const &create_info )
		{
			auto desc = HandlerBase< T >::create( dev , blob , create_info );
			reinterpret_cast<  ObjectPool< T > * >( &blob )[ HandlerBase< T >::INDEX ][ index ] = desc;
			Pair< Handle , T > res;
			res.key.index = index;
			res.value = desc;
			return res;
		}
		static uint allocate( ObjectsCounter &counter )
		{
			return reinterpret_cast< uint * >( &counter )[ HandlerBase< T >::INDEX ]++;
		}
		void release( Device const &dev , ObjectsBlob &blob )
		{
			auto desc = ( ( ObjectPool< T > * )blob )[ HandlerBase< T >::INDEX ].free( index );
			HandlerBase< T >::release( dev , desc );
		}
		ShaderModuleDesc get( ObjectsBlob const &blob )
		{
			return reinterpret_cast<  ObjectPool< T > const * >( &blob )[ HandlerBase< T >::INDEX ][ index ];
		}
	};
}