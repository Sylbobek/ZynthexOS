#include "sched/sched.h"
#include "video/vga.h"
#include "core/time.h"

static void test_task_a(void)
{
    uint32_t counter = 0;
    while (1)
    {
        vga_print("Task A: ");
        vga_print_dec(counter++);
        vga_print(" ticks=");
        vga_print_dec(timer_get_ticks());
        vga_print("\n");
        for (volatile int i = 0; i < 10000000; ++i) {} // busy wait to be preempted
    }
}

static void test_task_b(void)
{
    uint32_t counter = 0;
    while (1)
    {
        vga_print("Task B: ");
        vga_print_dec(counter++);
        vga_print(" ticks=");
        vga_print_dec(timer_get_ticks());
        vga_print("\n");
        for (volatile int i = 0; i < 10000000; ++i) {} // busy wait to be preempted
    }
}

void scheduler_create_test_tasks(void)
{
    task_create_kernel(test_task_a);
    task_create_kernel(test_task_b);
}
