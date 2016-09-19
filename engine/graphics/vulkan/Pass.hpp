#pragma once
#include <engine/graphics/vulkan/defines.hpp>
namespace VK
{
	struct SubpassInfo
	{
		LocalArray< VkAttachmentReference , 10 > color_attachments;
		bool use_depth_stencil;
		VkAttachmentReference depth_stencil_attachment;
	};
	struct PassInfo
	{
		LocalArray< SubpassInfo , 10 > subpasses;
		LocalArray< VkAttachmentDescription , 10 > attachments;
		//LocalArray< VkSubpassDependency , 10 > dependencies;
	};
	inline VkAttachmentReference defaultAttachmentRef( uint index )
	{
		return{ index , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	}
	inline VkAttachmentReference defaultDepthStencilAttachmentRef( uint index )
	{
		return{ index , VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	}
	inline VkAttachmentDescription defaultAttachmentDesc( VkFormat format )
	{
		return{ 0 , format , VK_SAMPLE_COUNT_1_BIT , VK_ATTACHMENT_LOAD_OP_CLEAR , VK_ATTACHMENT_STORE_OP_STORE ,
			VK_ATTACHMENT_LOAD_OP_CLEAR , VK_ATTACHMENT_STORE_OP_STORE ,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	}
	inline VkAttachmentDescription defaultDepthStencilAttachmentDesc( VkFormat format )
	{
		return{ 0 , format , VK_SAMPLE_COUNT_1_BIT , VK_ATTACHMENT_LOAD_OP_CLEAR , VK_ATTACHMENT_STORE_OP_STORE ,
			VK_ATTACHMENT_LOAD_OP_CLEAR , VK_ATTACHMENT_STORE_OP_STORE ,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL , VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	}
}