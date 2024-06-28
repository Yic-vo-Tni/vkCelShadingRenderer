//
// Created by lenovo on 3/30/2024.
//

#ifndef VKMMD_VKIMGUITASKQUEUE_H
#define VKMMD_VKIMGUITASKQUEUE_H

namespace yic {

    class vkImguiTaskQueue {
    protected:
        using imguiTask = std::function<void()>;
    public:
        int addTask(const imguiTask & task){
            int id = mNextId++;
            mImguiTasks[id] = task;
            return id;
        }

        void addTaskToCollapsingHeader(const std::string& name, const imguiTask& task){
            for(auto& header : mCollapsingHeaderTasks){
                if (header.first == name){
                    header.second.emplace_back(task);
                    return;
                }
            }

            mCollapsingHeaderTasks.emplace_back(name, std::vector<imguiTask>{task});
        }

        void renderCollapsingHeader(){
            for(auto& entry : mCollapsingHeaderTasks){
                if(ImGui::CollapsingHeader(entry.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen)){
                    for(const auto& task : entry.second){
                        task();
                    }
                }
            }
        }

        void execute(){
            for(auto& task : mImguiTasks){
                task.second();
            }
        }

        bool findTask(int taskId){
            return mImguiTasks.find(taskId) != mImguiTasks.end();
        }

        void clear(int taskId){
            mImguiTasks.erase(taskId);
        }

        void clearCollapsingHeader(){
            mCollapsingHeaderTasks.clear();
        }

    private:
        int mNextId{};
        std::map<int, imguiTask> mImguiTasks;
        std::vector<std::string> mStructure;
        std::vector<std::pair<std::string, std::vector<imguiTask>>> mCollapsingHeaderTasks;
    };


    enum imguiRenderWindowType{
        eView, eToolbar, editor, eObjectManager, eShadowMap, eTest, eStructure
    };
    class vkImguiManager : public vkImguiTaskQueue{
    public:
        vkGet auto get = [](){ return Singleton<vkImguiManager>::get();};

        int addRenderToView(const imguiTask& task){ return mTasks[eView].addTask(task); }
        int addRenderToToolbar(const imguiTask& task){ return mTasks[eToolbar].addTask(task); }
        int addRenderToShadowMap(const imguiTask& task){ return mTasks[eShadowMap].addTask(task); }
        int addRenderToTest(const imguiTask& task) { return mTasks[eTest].addTask(task);}
        void addCollapsingHeaderTasksToObjManager(const std::string& name, const imguiTask& task){ mTasks[eObjectManager].addTaskToCollapsingHeader(name, task);}

        [[nodiscard]] inline auto& getImguiTask(const imguiRenderWindowType& type) { return mTasks[type];}
    private:
        std::unordered_map<imguiRenderWindowType, vkImguiTaskQueue> mTasks;
    };

    enum ImguiRenderModel{
        eRenderModel, eEditorModel
    };
    enum EditorModel{
        eAllAABB, eHighLightAABB
    };
    class vkImguiRenderModel : public vkImguiTaskQueue{
    public:
        vkGet auto get = [](){ return Singleton<vkImguiRenderModel>::get();};

        [[nodiscard]] inline auto& getCurrentModelTask() { return mCurrentModel;}
        void setRenderCurrentModel(const ImguiRenderModel& currentModel) { mCurrentModel = currentModel;}

        [[nodiscard]] inline auto& getCurrentEditorModel() const { return mCurrentEditorModel;}
        void setEditorCurrentModel(const EditorModel& currentModel) {mCurrentEditorModel = currentModel;}
    private:
        ImguiRenderModel mCurrentModel{eRenderModel};
        EditorModel mCurrentEditorModel{eHighLightAABB};
        std::unordered_map<ImguiRenderModel, vkImguiTaskQueue> mTasks;
    };

} // yic

#endif //VKMMD_VKIMGUITASKQUEUE_H
