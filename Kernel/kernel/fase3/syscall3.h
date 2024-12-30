// syscall3.h
#ifndef _SYSCALL3_H_
#define _SYSCALL3_H_

#include <linux/sched.h>

int do_prlimit(struct task_struct *task, int resource, const struct rlimit *new_rlim, struct rlimit *old_rlim);

#endif // _SYSCALL3_H_

