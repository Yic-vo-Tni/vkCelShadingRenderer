//
// Created by lenovo on 4/16/2024.
//

#ifndef VKMMD_IMGUITERMINAL_H
#define VKMMD_IMGUITERMINAL_H

#include "winpty-src/src/include/winpty.h"
#include "log/Log.h"

namespace yic {

    class ImGuiTerminal {
        struct colorLog{
            std::string message;
            ImVec4 color;
        };
    public:
        vkGet auto get = [](){ return Singleton<ImGuiTerminal>::get();};

        ImGuiTerminal();
        ~ImGuiTerminal();

        bool update();
        bool draw();

        bool sendInput(const std::string &input);
        bool addLog(const std::string& ling);
        bool addColorLog(const std::string& ling, ImVec4 color = ImVec4{1.f, 1.f, 1.f, 1.f});

    private:
        bool addLine(const std::string &line);
        bool clearLine();
        bool displayLines();

        std::string removeAnsiEscapeCodes(const std::string & text);

    private:
        winpty_t *mPty;
        winpty_config_t *mConfig;
        winpty_spawn_config_t *mSpawnConfig;
        HANDLE mOutputReadHandle;
        HANDLE mInputWriteHandle;
        std::vector<char> mBuf{};
        std::string mOutput{};
        std::deque<std::string> mLines{};
        std::deque<colorLog> mColorLines{};

        std::mutex mMutex;
        bool mScrollToBottom = true;
    };

    class ImGuiStreamBuf : public std::streambuf {
    public:
        vkGet auto get = [](){ return Singleton<ImGuiStreamBuf>::get();};
    protected:
        int_type overflow(int_type c) override {
            mBuf += static_cast<char>(c);
            if (c == '\n') {
                ImGuiTerminal::get()->addColorLog(mBuf);
                mBuf.clear();
            }
            return c;
        }

        std::streamsize xsputn(const char* s, std::streamsize n) override {
            mBuf.append(s, n);
            size_t pos;
            while ((pos = mBuf.find('\n')) != std::string::npos) {
                ImGuiTerminal::get()->addColorLog(mBuf.substr(0, pos));
                mBuf.erase(0, pos + 1);
            }
            return n;
        }

    private:
        std::string mBuf{};
    };


    template<typename T>
    class ImGuiSpdlogSink : public spdlog::sinks::base_sink<T> {
    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override{
            spdlog::memory_buf_t formatted;
            this->formatter_->format(msg, formatted);

            ImVec4 color;
            switch (msg.level) {
                case spdlog::level::info:
                    color = ImVec4(0.f, 1.f, 0.f, 1.f);
                    break;
                case spdlog::level::warn:
                    color = ImVec4(1.f, 1.f, 0.f, 1.f);
                    break;
                case spdlog::level::err:
                    color = ImVec4(1.f, 0.f, 0.f, 1.f);
                    break;
                default:
                    color = ImVec4(1.f, 1.f, 1.f, 1.f);
                    break;
            }
            //ImGuiTerminal::get()->addLog(fmt::to_string(formatted));
            ImGuiTerminal::get()->addColorLog(fmt::to_string(formatted), color);
        }

        void flush_() override {

        }
    };

    using ImGuiSpdlogSink_mt = ImGuiSpdlogSink<std::mutex>;

    inline void setup_logger() {
        Log::addSinks(std::make_shared<ImGuiSpdlogSink_mt>());
    }

} // yic

#endif //VKMMD_IMGUITERMINAL_H
