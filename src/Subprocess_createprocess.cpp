/*
 * Subprocess_createprocess.cpp
 *
 * Copyright (c) 2023 Marius Zwicker
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <array>
#include <iostream>
#include <cstring>

#include "Subprocess.h"
#include "Logging.h"
#include "Util.h"

#include "config.h"

#if LINTER_CACHE_HAVE_CREATE_PROCESS
    #include <Windows.h>
    #undef ERROR
#endif

static std::string
makeExe(const std::string& filename)
{
    auto const ext = filename.substr(filename.size() - 4);
    if (ext == ".exe" || ext == ".bat") {
        return filename;
    }
    return filename + ".exe";
}

static std::string
lastErrorString(DWORD dw = -1)
{
    LPVOID lpMsgBuf;
    if (dw < 0) {
        dw = GetLastError();
    }

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr,
                   dw,
                   MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                   reinterpret_cast<char*>(&lpMsgBuf),
                   0,
                   nullptr);

    std::string error;
    if (lpMsgBuf != nullptr) {
        error = reinterpret_cast<const char*>(lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
    return error;
}

static DWORD
createPipeEx(PHANDLE readPipe,
             PHANDLE writePipe,
             LPSECURITY_ATTRIBUTES attributes,
             DWORD size)
{
    static int pipeIndex = 0;
    const auto pipeName = "\\\\.\\pipe\\LinterCache" +
                          std::to_string(GetCurrentProcessId()) +
                          std::to_string(++pipeIndex);
    *readPipe = CreateNamedPipeA(pipeName.c_str(),
                                 PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
                                 PIPE_TYPE_BYTE,
                                 1,
                                 size,
                                 size,
                                 INFINITE,
                                 attributes);
    if (INVALID_HANDLE_VALUE == *readPipe) {
        return FALSE;
    }
    *writePipe = CreateFileA(pipeName.c_str(),
                             GENERIC_WRITE,
                             0,
                             attributes,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             nullptr);
    if (INVALID_HANDLE_VALUE == *writePipe) {
        CloseHandle(*readPipe);
        return FALSE;
    }
    return TRUE;
}

struct DrainHelper
{
    DrainHelper()
    {
        memset(&overlapped, 0, sizeof(overlapped));
        overlapped.hEvent = this;
    }

    void run(HANDLE fd, std::string& buffer)
    {
        running = false;
        this->buffer = &buffer;
        initialSize = buffer.size();

        constexpr size_t incrementSize = 1024;
        buffer.resize(initialSize + incrementSize);
        ReadFileEx(fd,
                   buffer.data() + initialSize,
                   incrementSize,
                   &overlapped,
                   &DrainHelper::completionRoutine);
        auto dw = GetLastError();
        if (dw == ERROR_BROKEN_PIPE) {
            // ok, process has ended
            buffer.resize(initialSize);
        } else if (dw != ERROR_SUCCESS) {
            LOG(ERROR) << "ReadFileEx failed: " << dw << "/"
                       << lastErrorString(dw);
            buffer.resize(initialSize);
        } else {
            running = true;
        }
    }

    static void completionRoutine(DWORD /* errorCode */,
                                  DWORD bytesTransferred,
                                  LPOVERLAPPED overlapped)
    {
        auto* helper = static_cast<DrainHelper*>(overlapped->hEvent);
        if (bytesTransferred >= 0) {
            helper->buffer->resize(helper->initialSize + bytesTransferred);
        }
        helper->running = false;
    }

    bool running = false;
    OVERLAPPED overlapped;
    std::string* buffer = nullptr;
    size_t initialSize = 0;
};

