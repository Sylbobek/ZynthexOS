#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>

#define MAX_TASKS 16
#define KERNEL_STACK_SIZE 4096

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_ZOMBIE
} task_state_t;

typedef struct task {
    uint32_t pid;
    task_state_t state;
    uint8_t* kernel_stack;   // top of kernel stack for this task
    uint32_t esp;           // saved kernel esp for context switch
    uint32_t eip;           // entry point (for now kernel tasks only)
    struct task* next;
} task_t;

void scheduler_init(void);
void scheduler_tick(void);
void scheduler_add_task(task_t* task);
void scheduler_yield(void);
task_t* scheduler_pick_next(void);
void scheduler_switch_to(task_t* new_task);

// Simple task creator (kernel-only for now)
task_t* task_create_kernel(void (*entry)(void));
void scheduler_create_test_tasks(void);

#endif
