//
// Created by lenovo on 4/16/2024.
//

#include <log/Log.h>
#include <regex>
#include "ImGuiTerminal.h"

namespace yic {

    ImGuiTerminal::ImGuiTerminal() {
        mConfig = winpty_config_new(0, nullptr);
        mPty = winpty_open(mConfig, nullptr);

        winpty_set_size(mPty, 100, 80, nullptr);

        std::wstring cmdPath = L"C:\\Windows\\System32\\cmd.exe";
        mSpawnConfig = winpty_spawn_config_new(WINPTY_SPAWN_FLAG_AUTO_SHUTDOWN,
                                               cmdPath.c_str(),
                                               nullptr, nullptr, nullptr, nullptr);

        DWORD createProcessError;
        winpty_spawn(mPty, mSpawnConfig, nullptr, nullptr, &createProcessError, nullptr);

        LPCWSTR conOutPipeName = winpty_conout_name(mPty);
        mOutputReadHandle = CreateFileW(conOutPipeName, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);

        LPCWSTR conInputPipeName = winpty_conin_name(mPty);
        mInputWriteHandle = CreateFileW(conInputPipeName, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

        mBuf.resize(4096);
    }

    ImGuiTerminal::~ImGuiTerminal() {
        clearLine();
        winpty_free(mPty);
        winpty_config_free(mConfig);
        winpty_spawn_config_free(mSpawnConfig);
    }

    bool ImGuiTerminal::addLine(const std::string &line) {
        mLines.emplace_back(line);
        if (mLines.size() > 500){
            mLines.pop_front();
        }

        return true;
    }

    bool ImGuiTerminal::clearLine() {
        mLines.clear();

        return true;
    }

    bool ImGuiTerminal::displayLines() {
        for(const auto& line : mLines){
            ImGui::TextUnformatted(line.c_str());
        }

        for(const auto& colorLine : mColorLines){
            ImGui::TextColored(colorLine.color, "%s", colorLine.message.c_str());
        }

        return true;
    }

    bool ImGuiTerminal::update() {
        DWORD bytesRead;

        if (PeekNamedPipe(mOutputReadHandle, nullptr, 0, nullptr, &bytesRead, nullptr) && bytesRead > 0){
            ReadFile(mOutputReadHandle, mBuf.data(), mBuf.size(), &bytesRead, nullptr);

            std::string temp(mBuf.data(), bytesRead);

            std::size_t pos = 0;
            std::size_t lastPos = 0;
            while ((pos = temp.find('\n', lastPos)) != std::string::npos){
                std::string line = temp.substr(lastPos, pos - lastPos);
                std::string cleanded = removeAnsiEscapeCodes(temp);
                addLine(line);
                lastPos = pos + 1;
            }

            if (lastPos < temp.size()){
                std::string cleanded = removeAnsiEscapeCodes(temp);
                addLine(temp.substr(lastPos));
            }
        }

        return true;
    }

    bool ImGuiTerminal::draw() {
        ImGuiStyle &yStyle = ImGui::GetStyle();
        yStyle.Colors[ImGuiCol_WindowBg].w = 0.85f;
        ImGui::Begin("Terminal");
        displayLines();

        char inputBuf[255] = "";
        if (ImGui::InputText("Input", inputBuf, sizeof (inputBuf), ImGuiInputTextFlags_EnterReturnsTrue)){
            std::string inputStr = std::string (inputBuf);

            sendInput(inputStr);

            inputBuf[0] = '\0';
        }

        if (mScrollToBottom){
            ImGui::SetScrollHereY(1.f);
            mScrollToBottom = false;
        }

        ImGui::End();

        return true;
    }

    bool ImGuiTerminal::sendInput(const std::string &input) {
        DWORD bytesWritten;
        WriteFile(mInputWriteHandle, input.c_str(), input.length(), &bytesWritten, nullptr);

        return true;
    }

    bool ImGuiTerminal::addLog(const std::string &line) {
        std::lock_guard<std::mutex> lock(mMutex);
        mLines.push_back(line);
        if (mLines.size() > 500){
            mLines.pop_front();
        }

        return true;
    }

    bool ImGuiTerminal::addColorLog(const std::string &line, ImVec4 color) {
        std::lock_guard<std::mutex> lock(mMutex);
        mColorLines.emplace_back(line, color);
        if (mColorLines.size() > 500){
            mColorLines.pop_front();
        }

        mScrollToBottom = true;

        return true;
    }

    std::string ImGuiTerminal::removeAnsiEscapeCodes(const std::string& text) {
        static const std::regex ansi_escape_codes(R"(\x1B\[[0-?]*[ -/]*[@-~])");
        return std::regex_replace(text, ansi_escape_codes, "");
    }

} // yic