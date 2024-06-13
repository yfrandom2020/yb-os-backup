/*--------------------------------------------------------------------------------------------------------------------*/
// This is the main kernel file
// This file is the main code that gets exectuted upon booting the os
// The main function is kernelMain which will in turn call the other functions, such as IDT and PIC related function ()
// Realistically, this os follows a monolythic kernel design so all processes are level 0
/*--------------------------------------------------------------------------------------------------------------------*/
#include <kernel.h>
#include <initializers.h>

uint8_t x = 0;
uint8_t y = 0;
uint8_t up_time = 0;
uint16_t* VideoMemory = (uint16_t*) VIDEO_MEMORY_ADDRESS;
int command_length = 0;
ata ata0m(true, 0x1F0);


// Conventions
typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

extern "C" void kernelMain(void* multiboot_structure, uint32_t magicnumber)
{
    // This is the kernelMain function - the main function of the kernel and the os
    // This function is the first to run and is the one called from the loader.s file
    
    initializers(); // In order: clear, gdt, idt, isr, irq, pic, ata, mbr, vbr


    for (int i = 0; i < 10; i++)
    {
        i--;
        while(1);
    }
}

