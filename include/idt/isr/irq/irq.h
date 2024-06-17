#pragma once
#include <idt/isr/isr.h>
#include <port/pic.h>

void Keyboard(Registers* state);
void Timer(Registers* state);
void Disk(Registers* state);
void Populate_Irq_Entries();
void HandleKeyboardData(uint8_t data);
void printfHex16(uint16_t key);
void printfHex(uint8_t key);

// These variables are extern and are defined in kernel.h
extern int write_state; // Special variable to determine whether action taken in write to file 
extern uint8_t write_file_buffer[2000];
extern uint16_t write_file_buffer_index;
extern bool newline_received;


// uint8_t write_file_buffer[2000];
// uint16_t write_file_buffer_index;
// bool newline_received = false;
// int write_state = 0;

void initialize_write_buffer();