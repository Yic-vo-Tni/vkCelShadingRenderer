//
// Created by lenovo on 3/29/2024.
//

#include "vkRenderPass.h"

namespace yic {

//    vkRenderPass::vkRenderPass() {
//        std::vector<vk::AttachmentDescription> attachmentDescription{{{},
//                                                                             mRenderPassFormat.mColorFormat, vk::SampleCountFlagBits::e1,
//                                                                             vk::AttachmentLoadOp::eLoad,
//                                                                             vk::AttachmentStoreOp::eStore,
//                                                                             vk::AttachmentLoadOp::eDontCare,
//                                                                             vk::AttachmentStoreOp::eDontCare,
//                                                                             vk::ImageLayout::eColorAttachmentOptimal,
//                                                                             vk::ImageLayout::eColorAttachmentOptimal},
//                                                                     {{},
//                                                                             mRenderPassFormat.mDepthFormat, vk::SampleCountFlagBits::e1,
//                                                                             vk::AttachmentLoadOp::eClear,
//                                                                             vk::AttachmentStoreOp::eStore,
//                                                                             vk::AttachmentLoadOp::eDontCare,
//                                                                             vk::AttachmentStoreOp::eDontCare,
//                                                                             vk::ImageLayout::eUndefined,
//                                                                             vk::ImageLayout::eDepthStencilAttachmentOptimal}};
//        std::vector<vk::AttachmentReference> attachmentReference{{0, vk::ImageLayout::eColorAttachmentOptimal},
//                                                                 {1, vk::ImageLayout::eDepthStencilAttachmentOptimal}};
//
//        vk::SubpassDescription subpass{{}, vk::PipelineBindPoint::eGraphics, {}, attachmentReference[0], {}, &attachmentReference[1]};
//
//        vk::RenderPassCreateInfo createInfo{{}, attachmentDescription, subpass, {}};
//        vkCreate([&](){ mRenderPass = mDevice.createRenderPass(createInfo); }, "create renderPass");
//    }



} // yic