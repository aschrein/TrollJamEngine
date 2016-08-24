#include <stdafx.h>
#include <engine/graphics/vulkan/RendererVK.hpp>
namespace Graphics
{
	using namespace VK;
	uint Renderer::createVertexBuffer( AttributeBufferDesc const *attribute_buffer_desc )
	{
		RendererVK* thisgl = ( RendererVK* )this;
		return thisgl->pushCreationQueue( { attribute_buffer_desc , CreationType::ATTRIBUTE_BUFFER } );
	}
	uint Renderer::createIndexBuffer( IndexBufferDesc *index_buffer_desc )
	{
		RendererVK* thisgl = ( RendererVK* )this;
		return thisgl->pushCreationQueue( { index_buffer_desc , CreationType::INDEX_BUFFER } );
	}
	uint Renderer::createTexture( TextureDesc const *bitmap )
	{
		RendererVK* thisgl = ( RendererVK* )this;
		return thisgl->pushCreationQueue( { bitmap , CreationType::TEXTURE } );
	}
	bool Renderer::isReady()
	{
		RendererVK* thisgl = ( RendererVK* )this;
		return thisgl->ready_flag.isSet();
	}
	void Renderer::wait()
	{
		RendererVK* thisgl = ( RendererVK* )this;
		//if( !ready_flag.isSet() )
		{
			thisgl->ready_signal.wait();
		}
		thisgl->ready_signal.reset();
	}
	Allocators::Allocator *Renderer::getAuxiliaryAllocator()
	{
		RendererVK* thisgl = ( RendererVK* )this;
		return thisgl->swap_auxiliary_allocator;
	}
	void Renderer::render()
	{
		RendererVK* thisgl = ( RendererVK* )this;
		thisgl->render_signal.signal();
	}
	void Renderer::pushCommand( CommandBuffer cmd_buffer )
	{
		RendererVK* thisgl = ( RendererVK* )this;
		thisgl->command_queue.push( cmd_buffer );
	}
}
namespace OS
{
	Renderer *Window::createRenderer( Allocators::Allocator *allocator )
	{
		using namespace VK;
		RendererVK *rvk = allocator->alloc< RendererVK >();
		new( rvk ) RendererVK();
		rvk->allocator = allocator;
		rvk->wnd = this;
		rvk->working_flag.set();
		rvk->hdc = hdc;
		HWND window_handler = this->hwnd;
		HINSTANCE window_instance = this->hinstance;
		rvk->thread = Thread::create(
			[ = ]()
		{
			{
				VkApplicationInfo vkappinfo;
				Allocator::zero( &vkappinfo );
				vkappinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				vkappinfo.pApplicationName = "vk_app";
				vkappinfo.pEngineName = "vk_engine";
				vkappinfo.apiVersion = VK_API_VERSION_1_0;
				VkInstanceCreateInfo vkinstinfo;
				Allocator::zero( &vkinstinfo );
				vkinstinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				vkinstinfo.pApplicationInfo = &vkappinfo;
				char* extension_names[] =
				{
					VK_KHR_SURFACE_EXTENSION_NAME ,
					VK_KHR_WIN32_SURFACE_EXTENSION_NAME
				};
				vkinstinfo.enabledExtensionCount = 2;
				vkinstinfo.ppEnabledExtensionNames = extension_names;
				auto res = vkCreateInstance( &vkinstinfo , nullptr , &rvk->instance );
				if( res != VK_SUCCESS )
				{
					OS::IO::debugLogln( "error while creating VK instance" );
					return;
				}
			}
			{
				{
					uint32_t phys_dev_count = 0;
					VkPhysicalDevice all_phys_devices[ 10 ];
					Allocator::zero( all_phys_devices );
					vkEnumeratePhysicalDevices( rvk->instance , &phys_dev_count , nullptr );
					auto res = vkEnumeratePhysicalDevices( rvk->instance , &phys_dev_count , all_phys_devices );
					if( res != VK_SUCCESS )
					{
						OS::IO::debugLogln( "error while Enumerating VK devices" );
						return;
					}
					rvk->pdev = all_phys_devices[ 0 ];
				}
				VkDeviceQueueCreateInfo vkdevqueueinfo;
				uint32_t queue_index = 0;
				uint32_t queue_count;
				{
					VkQueueFamilyProperties all_queue_prop[ 10 ];
					Allocator::zero( all_queue_prop );
					vkGetPhysicalDeviceQueueFamilyProperties( rvk->pdev , &queue_count , nullptr );
					vkGetPhysicalDeviceQueueFamilyProperties( rvk->pdev , &queue_count , all_queue_prop );
					ito( queue_count )
					{
						if( all_queue_prop[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT )
						{
							queue_index = i;
							break;
						}
					}
					Allocator::zero( &vkdevqueueinfo );
					float queue_priorities[] = { 0.0f };
					vkdevqueueinfo.queueFamilyIndex = queue_index;
					vkdevqueueinfo.queueCount = 1;
					vkdevqueueinfo.pQueuePriorities = queue_priorities;
				}
				{
					VkDeviceCreateInfo vkdevinfo;
					Allocator::zero( &vkdevinfo );
					vkdevinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
					vkdevinfo.queueCreateInfoCount = 1;
					vkdevinfo.pQueueCreateInfos = &vkdevqueueinfo;
					auto res = vkCreateDevice( rvk->pdev , &vkdevinfo , nullptr , &rvk->dev );
					if( res != VK_SUCCESS )
					{
						OS::IO::debugLogln( "error while creating VK device" );
						return;
					}
					vkGetPhysicalDeviceProperties( rvk->pdev , &rvk->pdev_prop );
					vkGetPhysicalDeviceFeatures( rvk->pdev , &rvk->pdev_features );
					vkGetPhysicalDeviceMemoryProperties( rvk->pdev , &rvk->pdev_mem_prop );
					vkGetDeviceQueue( rvk->dev , queue_index , 0 , &rvk->dev_queue );
				}
				{
					VkFormat depth_formats[] = {
						VK_FORMAT_D32_SFLOAT_S8_UINT,
						VK_FORMAT_D32_SFLOAT,
						VK_FORMAT_D24_UNORM_S8_UINT,
						VK_FORMAT_D16_UNORM_S8_UINT,
						VK_FORMAT_D16_UNORM
					};
					VkFormat acceptable_format = VK_FORMAT_UNDEFINED;
					for( auto& format : depth_formats )
					{
						VkFormatProperties formatProps;
						vkGetPhysicalDeviceFormatProperties( rvk->pdev , format , &formatProps );
						if( formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
						{
							acceptable_format = format;
							break;
						}
					}
					if( acceptable_format == VK_FORMAT_UNDEFINED )
					{
						OS::IO::debugLogln( "could not find acceptable depth format" );
						return;
					}
					rvk->depth_format = acceptable_format;
				}
				{
					VkWin32SurfaceCreateInfoKHR surface_create_info;
					Allocator::zero( &surface_create_info );
					surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
					surface_create_info.hinstance = window_instance;
					surface_create_info.hwnd = window_handler;
					auto res = vkCreateWin32SurfaceKHR( rvk->instance , &surface_create_info , nullptr , &rvk->surface );
					if( res != VK_SUCCESS )
					{
						OS::IO::debugLogln( "error while creating VK surface" );
						return;
					}
					{
						//in case of that, it is possible to use separate graphics and present queues
						VkBool32 present_support;
						vkGetPhysicalDeviceSurfaceSupportKHR( rvk->pdev , queue_index , rvk->surface , &present_support );
						if( !present_support )
						{
							OS::IO::debugLogln( "error VK: graphics queue does not support present" );
							return;
						}
					}
					{
						uint32_t format_count;
						vkGetPhysicalDeviceSurfaceFormatsKHR( rvk->pdev , rvk->surface , &format_count , NULL );
						VkSurfaceFormatKHR surface_formats[ 20 ];
						vkGetPhysicalDeviceSurfaceFormatsKHR( rvk->pdev , rvk->surface , &format_count , surface_formats );
						if( surface_formats[ 0 ].format == VK_FORMAT_UNDEFINED )
						{
							rvk->color_format = VK_FORMAT_B8G8R8A8_UNORM;
						} else
						{
							rvk->color_format = surface_formats[ 0 ].format;
						}
						rvk->color_space = surface_formats[ 0 ].colorSpace;
					}
					{
						VkSurfaceCapabilitiesKHR surface_capabilities;
						vkGetPhysicalDeviceSurfaceCapabilitiesKHR( rvk->pdev , rvk->surface , &surface_capabilities );
						uint32_t present_modes_count;
						vkGetPhysicalDeviceSurfacePresentModesKHR( rvk->pdev , rvk->surface , &present_modes_count , nullptr );
						VkPresentModeKHR present_modes[ 10 ];
						vkGetPhysicalDeviceSurfacePresentModesKHR( rvk->pdev , rvk->surface , &present_modes_count , present_modes );
						VkExtent2D swap_chain_extent;
						if( surface_capabilities.currentExtent.width == -1 )
						{
							swap_chain_extent.width = 512;
							swap_chain_extent.height = 512;
						} else
						{
							swap_chain_extent = surface_capabilities.currentExtent;
						}
						VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
						ito( present_modes_count )
						{
							if( present_modes[ i ] == VK_PRESENT_MODE_MAILBOX_KHR )
							{
								present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
								break;
							} else if( present_modes[ i ] == VK_PRESENT_MODE_IMMEDIATE_KHR )
							{
								present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
							}
						}
						uint32_t desired_images_count = Math::MathUtil< uint32_t >::min( surface_capabilities.minImageCount + 1 , surface_capabilities.maxImageCount );
						VkSurfaceTransformFlagsKHR surface_transform_flags;
						if( surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR )
						{
							surface_transform_flags = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
						} else
						{
							surface_transform_flags = surface_capabilities.currentTransform;
						}
						{
							VkSwapchainCreateInfoKHR swap_chain_create_info;
							Allocator::zero( &swap_chain_create_info );
							swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
							swap_chain_create_info.surface = rvk->surface;
							swap_chain_create_info.minImageCount = desired_images_count;
							swap_chain_create_info.imageFormat = rvk->color_format;
							swap_chain_create_info.imageColorSpace = rvk->color_space;
							swap_chain_create_info.imageExtent = swap_chain_extent;
							swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
							swap_chain_create_info.preTransform = ( VkSurfaceTransformFlagBitsKHR )surface_transform_flags;
							swap_chain_create_info.imageArrayLayers = 1;
							swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
							swap_chain_create_info.presentMode = present_mode;
							swap_chain_create_info.clipped = VK_TRUE;
							swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
							auto res = vkCreateSwapchainKHR( rvk->dev , &swap_chain_create_info , nullptr , &rvk->swap_chain );
							if( res != VK_SUCCESS )
							{
								OS::IO::debugLogln( "error while creating swap chain" );
								return;
							}
						}
						{
							vkGetSwapchainImagesKHR( rvk->dev , rvk->swap_chain , &rvk->swap_chain_images.size , nullptr );
							auto res = vkGetSwapchainImagesKHR( rvk->dev , rvk->swap_chain , &rvk->swap_chain_images.size , &rvk->swap_chain_images[ 0 ] );
							if( res != VK_SUCCESS )
							{
								OS::IO::debugLogln( "error while querying swap chains images" );
								return;
							}
							/*for( auto &image : rvk->swap_chain_images )
							{
								VkImageViewCreateInfo image_view_info;
								Allocator::zero( &image_view_info );
								image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
								image_view_info.format = rvk->color_format;
								image_view_info.components = {
									VK_COMPONENT_SWIZZLE_R,
									VK_COMPONENT_SWIZZLE_G,
									VK_COMPONENT_SWIZZLE_B,
									VK_COMPONENT_SWIZZLE_A
								};
								image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
								image_view_info.subresourceRange.levelCount = 1;
								image_view_info.subresourceRange.layerCount = 1;
								image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
								{
									VkImageSubresourceRange subresource_range = {};
									subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
									subresource_range.baseMipLevel = 0;
									subresource_range.levelCount = 1;
									subresource_range.layerCount = 1;
								}
								image_view_info.image = image;
								VkImageView image_view;
								auto res = vkCreateImageView( rvk->dev , &image_view_info , nullptr , &image_view );
								if( res != VK_SUCCESS )
								{
									OS::IO::debugLogln( "error while creating view for swap chains presentable images" );
									return;
								}
								rvk->swap_chain_images_views.push( image_view );
							}*/
						}
						{
							VkSemaphoreCreateInfo semaphore_create_info;
							Allocator::zero( &semaphore_create_info );
							semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
							semaphore_create_info.pNext = NULL;
							semaphore_create_info.flags = 0;
							vkCreateSemaphore( rvk->dev , &semaphore_create_info , nullptr , &rvk->semaphores.present_complete );
							vkCreateSemaphore( rvk->dev , &semaphore_create_info , nullptr , &rvk->semaphores.render_complete );

						}
						{
							VkCommandPoolCreateInfo command_pool_create_info = {};
							command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
							command_pool_create_info.queueFamilyIndex = queue_index;
							command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
							auto res = vkCreateCommandPool( rvk->dev , &command_pool_create_info , nullptr , &rvk->command_pool );
							if( res != VK_SUCCESS )
							{
								OS::IO::debugLogln( "error while creating command pool" );
								return;
							}
						}
						{
							VkCommandBufferAllocateInfo cmd_buf_alloc_info = {};
							cmd_buf_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
							cmd_buf_alloc_info.commandPool = rvk->command_pool;
							cmd_buf_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
							cmd_buf_alloc_info.commandBufferCount = rvk->swap_chain_images.size;
							rvk->cmd_buffers_per_image.size = rvk->swap_chain_images.size;
							auto res = vkAllocateCommandBuffers( rvk->dev , &cmd_buf_alloc_info , &rvk->cmd_buffers_per_image[ 0 ] );
							if( res != VK_SUCCESS )
							{
								OS::IO::debugLogln( "error while allocating command buffers" );
								return;
							}
							{
								VkCommandBufferBeginInfo cmd_begin_info;
								Allocator::zero( &cmd_begin_info );
								cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
								cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
								VkClearColorValue clear_color = { { 1.0f, 0.8f, 0.4f, 0.0f } };
								VkImageSubresourceRange image_subresource_range;
								image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
								image_subresource_range.baseArrayLayer = 0;
								image_subresource_range.baseMipLevel = 0;
								image_subresource_range.levelCount = 1;
								image_subresource_range.layerCount = 1;
								ito( rvk->swap_chain_images.size )
								{
									VkImageMemoryBarrier image_barrier_present_to_clear;
									Allocator::zero( &image_barrier_present_to_clear );
									image_barrier_present_to_clear.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
									image_barrier_present_to_clear.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
									image_barrier_present_to_clear.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
									image_barrier_present_to_clear.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
									image_barrier_present_to_clear.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
									image_barrier_present_to_clear.srcQueueFamilyIndex = queue_index;
									image_barrier_present_to_clear.dstQueueFamilyIndex = queue_index;
									image_barrier_present_to_clear.image = rvk->swap_chain_images[ i ];
									image_barrier_present_to_clear.subresourceRange = image_subresource_range;
									VkImageMemoryBarrier image_barrier_clear_to_present;
									Allocator::zero( &image_barrier_clear_to_present );
									image_barrier_clear_to_present.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
									image_barrier_clear_to_present.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
									image_barrier_clear_to_present.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
									image_barrier_clear_to_present.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
									image_barrier_clear_to_present.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
									image_barrier_clear_to_present.srcQueueFamilyIndex = queue_index;
									image_barrier_clear_to_present.dstQueueFamilyIndex = queue_index;
									image_barrier_clear_to_present.image = rvk->swap_chain_images[ i ];
									image_barrier_clear_to_present.subresourceRange = image_subresource_range;

									vkBeginCommandBuffer( rvk->cmd_buffers_per_image[ i ] , &cmd_begin_info );
									vkCmdPipelineBarrier( rvk->cmd_buffers_per_image[ i ] , VK_PIPELINE_STAGE_TRANSFER_BIT , VK_PIPELINE_STAGE_TRANSFER_BIT ,
										0 , 0 , nullptr , 0 , nullptr , 1 , &image_barrier_present_to_clear );
									vkCmdClearColorImage( rvk->cmd_buffers_per_image[ i ] , rvk->swap_chain_images[ i ] , VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ,
										&clear_color , 1 , &image_subresource_range );
									vkCmdPipelineBarrier( rvk->cmd_buffers_per_image[ i ] , VK_PIPELINE_STAGE_TRANSFER_BIT , VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT ,
										0 , 0 , nullptr , 0 , nullptr , 1 , &image_barrier_clear_to_present );
									vkEndCommandBuffer( rvk->cmd_buffers_per_image[ i ] );
								}

							}
						}
						
						
					}
				}
			}
			while( true )
			{
				uint32_t image_index;
				auto res = vkAcquireNextImageKHR( rvk->dev , rvk->swap_chain , UINT64_MAX , rvk->semaphores.present_complete , VK_NULL_HANDLE , &image_index );
				if( res != VK_SUCCESS )
				{
					OS::IO::debugLogln( "error while Acquiring next image from swap_chain" );
					return;
				}
				Allocator::zero( &rvk->submit_info_skelet );
				rvk->submit_info_skelet.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				rvk->submit_info_skelet.pNext = NULL;
				VkPipelineStageFlags pipeline_stage_flag = VK_PIPELINE_STAGE_TRANSFER_BIT;
				rvk->submit_info_skelet.pWaitDstStageMask = &pipeline_stage_flag;
				rvk->submit_info_skelet.pSignalSemaphores = &rvk->semaphores.render_complete;
				rvk->submit_info_skelet.signalSemaphoreCount = 1;
				rvk->submit_info_skelet.pWaitSemaphores = &rvk->semaphores.present_complete;
				rvk->submit_info_skelet.waitSemaphoreCount = 1;
				rvk->submit_info_skelet.commandBufferCount = 1;
				rvk->submit_info_skelet.pCommandBuffers = &rvk->cmd_buffers_per_image[ image_index ];
				vkQueueSubmit( rvk->dev_queue , 1 , &rvk->submit_info_skelet , VK_NULL_HANDLE );
				VkPresentInfoKHR present_info;
				Allocator::zero( &present_info );
				present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
				present_info.waitSemaphoreCount = 1;
				present_info.pWaitSemaphores = &rvk->semaphores.render_complete;
				present_info.pSwapchains = &rvk->swap_chain;
				present_info.swapchainCount = 1;
				present_info.pImageIndices = &image_index;
				vkQueuePresentKHR( rvk->dev_queue , &present_info );
			}
			//rvk->mainloop();
		} , Allocator::singleton
		);
		return rvk;
	}
}
namespace VK
{
	uint RendererVK::pushCreationQueue( CreationDesc desc )
	{
		uint new_handler = 0u;
		/*switch( desc.type )
		{
		case CreationType::INDEX_BUFFER:
		case CreationType::ATTRIBUTE_BUFFER:
			new_handler = buffers.getSize();
			buffers.push( Buffer() );
			break;
		case CreationType::TEXTURE:
			new_handler = textures.getSize();
			textures.push( TextureView() );
			break;
		}
		desc.handler = new_handler;
		creation_queue.push( desc );*/
		return new_handler;
	}
	void RendererVK::mainloop()
	{
		Unique< FileConsumer > local_file_consumer( new FileConsumer() );
		String vert_filename = "shaders/vk/simple.vs";
		String frag_filename = "shaders/vk/simple.fs";
		FileManager::singleton->loadFile( { frag_filename , vert_filename } , local_file_consumer.get() );
		Shared< FileImage > frag_file , vert_file;
		int files = 2;
		while( files )
		{
			FileEvent file_event = local_file_consumer->popEvent( true ).getValue();
			if( file_event.filename == vert_filename )
			{
				vert_file = std::move( file_event.file_result.getValue() );
				files--;
			} else if( file_event.filename == frag_filename )
			{
				frag_file = std::move( file_event.file_result.getValue() );
				files--;
			}
		}
		Result< CommandBuffer > command_res;
		i2 window_size;
		goto wait_section;

		while( working_flag.isSet() )
		{
			window_size = wnd->getSize();
			{
				Result< CreationDesc > res;
				while( ( res = creation_queue.pop() ).isPresent() )
				{
					auto p = res.getValue();
					switch( p.type )
					{
					case CreationType::ATTRIBUTE_BUFFER:
					{
						AttributeBufferDesc const *attribute_buffer_desc = ( AttributeBufferDesc const * )p.data;
					}
					break;
					case CreationType::INDEX_BUFFER:
					{
						IndexBufferDesc const *index_buffer_desc = ( IndexBufferDesc const * )p.data;
					}
					break;
					case CreationType::TEXTURE:
					{
						TextureDesc const *desc = ( TextureDesc const * )p.data;
					}
					break;
					}
				}
			}
			while( ( command_res = command_queue.pop() ).isPresent() )
			{
				auto command_buffer = command_res.getValue();
				ito( command_buffer.getSize() )
				{
					Command const &cmd = command_buffer[ i ];
					switch( cmd.type )
					{
					case CommandType::CLEAR_COLOR:
						break;
					case CommandType::CLEAR_DEPTH:
						break;
					case CommandType::SET_VIEW_PROJ:
					{
						f4x4 *view_proj = ( f4x4 * )cmd.data;
					}
					break;
					case CommandType::DRAW_INDEXED_MESH:
					{
						DrawMeshDesc *desc = ( DrawMeshDesc* )cmd.data;
						Material *material = desc->material;
					}
					break;
					}
				}
			}
			SwapBuffers( hdc );
wait_section:
			auto tmp = auxiliary_allocator;
			auxiliary_allocator = swap_auxiliary_allocator;
			swap_auxiliary_allocator = tmp;
			swap_auxiliary_allocator->reset();
			ready_flag.set();
			ready_signal.signal();
			render_signal.wait();
			render_signal.reset();
			ready_flag.reset();
		}
	}
}