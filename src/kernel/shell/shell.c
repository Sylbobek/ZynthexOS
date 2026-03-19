#include "shell.h"
#include "drivers/keyboard.h"
#include "core/console.h"
#include "video/vga.h"
#include "core/debug.h"
#include "lib/string.h"
#include <stddef.h>
#include "core/time.h"
#include "arch/x86/cpuid.h"
#include "mm/pmm.h"
#include "mm/heap.h"
#include "include/system.h"
#include <stdint.h>
#include "fs/vfs/vfs.h"
#include "fs/fd/fd.h"
#include "core/log.h"


#define MAX_ARGS 8

static void execute_command(char* input);
static void parse_arguments(char* input, char* argv[], int* argc);

static void* last_alloc = NULL;
static vfs_node_t* cwd = NULL;

int mode = 0; // 0 = kernel, 1 = root, 2 = user

static void cmd_cur(int argc, char* argv[])
{
    if (argc != 2)
    {
        vga_print("Usage: cur <0-4>\n");
        return;
    }

    if (strcmp(argv[1], "vfs") == 0)
    {
        vga_print("[VFS]\n");
        vga_print(" ls\n");
        vga_print(" cd <path>\n");
        vga_print(" mk <file>\n");
        vga_print(" mkr <file>\n");
        vga_print(" rd <file>\n");
        vga_print(" write <file> <data...>\n");
        vga_print(" fsseed\n");
        return;
    }

    int preset = atoi(argv[1]);

    switch (preset)
    {
        case 0:
            vga_disable_cursor(); // hidden
            vga_print("Cursor: disabled\n");
            break;
        case 1:
            vga_enable_cursor(14, 15); // classic underline
            vga_print("Cursor: underline\n");
            break;

        case 2:
            vga_enable_cursor(8, 15); // half block
            vga_print("Cursor: half block\n");
            break;

        case 3:
            vga_enable_cursor(0, 15); // full block
            vga_print("Cursor: full block\n");
            break;

        case 4:
            vga_enable_cursor(15, 15); // thin line
            vga_print("Cursor: thin line\n");
            break;
        default:
            vga_print("Invalid preset (1-4)\n");
            break;
    }
}


static void cmd_memsummary(int argc, char* argv[])
{
    (void)argc; (void)argv;

    uint32_t total = pmm_get_total_frames();
    uint32_t free  = pmm_get_free_frames();
    uint32_t used  = total - free;

    uint32_t total_kb = total * 4;
    uint32_t used_kb  = used * 4;

    vga_print("Memory Summary:\n");
    vga_print("  Used:  "); vga_print_dec(used); vga_print(" frames (" ); vga_print_dec(used_kb); vga_print(" KiB)\n");
    vga_print("  Total: "); vga_print_dec(total); vga_print(" frames (" ); vga_print_dec(total_kb); vga_print(" KiB)\n");
}


static void cmd_heapcheck(int argc, char* argv[])
{
    (void)argc; (void)argv;
    heap_validate();
    vga_print("Heap validation completed.\n");
}


static void cmd_fsseed(int argc, char* argv[])
{
    (void)argc; (void)argv;

    // Ensure root file
    const char* about_text = "zynthexos: minimal RAMFS seed\nThis is a demo file.\n";
    vfs_node_t* about = vfs_lookup("/about.txt");
    if (!about)
    {
        if (vfs_root && vfs_root->ops && vfs_root->ops->create(vfs_root, "about.txt", VFS_FILE) == 0)
            about = vfs_lookup("/about.txt");
    }

    if (about && about->ops && about->ops->write)
    {
        about->ops->write(about, 0, strlen(about_text), (uint8_t*)about_text);
        vga_print("Seeded /about.txt\n");
    }
    else
    {
        vga_print("Failed to seed /about.txt\n");
    }

    // Ensure /tmp/hello.txt
    vfs_node_t* tmp = vfs_lookup("/tmp");
    const char* tmp_text = "hello from /tmp on zynthexos\n";
    if (tmp && (tmp->flags & VFS_DIR) && tmp->ops && tmp->ops->create(tmp, "hello.txt", VFS_FILE) == 0)
    {
        vfs_node_t* hello = vfs_lookup("/tmp/hello.txt");
        if (hello && hello->ops && hello->ops->write)
        {
            hello->ops->write(hello, 0, strlen(tmp_text), (uint8_t*)tmp_text);
            vga_print("Seeded /tmp/hello.txt\n");
        }
    }
    else
    {
        vga_print("Failed to seed /tmp/hello.txt\n");
    }
}


