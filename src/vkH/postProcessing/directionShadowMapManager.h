//
// Created by lenovo on 5/5/2024.
//

#ifndef VKMMD_DIRECTIONSHADOWMAPMANAGER_H
#define VKMMD_DIRECTIONSHADOWMAPMANAGER_H

#include "shadowMap.h"

namespace yic {

    class directionShadowMapManager {
    public:
        directionShadowMapManager();
        void update(const glm::mat4& modelMatrix);

        [[nodiscard]] inline auto& getSet() const { return mShadowMapDescriptor ;}
        [[nodiscard]] inline auto& getMvpMatrix() const { return mvpMatrix;}

    private:
        vkDescriptor mShadowMapDescriptor{};
        allocManager::bufSptr mLightMatrixBuf{};
        glm::mat4 mvpMatrix{};
    };

} // yic

#endif //VKMMD_DIRECTIONSHADOWMAPMANAGER_H
