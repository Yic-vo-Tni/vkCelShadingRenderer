//
// Created by lenovo on 12/20/2023.
//

#include "vkPipeline.h"

namespace yic {

    graphicsPipelineState::graphicsPipelineState() {
        inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList)
                .setPrimitiveRestartEnable({});

        vertexInputState.setVertexAttributeDescriptions({})
                .setVertexBindingDescriptions({});

        dynamicState.setDynamicStates({});

        viewportState.setViewports({})
                .setScissors({});

        rasterizationState.setDepthClampEnable({})
                .setRasterizerDiscardEnable({})
                .setPolygonMode(vk::PolygonMode::eFill)
                .setCullMode(vk::CullModeFlagBits::eBack)
                .setFrontFace(vk::FrontFace::eClockwise)
                .setDepthBiasEnable({})
                .setDepthBiasConstantFactor({})
                .setDepthBiasClamp({})
                .setDepthBiasSlopeFactor({})
                .setLineWidth(1.f);

        multisampleState.setRasterizationSamples(vk::SampleCountFlagBits::e1);

        depthStencilState.setDepthTestEnable(vk::True)
                .setDepthWriteEnable(vk::True)
                .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
                .setDepthBoundsTestEnable({})
                .setStencilTestEnable({})
                .setFront(vk::StencilOpState{})
                .setBack(vk::StencilOpState{})
                .setMinDepthBounds({})
                .setMaxDepthBounds({});

        colorBlendState.setLogicOpEnable({})
            //    .setLogicOp(vk::LogicOp::eClear)
                .setAttachments({});
           //     .setBlendConstants({0.f, 0.f, 0.f, 0.f});
    }

    void graphicsPipelineState::updateState() {
        colorBlendState.setAttachments(blendAttachmentStates);

        dynamicState.setDynamicStates(dynamicStates);

        vertexInputState.setVertexAttributeDescriptions(attributeDescriptions)
                        .setVertexBindingDescriptions(bindingDescriptions);

        if(viewports.empty()){
            viewportState.setViewportCount(1);
        } else{
            viewportState.setViewports(viewports);
        }

        if (scissors.empty()){
            viewportState.setScissorCount(1);
        } else{
            viewportState.setScissors(scissors);
        }
    }



} // yic