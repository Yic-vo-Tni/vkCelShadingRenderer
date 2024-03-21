//
// Created by lenovo on 2/9/2024.
//

#include "vkThread.h"

namespace tasker {

    vkThread::vkThread(vkTaskGroupType type) : mType(type){

    }

    vkThread &vkThread::operator()() {
        auto& wQueueMutex = wQueueFactory::get()->getWorkQ(mType)->getMutex();
        wQueueMutex.lock();
        wQueueMutex.unlock();

        while (!mDone){
            if (wQueueMutex.try_lock()){
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                continue;
            }
        }

        return *this;
    }

} // tasker