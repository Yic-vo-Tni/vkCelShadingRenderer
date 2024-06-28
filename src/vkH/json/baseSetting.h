//
// Created by lenovo on 5/8/2024.
//

#ifndef VKMMD_BASESETTING_H
#define VKMMD_BASESETTING_H

#include "nlohmann/json.hpp"
#include "log/Log.h"

namespace jsonFields{
    constexpr const char* width = "width";
    constexpr const char* height = "height";
    constexpr const char* window_size_id = "window_size_id";
    constexpr const char* drawMode = "drawMode";
}
//namespace options{
//    constexpr const char* items[] = {"1200 * 800", "1800 * 1200", "2400 * 1600"};
//    inline static int itemCurrentIdx = 1;
//}



namespace yic {

    using json = nlohmann::json;

    class baseSetting {
        struct options{
            options(const std::vector<const char*>& items, int idx) : items(items), itemCurrentIdx(idx){};
            std::vector<const char*> items;

            int itemCurrentIdx;
        };
    public:
        vkGet auto get = [](){ return Singleton<baseSetting>::get();};

        static void load(const std::string& fileName);
        static void save(const std::string& fileName);
        static void firstDefaultValue(const std::string& fileName);
        static void reStartApp();

        static void Gui();

        [[nodiscard]] inline static const auto& GetWidth()  { return get()->mWidth;}
        [[nodiscard]] inline static const auto& GetHeight() { return get()->mHeight;}
        [[nodiscard]] inline static const auto& GetPrimitiveTopology() { return get()->mPrimitiveTopology;}


    private:
        int mWidth = 1800, mHeight = 1200;
        vk::PrimitiveTopology mPrimitiveTopology{vk::PrimitiveTopology::eTriangleList};

        options mOptions_window{{"1200 * 800", "1800 * 1200", "2400 * 1600"}, 1};
        options mOptions_PrimitiveTopology{{"TriangleList", "PointList", "LineList"}, 0};
    };

} // yic

#endif //VKMMD_BASESETTING_H