static void cmd_cd(int argc, char* argv[])
{
    if (argc < 2)
    {
        cwd = vfs_root;
        return;
    }

    vfs_node_t* target;

    // Absolute path
    if (argv[1][0] == '/')
        target = vfs_lookup(argv[1]);
    else
        target = path_resolve(cwd, argv[1]);

    if (!target)
    {
        vga_print("Path not found\n");
        return;
    }

    if (!(target->flags & VFS_DIR))
    {
        vga_print("Not a directory\n");
        return;
    }

    cwd = target;
}


static void cmd_ls(int argc, char* argv[])
{
    vfs_node_t* dir;

    if (argc >= 2)
    {
        // Absolute path
        if (argv[1][0] == '/')
            dir = vfs_lookup(argv[1]);
        else
            dir = path_resolve(cwd, argv[1]);
    }
    else
    {
        dir = cwd;   
    }

    if (!dir)
    {
        vga_print("Path not found\n");
        return;
    }

    if (!(dir->flags & VFS_DIR))
    {
        vga_print("Not a directory\n");
        return;
    }

    if (!dir->ops->list)
    {
        vga_print("List not supported\n");
        return;
    }

    dir->ops->list(dir);
}




static void cmd_write(int argc, char* argv[])
{
    if (argc < 3)
    {
        vga_print("Usage: write <file> <text>\n");
        return;
    }

    int fd = sys_open(argv[1]);

    if (fd < 0)
    {
        vga_print("File not found\n");
        return;
    }

    sys_seek(fd, 0);

    for (int i = 2; i < argc; i++)
    {
        sys_write(fd, (uint8_t*)argv[i], strlen(argv[i]));

        if (i < argc - 1)
            sys_write(fd, (uint8_t*)" ", 1);
    }

    sys_close(fd);

    vga_print("Written.\n");
}


static void cmd_rd(int argc, char* argv[])
{
    if (argc < 2)
    {
        vga_print("Usage: rd <file>\n");
        return;
    }

    int fd = sys_open(argv[1]);

    if (fd < 0)
    {
        vga_print("File not found\n");
        return;
    }

    char buffer[256];
    memset(buffer, 0, sizeof(buffer));

    sys_seek(fd, 0);
    int read = sys_read(fd, (uint8_t*)buffer, sizeof(buffer) - 1);

    if (read > 0)
        vga_print(buffer);

    vga_print("\n");
    sys_close(fd);
}


static void cmd_mk(int argc, char* argv[])
{
    if (argc < 2)
    {
        vga_print("Usage: mk <file>\n");
        return;
    }

    if (!cwd)
    {
        vga_print("No working directory\n");
        return;
    }

    if (cwd->ops->create(cwd, argv[1], VFS_FILE) != 0)
    {
        vga_print("Failed to create file\n");
        return;
    }

    vga_print("File created\n");
}


static void cmd_mkr(int argc, char* argv[])
{
    if (argc < 2)
    {
        vga_print("Usage: mkr <file>\n");
        return;
    }

    // Resolve /tmp
    vfs_node_t* tmp = vfs_lookup("/tmp");

    if (!tmp)
    {
        vga_print("/tmp not found\n");
        return;
    }

    if (!(tmp->flags & VFS_DIR))
    {
        vga_print("/tmp is not a directory\n");
        return;
    }

    if (tmp->ops->create(tmp, argv[1], VFS_FILE) != 0)
    {
        vga_print("Failed to create file\n");
        return;
    }

    vga_print("RAM file created\n");
}


static void cmd_inttoascii(int argc, char* argv[])
{
    if (argc < 2)
    {
        vga_print("Usage: inttoascii <ascii codes...>\n");
        return;
    }

    vga_print("ASCII: ");

    for (int i = 1; i < argc; i++)
    {
        int value = atoi(argv[i]);

        if (value < 0 || value > 255)
        {
            vga_print("Invalid ASCII code\n");
            return;
        }

        char c = (char)value;
        vga_putchar(c);
    }

    vga_print("\n");
}



static void cmd_asciitoint(int argc, char* argv[])
{
    if (argc != 2)
    {
        vga_print("Usage: asciitoint <anything>\n");
        return;
    }

    vga_print("\nASCII code: ");
    for (int i = 0; argv[1][i] != '\0'; i++)
    {
        vga_print(" ");
        vga_print_dec((int)argv[1][i]);
    }

    vga_print("\n");
}

