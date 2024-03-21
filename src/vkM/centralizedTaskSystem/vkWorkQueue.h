//
// Created by lenovo on 2/9/2024.
//

#ifndef VKMMD_VKWORKQUEUE_H
#define VKMMD_VKWORKQUEUE_H

#include "vkTasker.h"

namespace tasker {

    class workQueue{
    public:
        void add(const std::shared_ptr<vkTasker>& tasker);
        std::shared_ptr<vkTasker> getNext();
        void clear();

        [[nodiscard]] inline auto& getMutex() const { return mQueueMutex;}
        [[nodiscard]] inline auto& getMutex()  { return mQueueMutex;}
    private:
        size_t length = 0;
        vkTasker* first = nullptr, * last = nullptr;
    private:
        std::queue<std::shared_ptr<vkTasker>> mQueue;
        std::mutex mQueueMutex;
    };

    class wQueueFactory{
        wQueueFactory(){
            if (mQueues.empty()){
                for(int i = 0; static_cast<vkTaskGroupType>(i) < vkTaskGroupType::eCount; i++){
                    mQueues[static_cast<vkTaskGroupType>(i)] = std::make_shared<workQueue>();
                    mPrimaryQueues[static_cast<vkTaskGroupType>(i)] = std::make_shared<workQueue>();
                    mBackupQueues[static_cast<vkTaskGroupType>(i)] = std::make_shared<workQueue>();
                }
            }
        };
        friend yic::Singleton<wQueueFactory>;

    public:
        vkGet auto get = [](){ return yic::Singleton<wQueueFactory>::get(); };

        template<typename... Args>
        wQueueFactory& execute(Args... args){
            (executeSingle(args), ...);
            return *this;
        }

        wQueueFactory& addTask2TextureQueue(genericSkyboxManagerSptr& skyboxManagerSptr, const std::vector<std::string>& paths, const std::string& id);
        wQueueFactory& addTask2TextureQueue(genericTexManagerSptr& texManagerSptr, const std::string& path, const std::string& id);

        wQueueFactory& subscribeRes(const std::string& id, const fnCallback &callback);

        // special task
        wQueueFactory& addResSpecialTask2End(const std::string& id);

        wQueueFactory& addSpecialTask2ThreadStart(vkTaskGroupType type);
        wQueueFactory& addSpecialTask2Start(const std::string& id);
        wQueueFactory& addSpecialTask2CompleteSetPromise(const std::string& id);
        wQueueFactory& waitForTaskCompletion(const std::string &id);
        wQueueFactory& notificationThreadEnd(vkTaskGroupType type);

    private: ///
        wQueueFactory& executeSingle(vkTaskGroupType type);

    public:
        std::shared_ptr<workQueue> getWorkQ(vkTaskGroupType type){
            if (auto item = mQueues.find(type); item != mQueues.end()){
                return item->second;
            } else{
                vkDebug{ vkError("failed to find the work queue"); }
                return nullptr;
            }
        }
    private:
        std::unordered_map<vkTaskGroupType, std::shared_ptr<workQueue>> mQueues;
        std::unordered_map<vkTaskGroupType, std::shared_ptr<workQueue>> mPrimaryQueues;
        std::unordered_map<vkTaskGroupType, std::shared_ptr<workQueue>> mBackupQueues;
        std::unordered_map<vkTaskGroupType, std::mutex> mPrimaryMutexes;
        std::unordered_map<vkTaskGroupType, std::mutex> mBackupMutexes;
        std::unordered_map<vkTaskGroupType, std::condition_variable> mConditionV;
        std::unordered_map<vkTaskGroupType, bool> mActiveQueueFlags;
        std::unordered_map<std::string, std::promise<void>> mPromise;
        std::unordered_map<std::string, std::future<void>> mFutures;
    };





} // tasker



#endif //VKMMD_VKWORKQUEUE_H