void
Process::run()
{
    const auto cmd = "\"" + _cmd.join("\" \"") + "\"";
    _stdout.clear();
    _stderr.clear();

    if (_cmd.empty()) {
        LOG(ERROR) << "Empty command: '" << cmd << "'";
        throw ProcessError(cmd, -1);
    }

    // https://learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
    SECURITY_ATTRIBUTES pipeAttr;
    pipeAttr.nLength = sizeof(pipeAttr);
    pipeAttr.bInheritHandle = TRUE;
    pipeAttr.lpSecurityDescriptor = nullptr;

    HANDLE stdout_fd[2];
    if (!createPipeEx(&stdout_fd[0], &stdout_fd[1], &pipeAttr, 0)) {
        LOG(ERROR) << "Failed to prepare stdout pipe: " << lastErrorString();
        throw ProcessError(
          "Failed to prepare stdout pipe: " + lastErrorString(), -1);
    }
    SetHandleInformation(stdout_fd[0], HANDLE_FLAG_INHERIT, 0);

    HANDLE stderr_fd[2];
    if (!createPipeEx(&stderr_fd[0], &stderr_fd[1], &pipeAttr, 0)) {
        LOG(ERROR) << "Failed to prepare stdout pipe: " << lastErrorString();
        throw ProcessError(
          "Failed to prepare stdout pipe: " + lastErrorString(), -1);
    }
    SetHandleInformation(stderr_fd[0], HANDLE_FLAG_INHERIT, 0);

    STARTUPINFO startInfo;
    ZeroMemory(&startInfo, sizeof(startInfo));
    startInfo.cb = sizeof(startInfo);
    if (0 == (_flags & Process::CAPTURE_STDERR)) {
        startInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    } else {
        startInfo.hStdError = stderr_fd[1];
    }
    if (0 == (_flags & Process::CAPTURE_STDOUT)) {
        startInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    } else {
        startInfo.hStdOutput = stdout_fd[1];
    }
    startInfo.hStdInput = GetStdHandle(STD_ERROR_HANDLE);
    startInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION procInfo;
    ZeroMemory(&procInfo, sizeof(procInfo));

    auto file = makeExe(_cmd[0]);
    if (Util::is_file(file)) {
        // good to go
    } else {
        auto path = StringList::split(getenv("PATH"), ';');
        for (auto candidate : path) {
            candidate += "/" + file;
            if (Util::is_file(candidate)) {
                file = candidate;
                break;
            }
        }
    }

    std::vector<char> cmdline(cmd.size() + 1);
    std::memcpy(cmdline.data(), cmd.c_str(), cmd.size());
    cmdline.back() = 0;
    auto result = CreateProcessA(file.c_str() /* module */,
                                 cmdline.data() /* cmdline */,
                                 nullptr /* sec attr */,
                                 nullptr /* thread attr */,
                                 TRUE /* inherit handles */,
                                 0 /* creation flags */,
                                 nullptr /* use our env */,
                                 nullptr /* use our wkdir */,
                                 &startInfo,
                                 &procInfo);
    if (!result) {
        LOG(ERROR) << "Failed to create process: " << lastErrorString();
        throw ProcessError(
          "Failed to create " + file + ": " + lastErrorString(), -1);
    }

    CloseHandle(procInfo.hThread);
    CloseHandle(stderr_fd[1]);
    CloseHandle(stdout_fd[1]);

    DrainHelper drainStderr;
    DrainHelper drainStdout;
    while (true) {
        if (0 != (_flags & Process::CAPTURE_STDERR) && !drainStderr.running) {
            // pull everything from stderr
            drainStderr.run(stderr_fd[0], _stderr);
        }
        if (0 != (_flags & Process::CAPTURE_STDOUT) && !drainStdout.running) {
            // pull everything from stdout
            drainStdout.run(stdout_fd[0], _stdout);
        }

        auto result = WaitForSingleObjectEx(
          procInfo.hProcess, INFINITE, /* alertable */ TRUE);
        if (result == WAIT_FAILED) {
            LOG(ERROR) << "Failed to wait: " << lastErrorString();
            break;
        }
        if (result == WAIT_OBJECT_0) {
            break;
        }
    }

    while (drainStderr.running || drainStdout.running) {
        SleepEx(50, true);
    }

    DWORD exitCode = 1;
    GetExitCodeProcess(procInfo.hProcess, &exitCode);
    CloseHandle(procInfo.hProcess);
    _exitCode = static_cast<int>(exitCode);

    if (0 != _exitCode) {
        throw ProcessError(cmd, _exitCode);
    }
}