static void cmd_calc(int argc, char* argv[])
{
    if (argc != 4)
    {
        vga_print("Usage: eval <a> <op> <b>\n");
        return;
    }

    int a = atoi(argv[1]);
    int b = atoi(argv[3]);
    char op = argv[2][0];

    uint32_t result = 0;

    switch (op)
    {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/': 
            if (b != 0)
                result = a / b;
            else
                vga_print("Division by zero!\n");
                return;
            break;
    }

    char buf[32];
    utoa(result, buf, 10);
    vga_print(buf);
    vga_print("\n");
}

static void cmd_echo(int argc, char* argv[])
{
    static char buffer[1024];
    buffer[0] = '\0';

    for (int i = 1; i < argc; i++)
    {
        if (i > 1)
            strcat(buffer, " ");

        strcat(buffer, argv[i]);
    }

    vga_print(buffer);
    vga_print("\n");

}

static void cmd_heapval(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    heap_validate();
}

static void cmd_clear(int argc, char* argv[])
{
    vga_clear();
}

static void cmd_memdump(int argc, char* argv[])
{
    if (argc < 3)
    {
        vga_print("Usage: memdump <addr> <bytes>\n");
        return;
    }

    uint32_t addr = atoi_hex(argv[1]);
    int bytes = atoi(argv[2]);

    if (bytes <= 0)
    {
        vga_print("Invalid byte count\n");
        return;
    }

    uint8_t* ptr = (uint8_t*)addr;

    for (int i = 0; i < bytes; i++)
    {
        if (i % 16 == 0)
        {
            vga_print("\n");
            vga_print_hex((uint32_t)(ptr + i));
            vga_print(": ");
        }

        uint8_t val = ptr[i];

        if (val < 16)
            vga_print("0");

        vga_print_hex(val);
        vga_print(" ");
    }
    vga_print("\n");
}

static void cmd_free(int argc, char* argv[])
{
    if (argc < 2)
    {
        vga_print("Usage: free <address>\n");
        return;
    }

    void* ptr = (void*)atoi_hex(argv[1]);

    if (!ptr)
    {
        vga_print("Invalid address\n");
        return;
    }

    kfree(ptr);

    vga_print("Freed: ");
    vga_print_hex((uint32_t)ptr);
    vga_print("\n");
}


static void cmd_freelast(int argc, char* argv[])
{
    (void)argc; (void)argv;

    if (!last_alloc)
    {
        vga_print("No previous allocation\n");
        return;
    }

    kfree(last_alloc);

    vga_print("Freed: ");
    vga_print_hex((uint32_t)last_alloc);
    vga_print("\n");

    last_alloc = NULL;
}


static void cmd_alloc(int argc, char* argv[])
{
    if (argc < 2)
    {
        vga_print("Usage: alloc <size>\n");
        return;
    }

    int size = atoi(argv[1]);

    if (size <= 0)
    {
        vga_print("Invalid size\n");
        return;
    }

    void* ptr = kmalloc((size_t)size);

    if (!ptr)
    {
        vga_print("Allocation failed\n");
        return;
    }

    last_alloc = ptr;

    vga_print("Allocated at: ");
    vga_print_hex((uint32_t)ptr);
    vga_print("\n");
}





static void cmd_heapinfo(int argc, char* argv[])
{
    (void)argc; (void)argv;
    heap_dump();
}


static void cmd_meminfo(int argc, char* argv[])
{
    (void)argc; (void)argv;

    uint32_t total = pmm_get_total_frames();
    uint32_t free  = pmm_get_free_frames();

    vga_print("Physical Memory:\n");
    vga_print("  Total frames: ");
    vga_print_dec(total);
    vga_print("\n");

    vga_print("  Free frames:  ");
    vga_print_dec(free);
    vga_print("\n");

    vga_print("  Used frames:  ");
    vga_print_dec(total - free);
    vga_print("\n");
}


static void cmd_cpuid(int argc, char* argv[])
{
    (void)argc; (void)argv;

    static char brand[49];

    cpu_get_brand(brand);

    vga_print("CPU: ");
    vga_print(brand);
    vga_print("\n");
}


static void cmd_uptime(int argc, char* argv[])
{
    (void)argc; (void)argv;

    uint32_t secs = timer_get_seconds();

    vga_print("Uptime: ");
    vga_print_dec(secs);
    vga_print(" seconds\n");
}

