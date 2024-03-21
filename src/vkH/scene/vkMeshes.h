//
// Created by lenovo on 12/25/2023.
//

#ifndef VULKAN_VKMESHES_H
#define VULKAN_VKMESHES_H

namespace yic {

    enum meshTypes{
        eTriangle,
        eSquare,
        eStar
    };

    class vkMeshes {
    public:
        vkMeshes();
        ~vkMeshes();

        void makeAssets();

        std::unordered_map<meshTypes, int> mOffsets;
        std::unordered_map<meshTypes, int> mSizes;

        [[nodiscard]] auto& getMeshes() const { return mMeshes;}
    private:
        void addToMesh(meshTypes type, const std::vector<float>& vertexData);
        int mOffset{};
        std::vector<float> mMeshes;
    };

} // yic

#endif //VULKAN_VKMESHES_H
