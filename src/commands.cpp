#include <types.h>
#include <port/port.h>
void printf(uint8_t* a, int flag);
void printfHex(uint8_t key);

#define KEYBOARD_BUFFER_SIZE 128
#define VIDEO_MEMORY_ADDRESS 0xb8000 // Special address in RAM that when written to prints on screen
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define WHITE_ON_BLACK 0x0F
#define MAX_COMMAND_LENGTH 128
#define MAX_COMMANDS 10
#define TIMER_FREQUENCY 18 // Usually this value on x86 

extern uint8_t x,y;
extern uint8_t up_time;
extern uint16_t* VideoMemory;

extern char command_buffer[MAX_COMMAND_LENGTH]; // Printing related - a buffer to store the characters written
extern int command_length;
// A file containing only the functions that are called by user when input
void clear_screen() 
{
    // QEMU prints some text to screen about boot device
    // Before doing any actions, clear screen
    for (int y = 0; y < SCREEN_HEIGHT; y++) 
    {
        for (int x = 0; x < SCREEN_WIDTH; x++) 
        {
            VideoMemory[y * SCREEN_WIDTH + x] = (WHITE_ON_BLACK << 8) | ' ';
        }
    }
    // Reset cursor position
    x = 0;
    y = 0;
    printf((uint8_t*)">", 0);  
}

void printDecimal()
{
    // Define a buffer to store the decimal representation
    uint8_t buffer[6]; // Max 5 digits for a 16-bit value + null terminator
    
    // Initialize variables
    int i = 0;
    uint16_t temp = up_time;
    
    // Handle special case when up_time is 0
    if (temp == 0) {
        buffer[i++] = '0';
    } else {
        // Iterate to extract each digit
        while (temp != 0) {
            // Extract the least significant digit
            uint16_t digit = temp % 10;
            
            // Convert the digit to ASCII and store it in the buffer
            buffer[i++] = digit + '0';
            
            // Move to the next digit
            temp /= 10;
        }
    }
    
    // Add null terminator to the end of the buffer
    buffer[i] = '\0';
    
    // Reverse the buffer
    int len = i;
    for (i = 0; i < len / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[len - i - 1];
        buffer[len - i - 1] = temp;
    }
    
    // Print the decimal representation
    printf(buffer,0);
}

void uptime()
{
    printf((uint8_t*)"the OS is up for: ",0);
    printfHex(up_time / TIMER_FREQUENCY);
    printf((uint8_t*)" seconds \n",0);
    printf((uint8_t*)">",0);
}

void ben_dover()
{
    printf((uint8_t*)"sapoj noice \n",0);
    printf((uint8_t*)">", 0);  
}

void shut_down() // test shut down - when qemu sits on specific port
{
    Port32Bit qemu(0xf4);
    qemu.Write(0x10);
    printf((uint8_t*)"\nShutting down FridkinOS... \n",0);
    //loop_flag = false;
}

void help_command() 
{
    printf((uint8_t*)"hello \n", 0);
    printf((uint8_t*)">", 0); 
}

void unknown_command() 
{
    printf((uint8_t*)"error \n", 0);
    printf((uint8_t*)">", 0);  
}