static void cmd_help(int argc, char* argv[])
{
    // No arguments -> show categories overview
    if (argc == 1)
    {
        vga_print("[SYS]\n");
        vga_print(" help      sysfetch   cpuid\n");
        vga_print(" uptime    state      panic\n");
        vga_print(" dmesg     doom       shutdown   reboot\n");
        vga_print(" clear\n");

        vga_print("\n[MEM]\n");
        vga_print(" meminfo   memsummary heapinfo   heapval\n");
        vga_print(" alloc     free       freelast\n");
        vga_print(" memdump   heapcheck\n");

        vga_print("\n[UTIL]\n");
        vga_print(" echo      calc       cur\n");
        vga_print(" asciitoint inttoascii\n");

        vga_print("\n[VFS]\n");
        vga_print(" ls        cd         mk        mkr\n");
        vga_print(" rd        write      fsseed\n");

        return;
    }

    if (strcmp(argv[1], "sys") == 0)
    {
        vga_print("[SYS]\n");
        vga_print(" help\n");
        vga_print(" sysfetch\n");
        vga_print(" cpuid\n");
        vga_print(" uptime\n");
        vga_print(" state\n");
        vga_print(" panic <msg>\n");
        vga_print(" dmesg\n");
        vga_print(" doom\n");
        vga_print(" shutdown\n");
        vga_print(" reboot\n");
        vga_print(" clear\n");
        return;
    }

    if (strcmp(argv[1], "mem") == 0)
    {
        vga_print("[MEM]\n");
        vga_print(" meminfo\n");
        vga_print(" memsummary\n");
        vga_print(" heapinfo\n");
        vga_print(" heapval\n");
        vga_print(" heapcheck\n");
        vga_print(" alloc <size>\n");
        vga_print(" free <addr>\n");
        vga_print(" freelast\n");
        vga_print(" memdump <addr> <n>\n");
        return;
    }

    if (strcmp(argv[1], "util") == 0)
    {
        vga_print("[UTIL]\n");
        vga_print(" echo <text>\n");
        vga_print(" calc <a> <op> <b>\n");
        vga_print(" cur <0-4>\n");
        vga_print(" asciitoint <text>\n");
        vga_print(" inttoascii <nums>\n");

        vga_print("\ncur presets:\n");
        vga_print(" 0 - disabled\n");
        vga_print(" 1 - underline\n");
        vga_print(" 2 - half block\n");
        vga_print(" 3 - full block\n");
        vga_print(" 4 - thin line\n");

        return;
    }

    vga_print("Unknown help category\n");
}




static void cmd_sysfetch(int argc, char* argv[])
{
    (void)argc; (void)argv;

    static char brand[49];
    cpu_get_brand(brand);

    uint32_t total = pmm_get_total_frames();
    uint32_t free  = pmm_get_free_frames();
    uint32_t secs  = timer_get_seconds();

    vga_print_color(boot_logo, BOOT_LOGO_COLOR);

    vga_print_color("Kernel build date: ", BOOT_LOGO_COLOR);
    vga_print_color(KERNEL_BUILD_DATE, 0xC);
    vga_print_color("\nVersion: ", BOOT_LOGO_COLOR);
    vga_print_color(KERNEL_VERSION, 0xC);

    vga_print("\n\nCPU:        ");
    vga_print(brand);
    vga_print("\n");

    vga_print("Uptime:     ");
    vga_print_dec(secs);
    vga_print(" seconds\n");

    vga_print("Memory:     ");
    vga_print_dec(total - free);
    vga_print("/");
    vga_print_dec(total);
    vga_print(" frames used\n");

    vga_print("Mode:       ");
    if (mode == 0)
        vga_print("kernel\n");
    else if (mode == 1)
        vga_print("root\n");
    else
        vga_print("user\n");
}



static void cmd_panic(int argc, char* argv[])
{
    if (argc < 2)
    {
        debug_panic("Manual panic triggered");
        return;
    }

    static char buffer[128];
    buffer[0] = '\0';

    for (int i = 1; i < argc; i++)
    {
        if (i > 1)
            strcat(buffer, " ");

        strcat(buffer, argv[i]);
    }

    debug_panic(buffer);
}

static void cmd_dmesg(int argc, char* argv[])
{
    (void)argc; (void)argv;
    log_dump();
}

static void cmd_doom(int argc, char* argv[])
{
    (void)argc; (void)argv;
    vga_print("DOOM placeholder:\n");
    vga_print(" - Wymaga: framebuffer VBE/GOP, scheduler + syscall, userland, WAD.\n");
    vga_print(" - Obecnie brak: tryb graficzny i procesy.\n");
    vga_print(" - Uruchomimy po dodaniu grafiki+userland.\n");
}


