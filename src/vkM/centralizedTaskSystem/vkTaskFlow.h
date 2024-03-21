//
// Created by lenovo on 2/10/2024.
//

#ifndef VKMMD_VKTASKFLOW_H
#define VKMMD_VKTASKFLOW_H

#include "vkEvent.h"

#include "taskflow/taskflow.hpp"

enum taskflowType{
    eInit,
    eGraphics, eTransfer,
    ePreRender, eRender,
    eCount,
};

namespace tasker {

    class vkTaskFlow{
    public:
        vkTaskFlow& addTask(const std::string& id, const fnCallback &callback);
        vkTaskFlow& addTasks(const std::vector<std::pair<std::string, fnCallback>>& tasks);

        vkTaskFlow& addTask_precede(const fnCallback &callback);
        vkTaskFlow& addTask(const fnCallback &callback);

        vkTaskFlow& addSpecialTask_notificationUpdate();
        vkTaskFlow& addDependency(const std::string & fromId, const std::string & toId);
        vkTaskFlow& composedTask(tf::Taskflow& taskflow);
        vkTaskFlow& executeTaskflow();

        [[nodiscard]] inline auto& getTaskflow()  { return mBackupTaskflow;}
    private:
        tf::Executor mExecutor;
        tf::Taskflow mPrimaryTaskflow, mBackupTaskflow;
        std::unordered_map<taskflowType, tf::Taskflow> mTaskFlows;
        std::unordered_map<std::string, tf::Task> mTasks;

        std::optional<tf::Task> mLastTask;
        std::atomic<bool> mUpdate = true;
    };


    class vkTaskflowFactory{
    public:
        vkGet auto get = []() { return yic::Singleton<vkTaskflowFactory>::get(); };

        [[nodiscard]] inline auto& getTaskFlow(const taskflowType& type)  { return mTaskflows[type];}

        vkTaskflowFactory& notificationUpdate();
        vkTaskflowFactory& executeTaskflow();
    private:
        bool mUpdate = true;
        tf::Executor mExecutor;
        tf::Taskflow mPrimaryTaskFlow, mBackupTaskFlow;
        std::unordered_map<taskflowType, vkTaskFlow> mTaskflows;
    };

} // tasker

namespace taskerBuilder{
    using fn_callback = std::function<void()>;
    inline auto getTaskflowFactory = tasker::vkTaskflowFactory::get();
    inline auto& getInitTaskFlow = tasker::vkTaskflowFactory::get()->getTaskFlow(eInit);
    inline auto& getGraphicsTaskFlow = tasker::vkTaskflowFactory::get()->getTaskFlow(eGraphics);
    inline auto& getTransferTaskFlow = tasker::vkTaskflowFactory::get()->getTaskFlow(eTransfer);
    inline auto& getPreRenderTaskFlow = tasker::vkTaskflowFactory::get()->getTaskFlow(ePreRender);

    struct toInit{
        static bool addGeneralSequentialTask(const fn_callback &callback);
    };

    struct toTransfer{
        static bool addTexture(genericTexManagerSptr& tex, const std::string& path);
        static bool addTexture(genericSkyboxManagerSptr & tex, const std::vector<std::string>& path);
    };

    struct toGraphics{
        static bool transformTexLayout(const fn_callback &callback);
    };

    struct toPreRender{
        static bool addGeneralSequentialTask(const fn_callback &callback);
    };

    struct executor{
        static bool executeTaskflow();
    };

}


#endif //VKMMD_VKTASKFLOW_H
