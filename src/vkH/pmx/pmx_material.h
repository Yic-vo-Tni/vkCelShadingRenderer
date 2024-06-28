//
// Created by lenovo on 5/2/2024.
//

#ifndef VKMMD_PMX_MATERIAL_H
#define VKMMD_PMX_MATERIAL_H

#include "niLou/vkImage.h"
#include "pmx_header.h"

namespace vkPmx {

    using namespace yic;

    struct effControl{
        bool eBlur{false};
        bool eHSV{false};
        bool eColor{false};
        bool eSpecular{false};
    };

    class pmx_material {
    public:
        explicit pmx_material(const std::string& path, vk::CommandBuffer& cmd);

        void setUniqueTex() {mUniqueTex = true;}
        void updateEffBuf(){
            mUniformEffectBuf->updateBuffer(mFragEffect);
        }

        [[nodiscard]] inline auto& getTexture() const { return mTexture;}
        [[nodiscard]] inline auto& getEffUnifBuf() const { return mUniformEffectBuf;}
        [[nodiscard]] inline auto& getFragEffect()  { return mFragEffect;}

        [[nodiscard]] inline auto& getUniqueTex() const { return mUniqueTex;}
        [[nodiscard]] inline auto& getUniqueEffControl() { return mUniqueEff;}
        [[nodiscard]] inline auto& getEffControl() { return mEffControl;}
    private:
        MMDFragEffect mFragEffect{0.6f, 10.f, 0.2f, 2.3f, 0.f, 1.f,
                                  0.5f, 0,3, 0.4f, 0.35f, 0.1f, 0.03f,
                                  0.f, 0.f, 0.f, 0.f,
                                  0.2f, 0.2f, 0.001f, 1.3f};
        allocManager::bufSptr mUniformEffectBuf{};

        genericTexManagerSptr mTexture;

        std::string name;
        bool mUniqueTex{false};
        effControl mEffControl{};
        bool mUniqueEff{false};
    };

} // yic

#endif //VKMMD_PMX_MATERIAL_H
