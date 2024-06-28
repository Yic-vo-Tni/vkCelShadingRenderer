//
// Created by lenovo on 12/20/2023.
//

#ifndef VULKAN_VKPIPELINE_H
#define VULKAN_VKPIPELINE_H

namespace yic {

    struct graphicsPipelineState{
        graphicsPipelineState();

        static inline vk::PipelineColorBlendAttachmentState makePipelineColorBlendAttachments(
                vk::ColorComponentFlags colorFlags = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                        | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
                        vk::Bool32 blendEnable = true,
                        vk::BlendFactor srcColorBF = vk::BlendFactor::eSrcAlpha, vk::BlendFactor dstColorBF = vk::BlendFactor::eOneMinusSrcAlpha,
                        vk::BlendOp colorBlendOp = vk::BlendOp::eAdd,
                        vk::BlendFactor srcAlphaBF = vk::BlendFactor::eSrcAlpha, vk::BlendFactor dstAlphaBF = vk::BlendFactor::eOneMinusSrcAlpha,
                        vk::BlendOp alphaBlendOp = vk::BlendOp::eAdd){
            vk::PipelineColorBlendAttachmentState att{blendEnable,
                                                      srcColorBF, dstColorBF, colorBlendOp,
                                                      srcAlphaBF, dstAlphaBF, alphaBlendOp,
                                                      colorFlags};
            return att;
        };
        void updateState();

        graphicsPipelineState& addBindingDescription(const vk::VertexInputBindingDescription& bindingDescription){
            bindingDescriptions.push_back(bindingDescription);
            return *this;
        }
        graphicsPipelineState& addAttributeDescriptions(const std::vector<vk::VertexInputAttributeDescription>& attributeDescription){
            attributeDescriptions.insert(attributeDescriptions.end(), attributeDescription.begin(), attributeDescription.end());
            return *this;
        }
        graphicsPipelineState& addDyState(const std::vector<vk::DynamicState>& state){
            dynamicStates.insert(dynamicStates.end(), state.begin(), state.end());
            return *this;
        }
    public:
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
        vk::PipelineVertexInputStateCreateInfo vertexInputState;
        vk::PipelineDynamicStateCreateInfo dynamicState;
        vk::PipelineViewportStateCreateInfo viewportState;
        vk::PipelineRasterizationStateCreateInfo rasterizationState;
        vk::PipelineMultisampleStateCreateInfo multisampleState;
        vk::PipelineDepthStencilStateCreateInfo depthStencilState;
        vk::PipelineColorBlendStateCreateInfo colorBlendState;

        std::vector<vk::PipelineColorBlendAttachmentState> blendAttachmentStates{};
    private:
        std::vector<vk::DynamicState> dynamicStates{vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};

        std::vector<vk::Viewport> viewports{};
        std::vector<vk::Rect2D> scissors{};
    };

    struct graphicsPipelineGenerator{
    public:
        graphicsPipelineGenerator(vk::Device device, const vk::PipelineLayout& layout, const vk::RenderPass& renderPass, graphicsPipelineState& pipelineState) :
            mDevice(device), mPipelineState(pipelineState){
            createInfo.layout = layout;
            createInfo.renderPass = renderPass;
            init();
        }

        vk::PipelineShaderStageCreateInfo& addShader(const std::string& code, vk::ShaderStageFlagBits flags, const char* entryPoint = "main"){
            std::vector<char> v;
            std::copy(code.begin(), code.end(), std::back_inserter(v));
            return addShader(v, flags, entryPoint);
        };
        template<typename T>
        vk::PipelineShaderStageCreateInfo& addShader(const std::vector<T>& code, vk::ShaderStageFlagBits flags, const char* entryPoint = "main"){
            vk::ShaderModuleCreateInfo shaderModuleCreateInfo{{}, sizeof(T) * code.size(), reinterpret_cast<const uint32_t*>(code.data())};

            vk::ShaderModule shaderModule{mDevice.createShaderModule(shaderModuleCreateInfo)};
            temporaryModules.push_back(shaderModule);

            return addShader(shaderModule, flags, entryPoint);
        };
        vk::PipelineShaderStageCreateInfo& addShader(vk::ShaderModule shaderModule, vk::ShaderStageFlagBits flags, const char* entryPoint = "main"){
            vk::PipelineShaderStageCreateInfo shaderStage{{}, flags, shaderModule, entryPoint};
            shaderStages.push_back(shaderStage);

            return shaderStages.back();
        };

        vk::Pipeline createGraphicsPipeline(){
            update();
            vk::Pipeline gPipeline;
            vkCreate([&](){ gPipeline = mDevice.createGraphicsPipeline({}, createInfo).value; }, "create graphics pipeline");
            return gPipeline;
        }

        vk::GraphicsPipelineCreateInfo createInfo{};
        [[nodiscard]] inline auto& getShaderStages() const { return shaderStages;}
    private:
        vk::Device mDevice;
        graphicsPipelineState& mPipelineState;

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{};
        std::vector<vk::ShaderModule> temporaryModules{};

        void init(){
            createInfo.setPInputAssemblyState(&mPipelineState.inputAssemblyState)
                    .setPVertexInputState(&mPipelineState.vertexInputState)
                    .setPDynamicState(&mPipelineState.dynamicState)
                    .setPViewportState(&mPipelineState.viewportState)
                    .setPRasterizationState(&mPipelineState.rasterizationState)
                    .setPMultisampleState(&mPipelineState.multisampleState)
                    .setPDepthStencilState(&mPipelineState.depthStencilState)
                    .setPColorBlendState(&mPipelineState.colorBlendState);
        }

        void update(){
            createInfo.setStages(shaderStages);
            mPipelineState.updateState();
        }
    };

    struct graphicsPipelineGeneratorCombined : public graphicsPipelineState, public graphicsPipelineGenerator{
        graphicsPipelineGeneratorCombined(vk::Device device, const vk::PipelineLayout& layout, const vk::RenderPass& renderPass)
            : graphicsPipelineState(), graphicsPipelineGenerator(device, layout, renderPass, *this){
        };
    };



} // yic

#endif //VULKAN_VKPIPELINE_H