static void cmd_state(int argc, char* argv[])
{
    (void)argc; (void)argv;
    debug_print_segments();
}

static void cmd_shutdown(int argc, char* argv[])
{
    (void)argc; (void)argv;

    vga_print("Shutting down...\n");

    // QEMU poweroff
    outw(0x604, 0x2000);

    sleep(1000);


    vga_print_color("Unable to shutdown\n", 0xC);
}

static void cmd_reboot(int argc, char* argv[])
{
    (void)argc; (void)argv;

    vga_print("Rebooting...\n");

    // QEMU reboot
    outb(0x64, 0xFE);

    sleep(1000);

    vga_print_color("Unable to reboot\n", 0xC);
}


typedef struct {
    const char* name;
    void (*handler)(int argc, char* argv[]);
} command_t;

static command_t commands[] = {
    {"help",  cmd_help},
    {"panic", cmd_panic},
    {"dmesg", cmd_dmesg},
    {"doom",  cmd_doom},
    {"state", cmd_state},
    {"uptime", cmd_uptime},
    {"cpuid", cmd_cpuid},
    {"meminfo", cmd_meminfo},
    {"memsummary", cmd_memsummary},
    {"heapinfo", cmd_heapinfo},
    {"heapcheck", cmd_heapcheck},
    {"alloc", cmd_alloc},
    {"free", cmd_free},
    {"freelast", cmd_freelast},
    {"memdump", cmd_memdump},
    {"heapval", cmd_heapval},
    {"sysfetch", cmd_sysfetch},
    {"shutdown", cmd_shutdown},  // FIXME: Only for QEMU
    {"reboot", cmd_reboot},      // FIXME: Only for QEMU
    {"clear", cmd_clear},
    {"echo", cmd_echo},
    {"calc", cmd_calc},
    {"asciitoint", cmd_asciitoint},
    {"inttoascii", cmd_inttoascii},
    {"mk", cmd_mk},
    {"mkr", cmd_mkr},
    {"rd", cmd_rd},
    {"write", cmd_write},
    {"fsseed", cmd_fsseed},
    {"ls", cmd_ls},
    {"cd", cmd_cd},
    {"cur", cmd_cur},
};

#define COMMAND_COUNT (sizeof(commands) / sizeof(commands[0]))


void shell_init(void)
{
    cwd = vfs_root;
    if (mode == 0)
    {
        vga_print_color("kernel", 0xC);
        vga_print("> ");
    }
    else if (mode == 1)
    {
        vga_print_color("root", 0xE);
        vga_print("> ");
    }
    else if (mode == 2)
    {
        vga_print_color("user", 0x9);
        vga_print("> ");
    }
}

void shell_update(void)
{
    while (keyboard_has_input())
    {
        char c = keyboard_getchar();

        if (c == '\n')
        {
            char* input = (char*)console_get_line();

            vga_putchar('\n');

            execute_command(input);

            console_clear_line();
            if (mode == 0)
            {
                vga_print_color("kernel", 0xC);
                vga_print("> ");
            }
            else if (mode == 1)
            {
                vga_print_color("root", 0xE);
                vga_print("> ");
            }
            else if (mode == 2)
            {
                vga_print_color("user", 0x9);
                vga_print("> ");
            }
        }
        else
        {
            console_handle_char(c);
        }
    }
}


static void execute_command(char* input)
{
    char local_buffer[256];

    strcpy(local_buffer, input);

    char* argv[MAX_ARGS];
    int argc = 0;

    parse_arguments(local_buffer, argv, &argc);

    if (argc == 0)
        return;

    for (size_t i = 0; i < COMMAND_COUNT; i++)
    {
        if (strcmp(argv[0], commands[i].name) == 0)
        {
            commands[i].handler(argc, argv);
            return;
        }
    }

    vga_print("Unknown command\n");
}


static void parse_arguments(char* input, char* argv[], int* argc)
{
    *argc = 0;

    while (*input && *argc < MAX_ARGS)
    {
        // Skip leading spaces
        while (*input == ' ')
            input++;

        if (*input == '\0')
            break;

        argv[*argc] = input;
        (*argc)++;

        // Move until next space
        while (*input && *input != ' ')
            input++;

        if (*input)
        {
            *input = '\0';
            input++;
        }
    }
}
