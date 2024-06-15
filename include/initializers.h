#pragma once 
#include <fat16/disk.h>
#include <fat16/fat16.h>
#include <util/util.h>
void clear_screen();
void initializers();
void GDT_Initialize();
extern "C" void IDT_Initialize();
extern "C" void ISR_Initialize();
void Populate_Irq_Entries();
extern "C" void Init_pic();
extern "C" void Enable_interrupts();
void ata_initialize();
void Init_ext();
extern ata ata0m;
extern uint8_t* up_message;
void test_disk();
void Write_File(uint8_t* name, uint8_t* data, uint32_t data_size);
void printfHex16(uint16_t key);
void initialize_write_buffer();