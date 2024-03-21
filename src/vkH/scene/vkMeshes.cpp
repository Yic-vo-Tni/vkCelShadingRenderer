//
// Created by lenovo on 12/25/2023.
//

#include "vkMeshes.h"

namespace yic {

    vkMeshes::vkMeshes() {
        makeAssets();
    }

    vkMeshes::~vkMeshes() {
        mMeshes.clear();
    }

    void vkMeshes::makeAssets() {
        std::vector<float> vertices = {{
                                               0.0f, -0.1f, 0.0f, 1.0f, 0.0f, 0.5f, 0.0f,
                                               0.1f, 0.1f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                                               -0.1f, 0.1f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
                                       }};
        addToMesh(meshTypes::eTriangle, vertices);
        vertices = {{
                            -0.1f, 0.1f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                            -0.1f, -0.1f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                            0.1f, -0.1f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                            0.1f, -0.1f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                            0.1f, 0.1f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
                            -0.1f, 0.1f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f
                    }};
        addToMesh(meshTypes::eSquare, vertices);
        vertices = {{
                            -0.1f, -0.05f, 0.0f, 0.0f, 1.0f, 0.0f, 0.25f,
                            -0.04f, -0.05f, 0.0f, 0.0f, 1.0f, 0.3f, 0.25f,
                            -0.06f, 0.0f, 0.0f, 0.0f, 1.0f, 0.2f, 0.5f,
                            -0.04f, -0.05f, 0.0f, 0.0f, 1.0f, 0.3f, 0.25f,
                            0.0f, -0.1f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f,
                            0.04f, -0.05f, 0.0f, 0.0f, 1.0f, 0.7f, 0.25f,
                            -0.06f, 0.0f, 0.0f, 0.0f, 1.0f, 0.2f, 0.5f,
                            -0.04f, -0.05f, 0.0f, 0.0f, 1.0f, 0.3f, 0.25f,
                            0.04f, -0.05f, 0.0f, 0.0f, 1.0f, 0.7f, 0.25f,
                            0.04f, -0.05f, 0.0f, 0.0f, 1.0f, 0.7f, 0.25f,
                            0.1f, -0.05f, 0.0f, 0.0f, 1.0f, 1.0f, 0.25f,
                            0.06f, 0.0f, 0.0f, 0.0f, 1.0f, 0.8f, 0.5f,
                            -0.06f, 0.0f, 0.0f, 0.0f, 1.0f, 0.2f, 0.5f,
                            0.04f, -0.05f, 0.0f, 0.0f, 1.0f, 0.7f, 0.25f,
                            0.06f, 0.0f, 0.0f, 0.0f, 1.0f, 0.8f, 0.5f,
                            0.06f, 0.0f, 0.0f, 0.0f, 1.0f, 0.8f, 0.5f,
                            0.08f, 0.1f, 0.0f, 0.0f, 1.0f, 0.9f, 1.0f,
                            0.0f, 0.02f, 0.0f, 0.0f, 1.0f, 0.5f, 0.6f,
                            -0.06f, 0.0f, 0.0f, 0.0f, 1.0f, 0.2f, 0.5f,
                            0.06f, 0.0f, 0.0f, 0.0f, 1.0f, 0.8f, 0.5f,
                            0.0f, 0.02f, 0.0f, 0.0f, 1.0f, 0.5f, 0.6f,
                            -0.06f, 0.0f, 0.0f, 0.0f, 1.0f, 0.2f, 0.5f,
                            0.0f, 0.02f, 0.0f, 0.0f, 1.0f, 0.5f, 0.6f,
                            -0.08f, 0.1f, 0.0f, 0.0f, 1.0f, 0.1f, 1.0f
                    }};
        addToMesh(meshTypes::eStar, vertices);

    }

    void vkMeshes::addToMesh(yic::meshTypes type, const std::vector<float> &vertexData) {
        for (float data: vertexData) {
            mMeshes.push_back(data);
        }
        int vertexCount = static_cast<int>(vertexData.size() / 7);

        mOffsets.insert(std::make_pair(type, mOffset));
        mSizes.insert(std::make_pair(type, vertexCount));

        mOffset += vertexCount;
    }

} // yic





//void vkMeshes::makeAssets() {
//    std::vector<float> vertices = { {
//                                            0.0f, -0.5f, 0.0f, 1.0f, 0.0f,
//                                            0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
//                                            -0.5f, 0.5f, 0.0f, 1.0f, 0.0f
//                                    } };
//    addToMesh(meshTypes::eTriangle, vertices);
//    vertices = { {
//                         -0.35f,  0.35f, 1.0f, 0.0f, 0.0f,
//                         -0.35f, -0.35f, 1.0f, 0.0f, 0.0f,
//                         0.35f, -0.35f, 1.0f, 0.0f, 0.0f,
//                         0.35f, -0.35f, 1.0f, 0.0f, 0.0f,
//                         0.35f,  0.35f, 1.0f, 0.0f, 0.0f,
//                         -0.35f,  0.35f, 1.0f, 0.0f, 0.0f
//                 } };
//    addToMesh(meshTypes::eSquare, vertices);
//    vertices = { {
//                         -0.5f, -0.25f, 0.0f, 0.0f, 1.0f,
//                         -0.2f, -0.25f, 0.0f, 0.0f, 1.0f,
//                         -0.3f,    0.0f, 0.0f, 0.0f, 1.0f,
//                         -0.2f, -0.25f, 0.0f, 0.0f, 1.0f,
//                         0.0f,  -0.5f, 0.0f, 0.0f, 1.0f,
//                         0.2f, -0.25f, 0.0f, 0.0f, 1.0f,
//                         -0.3f,    0.0f, 0.0f, 0.0f, 1.0f,
//                         -0.2f, -0.25f, 0.0f, 0.0f, 1.0f,
//                         0.2f, -0.25f, 0.0f, 0.0f, 1.0f,
//                         0.2f, -0.25f, 0.0f, 0.0f, 1.0f,
//                         0.5f, -0.25f, 0.0f, 0.0f, 1.0f,
//                         0.3f,    0.0f, 0.0f, 0.0f, 1.0f,
//                         -0.3f,    0.0f, 0.0f, 0.0f, 1.0f,
//                         0.2f, -0.25f, 0.0f, 0.0f, 1.0f,
//                         0.3f,    0.0f, 0.0f, 0.0f, 1.0f,
//                         0.3f,    0.0f, 0.0f, 0.0f, 1.0f,
//                         0.4f,   0.5f, 0.0f, 0.0f, 1.0f,
//                         0.f,   0.1f, 0.0f, 0.0f, 1.0f,
//                         -0.3f,    0.0f, 0.0f, 0.0f, 1.0f,
//                         0.3f,    0.0f, 0.0f, 0.0f, 1.0f,
//                         0.f,   0.1f, 0.0f, 0.0f, 1.0f,
//                         -0.3f,    0.0f, 0.0f, 0.0f, 1.0f,
//                         0.f,   0.1f, 0.0f, 0.0f, 1.0f,
//                         -0.4f,   0.5f, 0.0f, 0.0f, 1.0f
//                 } };
//    addToMesh(meshTypes::eStar, vertices);
//
//}