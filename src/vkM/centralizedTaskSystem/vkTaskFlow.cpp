//
// Created by lenovo on 2/10/2024.
//

#include "vkTaskFlow.h"

namespace tasker {

    vkTaskFlow &vkTaskFlow::addTask(const tasker::fnCallback &callback) {
        mBackupTaskflow.emplace(callback);

        return *this;
    }

    vkTaskFlow &vkTaskFlow::addTask(const std::string& id, const tasker::fnCallback &callback) {
        auto task = mBackupTaskflow.emplace(callback);
        mTasks[id] = task;

        return *this;
    }

    vkTaskFlow &vkTaskFlow::addTask_precede(const tasker::fnCallback &callback) {
        auto task = mBackupTaskflow.emplace(callback);

        if (mLastTask.has_value()){
            mLastTask->precede(task);
        }

        mLastTask = std::make_optional(task);

        return *this;
    }

    vkTaskFlow &vkTaskFlow::addTasks(const std::vector<std::pair<std::string, fnCallback>> &tasks) {
        for (const auto& [id, callback] : tasks) {
            addTask(id, callback);
        }
        return *this;
    }

    vkTaskFlow &vkTaskFlow::composedTask(tf::Taskflow &taskflow) {
        mBackupTaskflow.composed_of(taskflow);

        return *this;
    }

    vkTaskFlow &vkTaskFlow::addDependency(const std::string &fromId, const std::string &toId) {
        if (mTasks.find(fromId) != mTasks.end() && mTasks.find(toId) != mTasks.end()) {
            mTasks[fromId].precede(mTasks[toId]);
        }

        return *this;
    }

    vkTaskFlow &vkTaskFlow::addSpecialTask_notificationUpdate() {
        mUpdate = true;
        mLastTask.reset();

        return *this;
    }

    vkTaskFlow &vkTaskFlow::executeTaskflow() {
        if (mUpdate){
            mExecutor.run(mBackupTaskflow).wait();
            std::swap(mPrimaryTaskflow, mBackupTaskflow);
            mBackupTaskflow.clear();
            mUpdate = false;
        } else{
            mExecutor.run(mPrimaryTaskflow).wait();
        }

        return *this;
    }

    ////////////////////////////////////////////////

    vkTaskflowFactory &vkTaskflowFactory::notificationUpdate() {
        auto init = mBackupTaskFlow.composed_of(mTaskflows[eInit].getTaskflow());
        auto graphics = mBackupTaskFlow.composed_of(mTaskflows[eGraphics].getTaskflow());
        auto transfer = mBackupTaskFlow.composed_of(mTaskflows[eTransfer].getTaskflow());
        auto preRender = mBackupTaskFlow.composed_of(mTaskflows[ePreRender].getTaskflow());
        auto Render = mBackupTaskFlow.composed_of(mTaskflows[eRender].getTaskflow());

        init.precede(graphics);
        transfer.precede(graphics);

        graphics.precede(preRender);
        preRender.precede(Render);

        mUpdate = true;

        return *this;
    }


    vkTaskflowFactory &vkTaskflowFactory::executeTaskflow() {
        if (mUpdate){
            mExecutor.run(mBackupTaskFlow).wait();
            std::swap(mPrimaryTaskFlow, mBackupTaskFlow);
            mBackupTaskFlow.clear();
            mUpdate = false;
        } else {
            mExecutor.run(mPrimaryTaskFlow).wait();
        }


        return *this;
    }




} // tasker


namespace taskerBuilder{

    bool toInit::addGeneralSequentialTask(const taskerBuilder::fn_callback &callback) {
        getInitTaskFlow.addTask(callback);

        return true;
    }

    bool toTransfer::addTexture(genericTexManagerSptr& tex, const std::string& path) {
        getTransferTaskFlow.addTask_precede([&, path](){tex = std::make_shared<yic::vkImage>(path);});

        return true;
    }

    bool toTransfer::addTexture(genericSkyboxManagerSptr &tex, const std::vector<std::string> &path) {
        getTransferTaskFlow.addTask_precede([&, path](){ tex = std::make_shared<yic::vkCubeMap>(path); });

        return true;
    }

    /// -------------------- --------------------///

    bool toGraphics::transformTexLayout(const fn_callback &callback) {
        getGraphicsTaskFlow.addTask_precede(callback);

        return true;
    }


    /// -------------------- --------------------///

    bool toPreRender::addGeneralSequentialTask(const taskerBuilder::fn_callback &callback) {
        getPreRenderTaskFlow.addTask_precede(callback);

        return true;
    }

    /// -------------------- --------------------///


    bool executor::executeTaskflow() {
        tasker::vkTaskflowFactory::get()->notificationUpdate();
        tasker::vkTaskflowFactory::get()->executeTaskflow();

        return true;
    }











}





























