//
// Created by lenovo on 11/25/2023.
//

#ifndef VULKAN_VK_WINDOW_H
#define VULKAN_VK_WINDOW_H

#include "log/Log.h"
#include "niLou/vkCamera.h"
//#include "editor/modelTransformManager.h"
//#include "json/vkProject.h"

namespace yic {

    struct imgRect{
        ImVec2 min{};
        ImVec2 max{};
    };

    class Window : public nonCopyable{
        explicit Window(int w = {}, int h = {}, std::string  name = {});
        ~Window();

        friend Singleton<Window>;
    public:
        vkGet auto get = [](int w = {}, int h = {}, const std::string& name = {}){ return Singleton<Window>::get(w, h, name); };

        [[nodiscard]] auto& window() const { return window_;}

        void processInput();
        [[nodiscard]] glm::vec3 detectionMouseRay();
        bool rayIntersectsAABB(glm::vec3 min, glm::vec3 max, float &nearest);
        bool rayIntersectsXYZAABB(glm::vec3 min, glm::vec3 max);

        void addToImgRects(uint32_t id, const ImVec2& min, const ImVec2& max){
            imgRect imgRect{min, max};
            mImageRects[id] = imgRect;
        };
        void checkImgRect();
        void clearImgPath() {mImgPath.first = -1;}

        [[nodiscard]] auto& width() const { return width_;}
        [[nodiscard]] auto& height() const { return height_;}
        [[nodiscard]] inline auto& getImgPath() const { return mImgPath;}
        [[nodiscard]] inline auto& getUpdateImgRect() const { return updateImgRect;}

        [[nodiscard]] static inline const auto& MoveX()  { return get()->mMoveX;}
        [[nodiscard]] static inline const auto& MoveY()  { return get()->mMoveY;}
        [[nodiscard]] static inline const auto& MoveZ()  { return get()->mMoveZ;}
    private:

        int width_, height_;
        std::string name_;
        GLFWwindow* window_{};

        void initWindow();
        void setGLFWWindowIcon(GLFWwindow* window);

        double rectX, rectY;
        bool updateImgRect{false};
        std::unordered_map<uint32_t, imgRect> mImageRects{};
        std::pair<uint32_t, std::string> mImgPath{-1, ""};

    public:
        bool firstClick = true;
        double xLast{}, yLast{};
        double mLastX{}, mLastY;
        float deltaTime{}, lastFrame{};
        bool intersect{false}, mMoveXYZ{false}, isFirstMoveXYZ{false};

        //double mLastMouseMoveTime{}, mMouseMoveTimeOut{0.2};

        glm::vec3 mMoveX{0.f}, mMoveY{0.f}, mMoveZ{0.f};

        void calculateTime(){
            auto currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
        };
        void glfwCallback();
        void updateXYZMoveDistance();
    private:
        static void mouseCallback(GLFWwindow* window, double xPos, double yPos);
        static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
        static void defaultMouseCallback(GLFWwindow* window, double xPos, double yPos);
        static void defaultScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

        static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
        static void keyCallback(GLFWwindow* window, int key, int scancode,int action, int mode);

        static void dropCallback(GLFWwindow* window, int count, const char** paths);
        static void setCharCallback(GLFWwindow* window, unsigned int c);
    };

} // yic

#endif //VULKAN_VK_WINDOW_H
