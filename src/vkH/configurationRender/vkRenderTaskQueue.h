#include <utility>

//
// Created by lenovo on 3/29/2024.
//

#ifndef VKMMD_VKRENDERTASKQUEUE_H
#define VKMMD_VKRENDERTASKQUEUE_H

namespace yic {

    class vkRenderTaskQueue : public nonCopyable{
        using renderTask = std::function<void(const vk::CommandBuffer&)>;
        using objKey = const void *;
        friend Singleton<vkRenderTaskQueue>;
    public:
        vkGet auto get = [](){return Singleton<vkRenderTaskQueue>::get();};

        template<typename T>
        vkRenderTaskQueue& addTask(const T& ptr, const renderTask& task){
            auto key = static_cast<const void*>(ptr.get());
            mBackupTasks[key] = task;
            printTaskInfo();
            return *this;
        }
        template<typename T>
        vkRenderTaskQueue& removeTask(const T& ptr){
            auto key = static_cast<const void*>(ptr.get());
            if (key != nullptr) { mBackupTasks.erase(key); }
            printTaskInfo();
            return *this;
        }
        vkRenderTaskQueue& update(){
            mUpdate = true;
            return *this;
        }

        void execute(const vk::CommandBuffer& cmd){
            if (mUpdate) {
                mRenderTasks.clear();
                mRenderTasks = mBackupTasks;
                mUpdate = false;
            }
            for (auto &task: mRenderTasks) {
                task.second(cmd);
            }
        }

        void printTaskInfo(){
            std::cout << mBackupTasks.size() << std::endl;
            for(const auto& pair : mBackupTasks){
                std::cout << " " << pair.first << std::endl;
            }
        }

    private:
        std::unordered_map<objKey, renderTask> mRenderTasks, mBackupTasks;
        bool mUpdate{false};
    };

} // yic

#endif //VKMMD_VKRENDERTASKQUEUE_H
