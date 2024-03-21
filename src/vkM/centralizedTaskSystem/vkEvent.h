//
// Created by lenovo on 2/7/2024.
//

#ifndef VKMMD_VKEVENT_H
#define VKMMD_VKEVENT_H

#include "niLou/vkImage.h"

enum class vkTaskGroupType{
    eResourceLoadGroup,
    //ePreRenderGroup,
    eRenderGroup,
//    ePostProcessGroup,
//    eCleanupGroup,
    eCount,
};

namespace tasker {

    using fnCallback = std::function<void()>;

    class vkEvent {
    public:
        vkEvent& subscribe(vkTaskGroupType type, const fnCallback& callback){
            mListeners[type] = callback;
            if (mPublishComplete[type]){
                callback();
            }

            return *this;
        }


        vkEvent& singlePublish(vkTaskGroupType type){
            checkAndTrigger(type);

            return *this;
        }

        vkEvent& setPublishComplete(vkTaskGroupType type){
            mPublishComplete[type] = true;
            checkAndTrigger(type);

            return *this;
        }

    private:
        vkEvent& checkAndTrigger(vkTaskGroupType type){
            if (mPublishComplete[type] && mListeners.find(type) != mListeners.end()){
                mListeners[type]();
                mFinish = true;
            }

            return *this;
        }

        std::unordered_map<vkTaskGroupType, fnCallback> mListeners;
        std::unordered_map<vkTaskGroupType, bool> mPublishComplete;
    public:
        bool mFinish = false;
    };

    class vkEventFactory{
    public:
        vkGet auto get = [](){ return yic::Singleton<vkEventFactory>::get(); };

        vkEvent& getLoadResEvent(const std::string& id){
            auto item = mLoadResource.find(id);
            if (item != mLoadResource.end()){
                return item->second;
            } else{
                auto& event = mLoadResource[id];
                return event;
            }
        }

        vkEventFactory& subscribe(const std::string& id, vkTaskGroupType type, const fnCallback& callback){
            auto& event = getLoadResEvent(id);
            event.subscribe(type, callback);

            return *this;
        }

        vkEventFactory& clear(){
            for(auto it = mLoadResource.begin(); it != mLoadResource.end(); ){
                if (it->second.mFinish){
                    it = mLoadResource.erase(it);
                } else{
                    ++it;
                }
            }

            return *this;
        }

    private:
        std::unordered_map<std::string, vkEvent> mLoadResource;
    };

} // tasker

#endif //VKMMD_VKEVENT_H
