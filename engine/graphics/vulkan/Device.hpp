#pragma once
#include <engine/graphics/vulkan/defines.hpp>
#include <engine/graphics/vulkan/Unique.hpp>
#include <engine/graphics/vulkan/Instance.hpp>
#include <engine/graphics/vulkan/ObjectPool.hpp>
#include <engine/graphics/vulkan/Images.hpp>
#include <engine/graphics/vulkan/Pass.hpp>
#include <engine/graphics/vulkan/Pipeline.hpp>
#include <engine/graphics/vulkan/Buffers.hpp>
#include <engine/graphics/Graphics.hpp>
namespace VK
{
	struct ShaderModuleDesc
	{
		VkShaderModule handle;
		VkShaderStageFlagBits stage_flag;
		LocalArray< uint , 10 > dependant_pipelines;
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
	class Device
	{
	public:
		VK_OBJECT( Device );
		VkDevice handle;
		uint32_t graphics_queue_count;
		Instance::PhysicalDevice pdev;
		LocalArray< VkQueue , 10 > graphics_queues , compute_queues;
		ObjectPool< VkSemaphore > semaphore_pool;
		ObjectPool< ShaderModuleDesc > shader_module_pool;
		ObjectPool< VkSampler > samplers_pool;
		ObjectPool< VkFramebuffer > frame_buffers_pool;
		ObjectPool< VkRenderPass > pass_pool;
		ObjectPool< Image > image_pool;
		ObjectPool< ImageView > image_view_pool;
		ObjectPool< PipelineDesc > pipelines_pool;
		ObjectPool< Memory > memory_pool;
		ObjectPool< Buffer > buffer_pool;
		Memory dev_mem;
		Memory dev_texture_mem;
		Memory host_mem;
	public:
		Memory &getMemory( MemoryType mem_type )
		{
			Memory *memory = nullptr;
			switch( mem_type )
			{
			case MemoryType::HOST:
				memory = &host_mem;
				break;
			case MemoryType::DEV_BUFFER:
				memory = &dev_mem;
				break;
			case MemoryType::DEV_TEXTURE:
				memory = &dev_texture_mem;
				break;
			}
			return *memory;
		}
		static Device createGraphicsDevice( Instance const &instance , uint32_t graphics_queue_count = 1 , uint compute_queue_count = 1 )
		{
			float priorities[ 10 ] = { 0.0f };
			VkDeviceQueueCreateInfo vkdevqueueinfo[ 2 ];
			Allocator::zero( vkdevqueueinfo , 2 );
			vkdevqueueinfo[ 0 ].queueFamilyIndex = instance.getPhysicalDevice().getGraphicsQueueFamily();
			vkdevqueueinfo[ 0 ].queueCount = graphics_queue_count;
			vkdevqueueinfo[ 0 ].pQueuePriorities = priorities;
			vkdevqueueinfo[ 1 ].queueFamilyIndex = instance.getPhysicalDevice().getComputeQueueFamily();
			vkdevqueueinfo[ 1 ].queueCount = compute_queue_count;
			vkdevqueueinfo[ 1 ].pQueuePriorities = priorities;
			VkDeviceCreateInfo vkdevinfo;
			Allocator::zero( &vkdevinfo );
			vkdevinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			vkdevinfo.queueCreateInfoCount = 2;
			vkdevinfo.pQueueCreateInfos = vkdevqueueinfo;
			Device out;
			out.pdev = instance.getPhysicalDevice();
			out.handle = Factory< VkDevice >::create( instance.getPhysicalDevice().getHandle() , vkdevinfo );
			ito( graphics_queue_count )
			{
				out.graphics_queues.push( out.getGraphicsQueue( i ) );
			}
			ito( compute_queue_count )
			{
				out.compute_queues.push( out.getComputeQueue( i ) );
			}
			VkMemoryAllocateInfo alloc_info;
			Allocator::zero( &alloc_info );
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = 256 * 100000;
			alloc_info.memoryTypeIndex = instance.getPhysicalDevice().getMemoryIndex( 1665 , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
			out.host_mem.handle = Factory< VkDeviceMemory >::create( out.handle , alloc_info );
			out.host_mem.size = alloc_info.allocationSize;
			out.host_mem.rm.setLimit( alloc_info.allocationSize );

			alloc_info.allocationSize = 256 * 100000;
			alloc_info.memoryTypeIndex = instance.getPhysicalDevice().getMemoryIndex( 1665 , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
			out.dev_mem.handle = Factory< VkDeviceMemory >::create( out.handle , alloc_info );
			out.dev_mem.size = alloc_info.allocationSize;
			out.dev_mem.rm.setLimit( alloc_info.allocationSize );

			alloc_info.allocationSize = 256 * 100000;
			alloc_info.memoryTypeIndex = instance.getPhysicalDevice().getMemoryIndex( 258 , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
			out.dev_texture_mem.handle = Factory< VkDeviceMemory >::create( out.handle , alloc_info );
			out.dev_texture_mem.size = alloc_info.allocationSize;
			out.dev_texture_mem.rm.setLimit( alloc_info.allocationSize );

			out.graphics_queue_count = graphics_queue_count;
			return out;
		}
		Instance::PhysicalDevice const &getPhysicalDevice() const
		{
			return pdev;
		}
		Pair< uint , VkSemaphore > createSemaphore()
		{
			VkSemaphoreCreateInfo semaphore_create_info;
			Allocator::zero( &semaphore_create_info );
			semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphore_create_info.pNext = NULL;
			semaphore_create_info.flags = 0;
			auto out = Factory< VkSemaphore >::create( getHandle() , semaphore_create_info );
			return{ semaphore_pool.push( out ) , out };
		}
		Pair< uint , ShaderModuleDesc > createShaderModule( void const *data , uint size , VkShaderStageFlagBits stage_flag )
		{
			auto new_index = shader_module_pool.push( {} );
			inplaceShaderModule( new_index , data , size , stage_flag );
			return { new_index , shader_module_pool.objects[ new_index ] };
		}
		void inplaceShaderModule( uint index , void const *data , uint size , VkShaderStageFlagBits stage_flag )
		{
			VkShaderModuleCreateInfo shader_module_create_info;
			Allocator::zero( &shader_module_create_info );
			shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_module_create_info.codeSize = size;
			shader_module_create_info.pCode = ( const uint32_t* )data;
			ShaderModuleDesc out = { vkNew( getHandle() , shader_module_create_info ) , stage_flag ,{} };
			shader_module_pool.objects[ index ] = out;
		}
		void releaseShaderModuleInplace( uint index )
		{
			auto module_desc = shader_module_pool.objects[ index ];
			vkFree( handle , module_desc.handle );
		}
		void rebuildShaderModule( uint i , void const *data , uint size )
		{
			auto old_desc = shader_module_pool.objects[ i ];
			releaseShaderModuleInplace( i );
			inplaceShaderModule( i , data , size , old_desc.stage_flag );
			shader_module_pool.objects[ i ].dependant_pipelines = old_desc.dependant_pipelines;
			for( auto dependant_pipe : old_desc.dependant_pipelines )
			{
				rebuildPipline( dependant_pipe );
			}
		}
		Pair< uint , VkSampler > createSampler( Graphics::SamplerCreateInfo const &info )
		{
			VkSamplerCreateInfo sampler;
			Allocator::zero( &sampler );
			sampler.magFilter = getVK( info.mag_filter );
			sampler.minFilter = getVK( info.min_filter );
			sampler.mipmapMode = info.use_mipmap ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler.addressModeU = getVK( info.u_regime );
			sampler.addressModeV = getVK( info.v_regime );
			sampler.addressModeW = getVK( info.w_regime );
			sampler.compareOp = VK_COMPARE_OP_NEVER;
			sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			auto out = Factory< VkSampler >::create( getHandle() , sampler );
			return{ samplers_pool.push( out ) , out };
		}
		Pair< uint , VkFramebuffer > createFrameBuffer(
			VkRenderPass render_pass ,
			LocalArray< VkImageView , 5 > attachments_views , uint2 size
		)
		{
			VkFramebufferCreateInfo frame_buffer_create_info;
			Allocator::zero( &frame_buffer_create_info );
			frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frame_buffer_create_info.renderPass = render_pass;
			frame_buffer_create_info.attachmentCount = attachments_views.size;
			frame_buffer_create_info.pAttachments = &attachments_views[ 0 ];
			frame_buffer_create_info.width = size.x;
			frame_buffer_create_info.height = size.y;
			frame_buffer_create_info.layers = 1;
			auto out = Factory< VkFramebuffer >::create( getHandle() , frame_buffer_create_info );
			return{ frame_buffers_pool.push( out  ) , out };
		}
		Pair< uint , Image > createImage2D(
			uint width , uint height ,
			uint mip_levels , uint layers , VkImageLayout initial_layout , VkFormat format ,
			VkImageUsageFlags usage_flags
		)
		{
			VkImageCreateInfo image_info;
			Allocator::zero( &image_info );
			image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_info.pNext = NULL;
			image_info.imageType = VK_IMAGE_TYPE_2D;
			image_info.format = format;
			image_info.extent = { width, height, 1 };
			image_info.mipLevels = mip_levels;
			image_info.arrayLayers = layers;
			image_info.samples = VK_SAMPLE_COUNT_1_BIT;
			image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
			image_info.usage = usage_flags;
			image_info.flags = 0;
			image_info.initialLayout = initial_layout;

			VkMemoryRequirements mem_req;

			VkResult err;
			Image out;
			out.handle = Factory< VkImage >::create( getHandle() , image_info );
			vkGetImageMemoryRequirements( getHandle() , out.handle , &mem_req );
			out.depth = 1;
			out.format = format;
			out.height = height;
			out.width = width;
			out.layout = initial_layout;
			out.layers = layers;
			out.mip_levels = mip_levels;
			out.memory_offset = dev_texture_mem.allocate( mem_req.size );
			out.memory_size = mem_req.size;
			out.usage = usage_flags;
			vkBindImageMemory( getHandle() , out.handle , dev_texture_mem.handle , out.memory_offset );
			return{ image_pool.push( out ) , out };
		}
		Pair< uint , ImageView > createView2D( Image const &image ,
			VkComponentMapping mapping = { VK_COMPONENT_SWIZZLE_R , VK_COMPONENT_SWIZZLE_G , VK_COMPONENT_SWIZZLE_B , VK_COMPONENT_SWIZZLE_A } ,
			VkImageSubresourceRange subresource_range = { VK_IMAGE_ASPECT_COLOR_BIT , 0 , 1 , 0 , 1 } )
		{
			VkImageViewCreateInfo image_view_info;
			Allocator::zero( &image_view_info );
			image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_info.format = image.format;
			image_view_info.components = mapping;
			image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_info.subresourceRange = subresource_range;
			image_view_info.image = image.handle;
			ImageView image_view;
			image_view.handle = Factory< VkImageView >::create( getHandle() , image_view_info );
			image_view.range = image_view_info.subresourceRange;
			image_view.format = image.format;
			image_view.mapping = mapping;
			return{ image_view_pool.push( image_view ) , image_view };
		}
		Pair< uint , VkRenderPass > createGraphicsRenderPass( PassInfo const &info )
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

			auto out = Factory< VkRenderPass >::create( getHandle() , render_pass_info );
			return{ pass_pool.push( out ) , out };
		}
		void addStageDependency( uint stage , uint pipeline )
		{
			auto &stage_desc = shader_module_pool.objects[ stage ];
			stage_desc.dependant_pipelines.push( pipeline );
		}
		void removeStageDependency( uint stage , uint pipeline )
		{
			auto &stage_desc = shader_module_pool.objects[ stage ];
			ito( stage_desc.dependant_pipelines.size )
			{
				if( stage_desc.dependant_pipelines[ i ] == pipeline )
				{
					stage_desc.dependant_pipelines[ i ] = stage_desc.dependant_pipelines[ stage_desc.dependant_pipelines.size - 1 ];
					stage_desc.dependant_pipelines.size--;
					return;
				}
			}
		}
		void releasePipeline( uint index )
		{
			auto pipeline_desc = pipelines_pool.objects[ index ];
			for( auto stage_index : pipeline_desc.create_info.stages )
			{
				removeStageDependency( stage_index , index );
			}
			releasePipelineInplace( index );
			pipelines_pool.free( index );
		}
		void releasePipelineInplace( uint index )
		{
			auto pipeline_desc = pipelines_pool.objects[ index ];
			auto pipeline = pipeline_desc.pipeline;
			if( pipeline.desc_pool )
			{
				vkFree( handle , pipeline.desc_pool );
			}
			vkFree( handle , pipeline.pipeline_layout );
			for( auto desc_layout : pipeline.desc_set_layouts )
			{
				if( desc_layout )
				{
					vkFree( handle , desc_layout );
				}
			}
			vkFree( handle , pipeline.handle );
		}
		void rebuildPipline( uint index )
		{
			auto pipeline_desc = pipelines_pool.objects[ index ].create_info;
			releasePipelineInplace( index );
			inplacePipeline( pipeline_desc , index );
		}
		void inplacePipeline( PipelineCreateInfo const &info , uint index )
		{
			LocalArray< Pair< VkShaderStageFlagBits , VkShaderModule > , 5 > stages = {};
			for( auto stage_index : info.stages )
			{
				auto &stage = shader_module_pool.objects[ stage_index ];
				stages.push( { stage.stage_flag , stage.handle } );
			}
			auto pipeline = vkCreate(
				handle , pass_pool.objects[ info.renderpass ] , info.subpass , stages ,
				info.desc_info , info.push_constants , info.attrib_binding_info , info.attrib_info ,
				info.topology , info.color_blend_state_info
			);
			pipelines_pool.objects[ index ] = { info , pipeline };
		}
		Pair< uint , Pipeline > createPipeline( PipelineCreateInfo const &info )
		{
			uint new_index = pipelines_pool.push( {} );
			for( auto stage_index : info.stages )
			{
				addStageDependency( stage_index , new_index );
			}
			inplacePipeline( info , new_index );
			return{ new_index , pipelines_pool.objects[ new_index ].pipeline };
		}
		VkQueue getGraphicsQueue( uint index = 0 ) const
		{
			VkQueue queue;
			vkGetDeviceQueue( handle , pdev.getGraphicsQueueFamily() , index , &queue );
			return queue;
		}
		VkQueue getComputeQueue( uint index = 0 ) const
		{
			VkQueue queue;
			vkGetDeviceQueue( handle , pdev.getComputeQueueFamily() , index , &queue );
			return queue;
		}
		Pair< uint , Buffer > createBuffer( VkBufferUsageFlags usage , MemoryType mem_type , uint size )
		{
			Buffer out;
			VkBufferCreateInfo info;
			Allocator::zero( &info );
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = size;
			info.usage = usage;
			out.handle = Factory< VkBuffer >::create( handle , info );
			VkMemoryRequirements mem_req;
			vkGetBufferMemoryRequirements( handle , out.handle , &mem_req );
			out.size = size;
			out.usage = usage;
			out.mem_type = mem_type;
			
			out.offset = getMemory( mem_type ).allocate( mem_req.size );
			vkBindBufferMemory( handle , out.handle , getMemory( mem_type ).handle , out.offset );
			return{ buffer_pool.push( out ) , out };
		}
		void releaseBuffer( uint handle )
		{
			auto buffer = buffer_pool.free( handle );
			auto &mem = getMemory( buffer.mem_type );
			mem.free( buffer.offset );
			VK::Factory< VkBuffer >::release( getHandle() , buffer.handle );
		}
		void *map( Buffer const &buffer )
		{
			void *data;
			vkMapMemory( handle , getMemory( buffer.mem_type ).handle , buffer.offset , buffer.size , 0 , &data );
			return data;
		}
		void unmap( Buffer const &buffer )
		{
			vkUnmapMemory( handle , getMemory( buffer.mem_type ).handle );
		}
		void submitGraphicsCommandBuffer( uint queue_index , CommandBuffer const &command_buffer , VkSemaphore wait_semaphore = VK_NULL_HANDLE , VkSemaphore signal_semaphore = VK_NULL_HANDLE ) const
		{
			VkSubmitInfo submit_info;
			Allocator::zero( &submit_info );
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pNext = NULL;
			VkPipelineStageFlags pipeline_stage_flag = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			submit_info.pWaitDstStageMask = &pipeline_stage_flag;

			if( signal_semaphore )
			{
				submit_info.pSignalSemaphores = &signal_semaphore;
				submit_info.signalSemaphoreCount = 1;
			}
			if( wait_semaphore )
			{
				submit_info.pWaitSemaphores = &wait_semaphore;
				submit_info.waitSemaphoreCount = 1;
			}
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &command_buffer.getHandle();
			VKASSERTLOG( vkQueueSubmit( graphics_queues[ queue_index ] , 1 , &submit_info , VK_NULL_HANDLE ) );
		}
		void present( uint queue_index , VkSwapchainKHR swap_chain , uint image_index , VkSemaphore wait_semaphore = VK_NULL_HANDLE ) const
		{
			VkPresentInfoKHR present_info;
			Allocator::zero( &present_info );
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			if( wait_semaphore )
			{
				present_info.waitSemaphoreCount = 1;
				present_info.pWaitSemaphores = &wait_semaphore;
			}
			present_info.pSwapchains = &swap_chain;
			present_info.swapchainCount = 1;
			present_info.pImageIndices = &image_index;
			VKASSERTLOG( vkQueuePresentKHR( graphics_queues[ queue_index ] , &present_info ) );
		}
		void waitIdleGraphics( uint queue_index ) const
		{
			vkQueueWaitIdle( graphics_queues[ queue_index ] );
		}
		VkDevice getHandle() const
		{
			return handle;
		}
	};
}