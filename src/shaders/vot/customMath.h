//
// Created by lenovo on 3/26/2024.
//

#ifndef VKMMD_CUSTOMMATH_H
#define VKMMD_CUSTOMMATH_H

#include "glm/glm.hpp"
#include "vkIdGenerator.h"


namespace vot {

    class mathBase{
    public:
        mathBase(){ mID = vkIdGenerator::get()->generatorUniqueId();}

        std::string getID() const { return mID; }
        std::string getCode() const {return mShaderCode;}

    protected:
        std::string mShaderCode;
        std::string mID;
    };

    class cVec2 : public mathBase{
    public:
        cVec2() = default;
        cVec2(float x, float y) {
            // 构建初始GLSL表示
            mShaderCode += "vec2(" + std::to_string(x) + ", " + std::to_string(y) + ")";
        }

        // 不需要重载赋值运算符，但可以提供方法来构建复杂表达式
        void assign(const cVec2& rhs) {
            // 更新GLSL代码以反映赋值
            mShaderCode = rhs.getCode();
        }

        // 为减法提供特定方法
        void subtract(const cVec2& rhs) {
            // 构建减法GLSL代码
            mShaderCode += " - " + rhs.getCode();
        }

        // 提供方法获取x和y的GLSL代码表示
        std::string x() const { return mShaderCode + ".x"; }
        std::string y() const { return mShaderCode + ".y"; }

        [[nodiscard]] inline auto& getVal()  { return mVal;}
        [[nodiscard]] inline auto& getVal() const { return mVal;}
    private:
        float mX, mY;
        glm::vec2 mVal;
    };

    class cVec3 : public mathBase{
    public:


        [[nodiscard]] inline auto& getVal()  { return mVal;}
        [[nodiscard]] inline auto& getVal() const { return mVal;}
    private:
        glm::vec3 mVal;
    };


} // vot

#endif //VKMMD_CUSTOMMATH_H
