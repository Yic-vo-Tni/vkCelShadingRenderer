//
// Created by lenovo on 5/4/2024.
//

#ifndef VKMMD_MODELDATA_H
#define VKMMD_MODELDATA_H


namespace yic {

    struct yicVertex {
        glm::vec3 pos;
        glm::vec3 nor;
        glm::vec2 uv;
    };

    struct AABB {
        glm::vec3 min{FLT_MAX};
        glm::vec3 max{-FLT_MAX};
    };

    struct yicSubMesh {
        std::vector <yicVertex> vertices;
        std::vector <uint32_t> indices;

        allocManager::bufSptr vertBuf;
        allocManager::bufSptr indexBuf;
        genericTexManagerSptr diffuseTex;
        AABB meshAABB;
    };

    struct yicObject {
        std::vector <yicSubMesh> subMeshes;
        allocManager::bufSptr mvpBuf{};
        AABB objAABB{};
        glm::mat4 transform{1.f};
    };

    struct yicModel {
        std::vector <yicObject> objects;
        AABB modelAABB;
        glm::mat4 transform{1.f};
        glm::vec3 translation{0.f};
        //, scale{1.f};
        float uniformScale{1.f};
    };


}

#endif //VKMMD_MODELDATA_H
