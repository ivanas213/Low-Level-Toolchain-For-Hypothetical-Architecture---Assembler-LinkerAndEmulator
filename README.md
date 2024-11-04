# Low level toolchain for hypothetical architecture-Assembler, Linker and Emulator

The goal of this project is to create a small yet fully functional operating system core that supports time-shared threading.

Within this project, a memory allocator and thread management are implemented. The core provides the concept of threads, semaphores, and support for time-sharing, along with asynchronous context switching and preemption on timer and keyboard interrupts. The core is implemented as a "library" so that both the user program (application) and the core share the same address space, representing a statically linked single program preloaded into the computer’s main memory. Concurrent processes created within the application are threads executed within that program. This configuration is typical for embedded systems, which do not execute arbitrary programs loaded and run at the user's request but instead execute only the preinstalled program (along with the operating system) embedded in the target hardware. The core is implemented for the RISC-V processor architecture and a school computer with this processor. The implemented core will run in a virtual environment—an emulator for the RISC-V processor.