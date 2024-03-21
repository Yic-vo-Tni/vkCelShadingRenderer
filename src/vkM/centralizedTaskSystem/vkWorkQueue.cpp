//
// Created by lenovo on 2/9/2024.
//

#include "vkWorkQueue.h"

namespace tasker {

    void workQueue::add(const std::shared_ptr<vkTasker>& tasker) {
        mQueue.push(tasker);
    }

    std::shared_ptr<vkTasker> workQueue::getNext() {
        if (mQueue.empty())
            return nullptr;

        auto task = mQueue.front();
        mQueue.pop();
        return task;
    }

    void workQueue::clear() {
        while (!mQueue.empty()){
            mQueue.pop();
        }
    }

    ///--------------------------------------------  Factory

    wQueueFactory &wQueueFactory::addResSpecialTask2End(const std::string& id) {
        mQueues[vkTaskGroupType::eResourceLoadGroup]->add(std::make_shared<specialTask>(id));

        return *this;
    }

    wQueueFactory &wQueueFactory::addSpecialTask2Start(const std::string &id) {
        std::promise<void> tempPromise;
        mPromise[id] = std::move(tempPromise);
        mFutures[id] = mPromise[id].get_future();
        return *this;
    }

    wQueueFactory &wQueueFactory::addSpecialTask2CompleteSetPromise(const std::string &id) {
        mPromise[id].set_value();
        return *this;
    }

    wQueueFactory &wQueueFactory::waitForTaskCompletion(const std::string &id) {
        if (mFutures.find(id) != mFutures.end()){
            mFutures[id].wait();
        }
        return *this;
    }

    /// thread

    wQueueFactory &wQueueFactory::addSpecialTask2ThreadStart(vkTaskGroupType type) {
        auto lockPrimary = mPrimaryMutexes[type].try_lock();
        auto lockBackup = false;

        if (!lockPrimary){
            lockBackup = mBackupMutexes[type].try_lock();
        }

        if(lockPrimary || !lockBackup){
            mQueues[type] = mPrimaryQueues[type];
        } else {
            mQueues[type] = mBackupQueues[type];
        }

//        (mActiveQueueFlags[type] ? mPrimaryMutexes[type] : mBackupMutexes[type]).lock();
//        mQueues[type] = mActiveQueueFlags[type] ? mPrimaryQueues[type] : mBackupQueues[type];
//        mConditionV[type].notify_one();
        return *this;
    }

    /// add res task

    wQueueFactory &wQueueFactory::addTask2TextureQueue(genericTexManagerSptr &texManagerSptr, const std::string &path, const std::string &id) {
        mQueues[vkTaskGroupType::eResourceLoadGroup]->add(std::make_shared<loadTexture>(texManagerSptr, path, id));

        return *this;
    }
    wQueueFactory &wQueueFactory::addTask2TextureQueue(genericSkyboxManagerSptr &skyboxManagerSptr, const std::vector<std::string> &paths, const std::string& id) {
        mQueues[vkTaskGroupType::eResourceLoadGroup]->add(std::make_shared<loadSkyBox>(skyboxManagerSptr, paths, id));

        return *this;
    }

    /// subscribe

    wQueueFactory &wQueueFactory::subscribeRes(const std::string &id, const tasker::fnCallback &callback) {
        vkEventFactory::get()->subscribe(id, vkTaskGroupType::eResourceLoadGroup, callback);

        return *this;
    }


    // execute

    wQueueFactory &wQueueFactory::executeSingle(vkTaskGroupType type) {
        while (auto texWorkQ = mQueues[type]->getNext())
            texWorkQ->execute();

        mQueues[type]->clear();
        vkEventFactory::get()->clear();

        return *this;
    }

} // tasker