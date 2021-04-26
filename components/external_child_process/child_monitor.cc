/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/external_child_process/child_monitor.h"

#if defined(OS_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#elif defined(OS_WIN)
#include <windows.h>
#endif

#include <utility>

#include "base/bind_post_task.h"
#include "base/logging.h"
#include "base/process/kill.h"
#include "base/single_thread_task_runner.h"
#include "base/task/post_task.h"
#include "base/threading/thread_task_runner_handle.h"

namespace brave {

namespace {

#if defined(OS_POSIX)
int pipehack[2];

static void SIGCHLDHandler(int signo) {
  int error = errno;
  char ch = 0;
  (void)write(pipehack[1], &ch, 1);
  errno = error;
}

static void SetupPipeHack() {
  if (pipe(pipehack) == -1)
    VLOG(0) << "pipehack errno:" << errno;

  int flags;
  for (size_t i = 0; i < 2; ++i) {
    if ((flags = fcntl(pipehack[i], F_GETFL)) == -1)
      VLOG(0) << "get flags errno:" << errno;
    // Nonblock write end on SIGCHLD handler which will notify monitor thread
    // by sending one byte to pipe whose read end is blocked and wait for
    // SIGCHLD to arrives to avoid busy reading
    if (i == 1)
      flags |= O_NONBLOCK;
    if (fcntl(pipehack[i], F_SETFL, flags) == -1)
      VLOG(0) << "set flags errno:" << errno;
    if ((flags = fcntl(pipehack[i], F_GETFD)) == -1)
      VLOG(0) << "get fd flags errno:" << errno;
    flags |= FD_CLOEXEC;
    if (fcntl(pipehack[i], F_SETFD, flags) == -1)
      VLOG(0) << "set fd flags errno:" << errno;
  }

  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = SIGCHLDHandler;
  sigaction(SIGCHLD, &action, NULL);
}

static void TearDownPipeHack() {
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = SIG_DFL;
  sigaction(SIGCHLD, &action, NULL);
  for (size_t i = 0; i < 2; ++i)
    close(pipehack[i]);
}
#endif

void MonitorChild(base::ProcessHandle p_handle,
                  base::OnceCallback<void(base::ProcessId)> callback) {
  DCHECK(callback);
#if defined(OS_POSIX)
  char buf[PIPE_BUF];

  while (1) {
    if (read(pipehack[0], buf, sizeof(buf)) > 0) {
      pid_t pid;
      int status;

      if ((pid = waitpid(base::GetProcId(p_handle), &status, WNOHANG)) != -1) {
        if (WIFSIGNALED(status)) {
          VLOG(0) << "child(" << pid << ") got terminated by signal "
                  << WTERMSIG(status);
        } else if (WCOREDUMP(status)) {
          VLOG(0) << "child(" << pid << ") coredumped";
        } else if (WIFEXITED(status)) {
          VLOG(0) << "child(" << pid << ") exit (" << WEXITSTATUS(status)
                  << ")";
        }
        std::move(callback).Run(pid);
      }
    } else {
      // pipes closed
      break;
    }
  }
#elif defined(OS_WIN)
  WaitForSingleObject(p_handle, INFINITE);
  std::move(callback).Run(base::GetProcId(p_handle));
#else
#error unsupported platforms
#endif
}

}  // namespace

ChildMonitor::ChildMonitor()
    : child_monitor_thread_(new base::Thread("child_monitor_thread")) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
#if defined(OS_POSIX)
  SetupPipeHack();
#endif
  if (!child_monitor_thread_->Start()) {
    NOTREACHED();
  }
}
ChildMonitor::~ChildMonitor() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
#if defined(OS_POSIX)
  TearDownPipeHack();
#endif

  if (child_process_.IsValid()) {
    child_process_.Terminate(0, true);
#if defined(OS_MAC)
    // TODO(https://crbug.com/806451): The Mac implementation currently blocks
    // the calling thread for up to two seconds.
    base::PostTask(
        FROM_HERE,
        {base::ThreadPool(), base::MayBlock(), base::TaskPriority::BEST_EFFORT},
        base::BindOnce(&base::EnsureProcessTerminated,
                       Passed(&child_process_)));
#else
    base::EnsureProcessTerminated(std::move(child_process_));
#endif
  }
}

void ChildMonitor::Start(base::Process child,
                         base::OnceCallback<void(base::ProcessId)> callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  child_process_ = std::move(child);

  DCHECK(child_monitor_thread_.get());
  child_monitor_thread_->task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(
          &MonitorChild, child_process_.Handle(),
          base::BindPostTask(base::SequencedTaskRunnerHandle::Get(),
                             base::BindOnce(&ChildMonitor::OnChildCrash,
                                            weak_ptr_factory_.GetWeakPtr(),
                                            std::move(callback)))));
}

void ChildMonitor::OnChildCrash(
    base::OnceCallback<void(base::ProcessId)> callback,
    base::ProcessId pid) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(callback);

  child_process_.Close();
  std::move(callback).Run(pid);
}

}  // namespace brave
