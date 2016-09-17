#pragma once
#include <engine/graphics/vulkan/defines.hpp>
namespace VK
{
	struct PassInfo
	{
		LocalArray< LocalArray< VkAttachmentReference , 10 > , 10 > subpasses;
		LocalArray< VkAttachmentDescription , 10 > attachments;
		//LocalArray< VkSubpassDependency , 10 > dependencies;
	};
	inline VkAttachmentReference defaultAttachmentRef( uint index )
	{
		return{ index , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	}
	inline VkAttachmentDescription defaultAttachmentDesc( VkFormat format )
	{
		return{ 0 , format , VK_SAMPLE_COUNT_1_BIT , VK_ATTACHMENT_LOAD_OP_CLEAR , VK_ATTACHMENT_STORE_OP_STORE ,
			VK_ATTACHMENT_LOAD_OP_CLEAR , VK_ATTACHMENT_STORE_OP_STORE ,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	}
}