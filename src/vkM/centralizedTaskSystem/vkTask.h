//
// Created by lenovo on 2/11/2024.
//

#ifndef VKMMD_VKTASK_H
#define VKMMD_VKTASK_H


namespace tasker {

    struct toResource {
        static auto &addTexture(genericTexManagerSptr &texManagerSptr, const std::string &path, const std::string &id) {
            return tasker::wQueueFactory::get()->addTask2TextureQueue(texManagerSptr, path, id);
        }

        static auto &addTexture(genericSkyboxManagerSptr &skyboxManagerSptr, const std::vector<std::string> &paths,
                                const std::string &id) {
            return tasker::wQueueFactory::get()->addTask2TextureQueue(skyboxManagerSptr, paths, id);
        }

        struct specialTask{
            static auto& markGroupEnd(const std::string & id){
                return tasker::wQueueFactory::get()->addResSpecialTask2End(id);
            }

            struct promise{

                static auto& pre_task(const std::string &id){
                    tasker::wQueueFactory::get()->addSpecialTask2Start(id);
                    tasker::wQueueFactory::get()->addSpecialTask2CompleteSetPromise(id);
                    return tasker::wQueueFactory::get;
                }

                static auto& post_task(const std::string & id){
                    tasker::wQueueFactory::get()->waitForTaskCompletion(id);
                    return tasker::wQueueFactory::get;
                }


            };
        };




        struct eventListeners{
            static auto& subscribe(const std::string& id, const tasker::fnCallback& callback){
                return tasker::wQueueFactory::get()->subscribeRes(id, callback);
            }
        };
    };

    struct toRender{

        struct specialTask{

        };
    };

}

#endif //VKMMD_VKTASK_H
