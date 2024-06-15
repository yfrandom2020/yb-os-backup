#pragma once
#include <idt/isr/isr.h>
#include <port/pic.h>

void Keyboard(Registers* state);
void Timer(Registers* state);
void Disk(Registers* state);
void Populate_Irq_Entries();
extern int write_state; // Special variable to determine whether action taken in write to file 
extern uint8_t write_file_buffer[2000];
extern uint16_t write_file_buffer_index;
extern volatile bool newline_received;