// Kernel header
// The kernel header will include all the other header files
// Therefore, when creating header files of new tasks it is only needed to include the kernel.h file
#pragma once
#include <types.h>
#include "gdt/gdt.h"
#include "port/port.h"
#include "port/pic.h"
#include "idt/idt.h"
#include "idt/util.h"
#include "idt/isr/isr.h"
#include "idt/isr/isrgen.h"
#include "fat16/disk.h"
#include "fat16/fat16.h"
#include <util/util.h>


#define KEYBOARD_BUFFER_SIZE 128
#define VIDEO_MEMORY_ADDRESS 0xb8000 // Special address in RAM that when written to prints on screen
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define WHITE_ON_BLACK 0x0F
#define MAX_COMMAND_LENGTH 128
#define MAX_COMMANDS 10

uint8_t* up_message = (uint8_t*)"Welcome to Fridkin OS! \nIf this is your first time using this project feel free to use the help command to learn about the operating system functionality! \n";
uint8_t* help = (uint8_t*)"Fridkin os is an operating system project made by Yonathan Fridkin as YB project \nBelow is a list of useful commands:\n1.clear - to clean the screen completely";

extern uint8_t up_time;
typedef void (*command_func_t)(void); // Pointer to a void function that takes no arguments

extern uint16_t* VideoMemory;
extern uint8_t x,y;

extern uint8_t command_buffer[MAX_COMMAND_LENGTH]; // Printing related - a buffer to store the characters written
extern int command_length; // Indexer - point to the available element in buffer length

bool loop_flag = true;

extern ata ata0m; // ATA PIO disk interface with the virtual hard drive created in run.sh



void clear_screen();
void help_command();
void printDecimal();
void unknown_command();
void ben_dover();
void shut_down();
void uptime(); // Declare commands - implemnted in kernel.cpp
void ls();
void Read(uint8_t* file_name);

void printf(uint8_t* ptr, int flag);
void printfHex16(uint16_t key);
void printfHex32(uint32_t key);

typedef struct 
{
    // This struct describes the configuration for a single command recognized by the kernel
    // Each instruction has a corresponding function that is called by the execute_command 
    uint8_t *name;
    command_func_t func;
} __attribute__((packed)) command_t;

command_t all_commands[MAX_COMMANDS] = // List of all available commands
{
    {(uint8_t*)"clear", clear_screen},
    {(uint8_t*)"hello", help_command},
    {(uint8_t*)"ben dover", ben_dover},
    {(uint8_t*)"shut down", shut_down},
    {(uint8_t*)"up time", printDecimal},
    {(uint8_t*)"ls",ls},
    {(uint8_t*)"unknown", unknown_command}
};

void execute_command() // Called by putchar in case of \n from user. Go over the string stored in command buffer and figure out if it's valid command
{
    command_buffer[command_length] = '\0'; // Add a terminator at the end
    int found = 0;



    for (int i = 0; i < MAX_COMMANDS; i++) 
    {
        if (strcmp(command_buffer, all_commands[i].name) == 0) 
        {
            all_commands[i].func();
            found = 1;
            command_length = 0;
            break;
        }
    }

    if (check_partial((uint8_t*)"read", (uint8_t*)command_buffer))
    {
        found = 1;
        Read(check_partial((uint8_t*)"read", (uint8_t*)command_buffer));
    }

    if (!found) unknown_command();

    // Clear the command buffer
    command_length = 0;
}

void scroll() 
{
    // Instead of clearing the entire terminal we shift everything one line down
    for (int i = 0; i < SCREEN_HEIGHT - 1; i++) {
        for (int j = 0; j < SCREEN_WIDTH; j++) {
            VideoMemory[i * SCREEN_WIDTH + j] = VideoMemory[(i + 1) * SCREEN_WIDTH + j];
        }
    }
    for (int j = 0; j < SCREEN_WIDTH; j++) {
        VideoMemory[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + j] = (WHITE_ON_BLACK << 8) | ' ';
    }
    y = SCREEN_HEIGHT - 1;
    x = 0;
}


void putchar(char c, int flag) 
{
    switch (c) 
    {
        case '\n':
            x = 0;
            y++;
            if (flag == 1) execute_command();
            if (y >= SCREEN_HEIGHT) 
            {
                scroll();
            }
            break;
        default:
            if (flag == 1) 
            {
                command_buffer[command_length] = c;
                command_length++;
            }
            VideoMemory[y * SCREEN_WIDTH + x] = (WHITE_ON_BLACK << 8) | c;
            x++;
            break;
    }

    if (x >= SCREEN_WIDTH) // Move to the next line if needed
    {
        x = 0;
        y++;
    }
    if (y >= SCREEN_HEIGHT) 
    {
        scroll();
    }
}

// Basic printf function to print a string
void printf(uint8_t* str, int flag) 
{
    // Flag is a general parameter that indicates if call is from user or from kernel
    // If from user (1) we activate the keyboard buffer and invoke the commands list
    // Else pass
    // Future - add support of variables %d, %s
    for (int i = 0; str[i] != '\0'; i++) 
    {
        putchar(str[i], flag);
    }
}


void printfHex(uint8_t key)
{
    uint8_t* foo = (uint8_t*)"00";
    uint8_t* hex = (uint8_t*)"0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo,0);
}


void printfHex16(uint16_t key)
{
    printfHex((key >> 8) & 0xFF);
    printf((uint8_t*)"\n",0);

    printfHex(key  & 0xFF);
    printf((uint8_t*)"\n",0);

}


void printfHex32(uint32_t key) 
{
    printfHex((key >> 24) & 0xFF); // Print the most significant byte
    printf((uint8_t*)"\n",0);
    printfHex((key >> 16) & 0xFF); // Print the second most significant byte
    printf((uint8_t*)"\n",0);
    printfHex((key >> 8) & 0xFF); // Print the second least significant byte
    printf((uint8_t*)"\n",0);
    printfHex(key & 0xFF);
    printf((uint8_t*)"\n",0); 
    // Print the least significant byte
}

