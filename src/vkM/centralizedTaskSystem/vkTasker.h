//
// Created by lenovo on 2/7/2024.
//

#ifndef VKMMD_VKTASKER_H
#define VKMMD_VKTASKER_H

#include <utility>

#include "vkEvent.h"



namespace tasker {

    enum class taskStatus{
        ePending, eIn_Progress, eComplete,
        eInitialized, eWaitingForDependencies,
        eQueued,
        eFailed, eCancelled, ePaused,
    };

    class vkTasker {
    public:
        virtual vkTasker& execute() = 0;

    protected:
        std::string mId;
        taskStatus mTaskStatus = taskStatus::ePending;
    };

    ///--------------------------------------------------------------------///

    class loadTexture : public vkTasker{
    public:
        loadTexture(genericTexManagerSptr& tex, std::string path, std::string id);
        vkTasker & execute() override;

    private:
        genericTexManagerSptr& mTexSptr;
        std::string mPath;
    };


    class loadSkyBox : public vkTasker{
    public:
        loadSkyBox(genericSkyboxManagerSptr& skybox, std::vector<std::string> paths, std::string id);
        vkTasker & execute() override;

    private:
        genericSkyboxManagerSptr& mSkybox;
        std::vector<std::string> mPaths;
    };





    class specialTask : public vkTasker{
    public:
        explicit specialTask(std::string  id) : mId(std::move(id)) {}

        vkTasker & execute() override{
            vkEventFactory::get()->getLoadResEvent(mId).setPublishComplete(vkTaskGroupType::eResourceLoadGroup);
            return *this;
        }

    private:
        std::string mId;
    };


} // task


#endif //VKMMD_VKTASKER_H
