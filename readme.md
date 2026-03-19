# zynthexos

zynthexos is a 32-bit x86 hobby operating system built from scratch in C and Assembly.  
It is designed for educational purposes and low-level systems experimentation.

The project targets the i386 architecture and boots via Multiboot using GRUB.

---

## Features

### Boot & Initialization
- Multiboot-compliant boot process
- Custom linker script
- Assembly entry point
- Kernel stack setup

### Interrupt Handling
- Full IDT implementation (256 entries)
- CPU exception handling (0–31)
- Hardware IRQ handling (32–47)
- PIC remapping
- Programmable Interrupt Timer (PIT) at 100 Hz

### Memory Management

#### Physical Memory Manager (PMM)
- Bitmap-based frame allocator
- Multiboot memory map parsing
- Kernel and bitmap region reservation
- Frame allocation and freeing

#### Paging
- 4KB paging
- Identity-mapped first 4MB
- On-demand page table allocation
- Page mapping function
- TLB invalidation

#### Heap Allocator
- Dynamic kernel heap
- Block splitting
- Block coalescing
- Double-free detection
- Corruption detection using magic values
- Heap validation and dump utilities

### Drivers
- VGA text mode driver (0xB8000)
- Keyboard driver (scancode → ASCII)
- PIC driver
- PIT driver
- Port I/O abstraction (inb/outb/outw)

### Debugging
- Kernel panic screen
- CPU segment register inspection
- CR2 and CR3 register inspection
- Page fault diagnostics

### Shell
- Interactive kernel shell
- Command parsing with arguments
- Built-in system and memory tools

Available commands include:

- `help`
- `panic`
- `state`
- `cpuid`
- `uptime`
- `meminfo`
- `heapinfo`
- `heapval`
- `alloc <size>`
- `free <addr>`
- `freelast`
- `memdump <addr> <bytes>`
- `sysfetch`
- `shutdown` (QEMU)
- `reboot` (QEMU)
- `clear`

---

## Build

Just go to project root and run ./run.sh

---

## Requirments

quemu (or any virtual machine but then build directly with make), gcc, nasm