/*
 * Process.cpp
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

#include "Process.h"
#include "Logging.h"

#include "config.h"

#if LINTER_CACHE_HAVE_PIDFD_OPEN
    #include <sys/syscall.h>
    #include <sys/poll.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif
#if LINTER_CACHE_HAVE_KEVENT
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/event.h>
#endif
#if LINTER_CACHE_HAVE_EXECVP
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
#elif LINTER_CACHE_HAVE_POPEN
    #include <cstdio>
#elif LINTER_CACHE_HAVE__POPEN
    #include <cstdio>

inline FILE*
popen(const char* command, const char* mode)
{
    return _popen(command, mode);
}

inline int
pclose(FILE* stream)
{
    return _pclose(stream);
}
#endif

#if LINTER_CACHE_HAVE_PIDFD_OPEN || LINTER_CACHE_HAVE_KEVENT
static std::string
drain_fd(int fd)
{
    std::string buffer;

    size_t written = 0;
    size_t remaining = buffer.size();

    while (true) {
        if (remaining <= 0) {
            buffer.resize(buffer.size() + 512);
            remaining = buffer.size() - written;
        }

        char* data = buffer.data() + written;

        auto const actual = read(fd, data, remaining);
        if (actual <= 0) {
            break;
        }

        written += actual;
        remaining -= actual;
    }
    buffer.resize(written);
    return buffer;
}
#endif

Process::Process(const StringList& cmd, Process::Flags flags)
  : _flags(flags)
  , _cmd(cmd)
  , _exitCode(-1)
{}

void
Process::run()
{
    const auto cmd = _cmd.join(" ");
    _stdout.clear();
    _stderr.clear();

    if (_cmd.empty()) {
        LOG(ERROR) << "Empty command: '" << cmd << "'";
        throw ProcessError(cmd, -1);
    }

#if LINTER_CACHE_HAVE_EXECVP

    int stdout_fd[2];
    if (pipe(stdout_fd)) {
        LOG(ERROR) << "Failed to prepare stdout pipe: " << strerror(errno);
        throw ProcessError(cmd, -1);
    }

    int stderr_fd[2];
    if (pipe(stderr_fd)) {
        LOG(ERROR) << "Failed to prepare stderr pipe: " << strerror(errno);
        throw ProcessError(cmd, -1);
    }

    const auto* const file = _cmd[0].c_str();
    std::vector<char*> argv;
    argv.reserve(_cmd.size());
    for (size_t i = 0; i < _cmd.size(); ++i) {
        argv.push_back(const_cast<char*>(_cmd[i].c_str()));
    }
    argv.push_back(nullptr);

    const auto pid = fork();
    if (pid < 0) {
        LOG(ERROR) << "Error forking child process: " << strerror(errno);
        throw ProcessError(cmd, -1);
    }
    if (pid == 0) {
        // Child: Do not use LOG(..) to avoid race with parent!

        if (dup2(stdout_fd[1], STDOUT_FILENO) < 0) {
            std::cerr << "Failed to redirect stdout: " << strerror(errno)
                      << std::endl;
            exit(1);
        }
        if (dup2(stderr_fd[1], STDERR_FILENO) < 0) {
            std::cerr << "Failed to redirect stderr: " << strerror(errno)
                      << std::endl;
            exit(1);
        }

        if (execvp(file, argv.data()) < 0) {
            std::cerr << "Failed to exec cmd: " << strerror(errno) << std::endl;
            exit(1);
        }
        exit(0);
    } else {
        // Parent: Wait on child
        close(stdout_fd[1]);
        close(stderr_fd[1]);

        // Use fds as nonblocking
        fcntl(stdout_fd[0], F_SETFL, O_NONBLOCK);
        fcntl(stderr_fd[0], F_SETFL, O_NONBLOCK);

        // Helper to drain all outputs
        auto drain_fds = [&] {
            // pull everything from stderr
            auto buffer = drain_fd(stderr_fd[0]);
            _stderr += buffer;
            if (0 != (_flags & Flags::FORWARD_OUTPUT)) {
                std::cerr << buffer.data();
            }
            // pull everything from stdout
            buffer = drain_fd(stdout_fd[0]);
            _stdout += buffer;
            if (0 != (_flags & Flags::FORWARD_OUTPUT)) {
                std::cout << buffer.data();
            }
        };

    #if LINTER_CACHE_HAVE_PIDFD_OPEN

        auto pid_fd = static_cast<int>(syscall(SYS_pidfd_open, pid, 0));
        if (pid_fd < 0) {
            LOG(ERROR) << "Failed to obtain fd for pid: " << strerror(errno);
            throw ProcessError(cmd, -1);
        }

        bool child_alive = true;
        while (child_alive) {
            std::array<struct pollfd, 3> polls;
            polls[0].fd = pid_fd;
            polls[0].events = POLLIN;
            polls[0].revents = 0;
            polls[1].fd = stdout_fd[0];
            polls[1].events = POLLIN;
            polls[1].revents = 0;
            polls[2].fd = stderr_fd[0];
            polls[2].events = POLLIN;
            polls[2].revents = 0;

            const auto idx = poll(polls.data(), polls.size(), -1);
            if (idx < 0) {
                if (EINTR == errno) {
                    continue;
                }
                child_alive = false;
            }
            if (polls[0].revents & POLLIN) {
                child_alive = false;
            }
            drain_fds();
        }

        close(polls[0].fd);

    #elif LINTER_CACHE_HAVE_KEVENT

        std::array<struct kevent, 3> events;
        EV_SET(&events[0],
               pid,
               EVFILT_PROC,
               EV_ENABLE | EV_ADD | EV_CLEAR,
               NOTE_EXIT,
               0,
               nullptr);
        EV_SET(&events[1], stderr_fd[0], EVFILT_READ, EV_ADD, 0, 0, nullptr);
        EV_SET(&events[2], stdout_fd[0], EVFILT_READ, EV_ADD, 0, 0, nullptr);

        int kq = kqueue();
        if (kq < 0) {
            LOG(ERROR) << "Failed to create kqueue: " << strerror(errno);
            throw ProcessError(cmd, -1);
        }

        std::array<struct kevent, events.size()> tevents;
        bool child_alive = true;
        while (child_alive) {
            auto nev = kevent(kq,
                              events.data(),
                              events.size(),
                              tevents.data(),
                              tevents.size(),
                              nullptr);
            if (nev < 0) {
                LOG(ERROR) << "Failed to wait on kqueue: " << strerror(errno);
                throw ProcessError(cmd, -1);
            }
            drain_fds();
            for (int i = 0; i < nev; ++i) {
                if (tevents[i].flags & EV_ERROR) {
                    LOG(ERROR) << "Error in kqueue: "
                               << strerror(static_cast<int>(tevents[i].data));
                    throw ProcessError(cmd, -1);
                }
                if (tevents[i].ident == pid) {
                    // break when the child has ended
                    child_alive = false;
                    break;
                }
            }
        }

    #else
        #error "Need either pidfd_open() or kevent() support"

    #endif
        int exitcode = -1;
        if (wait(&exitcode) == pid) {
            drain_fds();

            close(stdout_fd[0]);
            close(stderr_fd[0]);

            if (WIFEXITED(exitcode) && WEXITSTATUS(exitcode) == EXIT_SUCCESS) {
                // all good
            } else {
                throw ProcessError(cmd, WEXITSTATUS(exitcode));
            }
        }
    }

#elif LINTER_CACHE_HAVE_POPEN || LINTER_CACHE_HAVE__POPEN

    // FIXME(zwicker): popen() is easy but also lazy as it requires
    //                 and additional spawn of a shell
    auto* stdoutHandle = popen(cmd.c_str(), "r");
    if (nullptr == stdoutHandle) {
        throw ProcessError(cmd, -1);
    }

    std::array<char, 512> buffer;
    while (fgets(buffer.data(), buffer.size(), stdoutHandle)) {
        _stdout.append(buffer.data());
        if (0 != (_flags & Flags::FORWARD_OUTPUT)) {
            std::cout << buffer.data();
        }
        buffer[0] = '\0';
    }
    _stdout.append(buffer.data());
    _exitCode = pclose(stdoutHandle);
    if (0 != (_flags & Flags::FORWARD_OUTPUT)) {
        std::cout << std::flush;
    }
    if (0 != _exitCode) {
        throw ProcessError(cmd, _exitCode);
    }
#else
    #error "No process implementation on this platform
#endif
}
