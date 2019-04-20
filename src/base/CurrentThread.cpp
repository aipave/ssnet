#include <sys/syscall.h>
#include <unistd.h>

#include "CurrentThread.h"

namespace ssnet {
thread_local int t_threadTid = 0;

int GetThreadID() {
    if (t_threadTid == 0) {
        t_threadTid = ::syscall(SYS_gettid);
    }
    return t_threadTid;
}
} // namespace ssnet