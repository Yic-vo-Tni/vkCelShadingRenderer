//
// Created by lenovo on 3/29/2024.
//

#ifndef VKMMD_VKRESMODEL_H
#define VKMMD_VKRESMODEL_H

#include "vkResourceBase.h"
#include "pmx/pmx_header.h"
#include "pmx/pmx_context.h"

namespace rs {

    class vkPmxResModel : public vkResourceBase{
    public:
        [[nodiscard]] inline auto& getContext() const { return mPmxContext;}

    private:
        std::unique_ptr<vkPmx::pmx_context> mPmxContext{};
    };

    class vkPmxLoadModel {
        using loadModelTask = std::function<void()>;
    public:


    private:
        std::vector<loadModelTask> mLoadModelTasks;
    };

} // rs

#endif //VKMMD_VKRESMODEL_H
