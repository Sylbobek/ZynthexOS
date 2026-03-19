#include "sched.h"
#include "mm/pmm.h"
#include "mm/heap.h"
#include "arch/x86/tss.h"
#include "core/log.h"

extern void scheduler_create_test_tasks(void);

static task_t* current_task = NULL;
static task_t* ready_queue = NULL;
static uint32_t next_pid = 1;

void scheduler_init(void)
{
    log_write("sched: init\n");
    scheduler_create_test_tasks();
}

void scheduler_add_task(task_t* task)
{
    if (!ready_queue)
    {
        ready_queue = task;
        task->next = task;  // circular list with one element
    }
    else
    {
        // Insert after current tail (the one pointing to head)
        task_t* tail = ready_queue;
        while (tail->next != ready_queue)
            tail = tail->next;

        tail->next = task;
        task->next = ready_queue;
    }
}

task_t* scheduler_pick_next(void)
{
    if (!ready_queue)
        return NULL;

    task_t* next = ready_queue;
    while (next && next->state != TASK_READY)
    {
        next = next->next;
        if (next == ready_queue)  // full loop
            break;
    }

    if (next && next->state == TASK_READY)
    {
        // Rotate queue
        ready_queue = next->next;
        return next;
    }

    return NULL;  // no ready tasks
}

void scheduler_switch_to(task_t* new_task)
{
    if (!new_task)
        return;

    // Set TSS.esp0 for privilege transitions
    tss_set_kernel_stack((uint32_t)new_task->kernel_stack + KERNEL_STACK_SIZE);

    current_task = new_task;
    new_task->state = TASK_RUNNING;

    // For now, we will jump to the task entry point via asm stub later
    // Placeholder: store context and trigger switch in assembly
    extern void task_switch_asm(task_t* old, task_t* new);
    task_switch_asm(NULL, new_task);
}

void scheduler_yield(void)
{
    if (current_task)
    {
        current_task->state = TASK_READY;
        task_t* next = scheduler_pick_next();
        if (next && next != current_task)
            scheduler_switch_to(next);
    }
}

void scheduler_tick(void)
{
    scheduler_yield();
}

task_t* task_create_kernel(void (*entry)(void))
{
    uint8_t* stack = kmalloc(KERNEL_STACK_SIZE);
    if (!stack)
        return NULL;

    task_t* task = kmalloc(sizeof(task_t));
    if (!task)
    {
        kfree(stack);
        return NULL;
    }

    task->pid = next_pid++;
    task->state = TASK_READY;
    task->kernel_stack = stack;
    task->esp = (uint32_t)stack + KERNEL_STACK_SIZE;
    task->eip = (uint32_t)entry;
    task->next = NULL;

    // Push a dummy return address onto stack so entry can 'return' to a safe halt
    uint32_t* sp = (uint32_t*)task->esp;
    *(--sp) = (uint32_t)entry;  // return address
    task->esp = (uint32_t)sp;

    scheduler_add_task(task);
    return task;
}
