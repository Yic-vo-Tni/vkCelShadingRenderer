//
// Created by lenovo on 12/30/2023.
//

#ifndef VULKAN_VKSCENE_H
#define VULKAN_VKSCENE_H

namespace yic {

    class vkScene : public nonCopyable{
    public:
        vkScene();

        std::vector<glm::vec3> trianglePositions;
        std::vector<glm::vec3> squarePositions;
        std::vector<glm::vec3> starPositions;
        std::vector<glm::mat4> mMatrix;
    };

} // yic

#endif //VULKAN_VKSCENE_H
