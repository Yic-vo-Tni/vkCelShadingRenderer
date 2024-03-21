//
// Created by lenovo on 2/9/2024.
//

#ifndef VKMMD_VKTHREAD_H
#define VKMMD_VKTHREAD_H

#include "vkWorkQueue.h"

namespace tasker {

    class vkThread {
    public:
        explicit vkThread(vkTaskGroupType type);

        vkThread& operator()();

    private:
        vkTaskGroupType mType;
        bool mDone;
    };

} // tasker

#endif //VKMMD_VKTHREAD_H
