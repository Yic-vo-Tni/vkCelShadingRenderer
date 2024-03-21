//
// Created by lenovo on 2/7/2024.
//

#include "vkTasker.h"

#include <utility>

namespace tasker {


    loadTexture::loadTexture(genericTexManagerSptr &tex, std::string path, std::string id) : mTexSptr(tex), mPath(std::move(path)) {
        mId = std::move(id);
    }

    vkTasker &loadTexture::execute() {
        mTexSptr = std::make_shared<yic::vkImage>(mPath);
        mTaskStatus = taskStatus::eComplete;
        vkEventFactory::get()->getLoadResEvent(mPath).singlePublish(vkTaskGroupType::eResourceLoadGroup);

        return *this;
    }



    loadSkyBox::loadSkyBox(genericSkyboxManagerSptr &skybox, std::vector<std::string> paths, std::string id) : mSkybox(skybox), mPaths(std::move(paths)){
        mId = std::move(id);
    }

    vkTasker &loadSkyBox::execute() {
        mSkybox = std::make_shared<yic::vkCubeMap>(mPaths);
        mTaskStatus = taskStatus::eComplete;
        vkEventFactory::get()->getLoadResEvent(mId).singlePublish(vkTaskGroupType::eResourceLoadGroup);

        return *this;
    }







} // task