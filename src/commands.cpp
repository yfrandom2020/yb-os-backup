#include <types.h>
#include <port/port.h>
#include <fat16/fat16.h>
#include <idt/isr/irq/irq.h>
// typedef struct
// {
//     // A useful struct that is used to represnt the values of the registers when pushed into the stack
//     // The general ISR_Handler receives a struct of this type and using the values inside it, it deciphers which ISR was called

//     uint32_t ds;                                            // data segment pushed by us
//     uint32_t edi, esi, ebp, useless, ebx, edx, ecx, eax;    // pusha
//     uint32_t interrupt, error;                              // we push interrupt, error is pushed automatically (or dummy)
//     uint32_t eip, cs, eflags, esp, ss;                      // pushed automatically by CPU
// } __attribute__((packed)) Registers;

Registers empty_struct = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
void printf(uint8_t* a, int flag);
void printfHex(uint8_t key);
void List_Entries(); // POC of read file function
void Read_File(uint8_t* name);
void Write_File(uint8_t* name, uint8_t* data, uint32_t data_size);
void initialize_write_buffer();
extern "C" void Enable_interrupts();
void Keyboard(Registers* state);


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

extern uint8_t* help;


// These variables are extern and are defined in kernel.h
// extern uint8_t write_file_buffer[2000];
// extern uint16_t write_file_buffer_index;
// extern bool newline_received;
// extern int write_state;


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

void helper()
{
    //printf((uint8_t*)"Write operation successful! \n",0);
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
    printf(help, 0);
    printf((uint8_t*)">", 0); 
}

void unknown_command() 
{
    printf((uint8_t*)"Error: command not recognized \n", 0);
    printf((uint8_t*)">", 0);  
}

void ls()
{
    // List entries
    List_Entries();
    printf((uint8_t*)"\n",0);
    printf((uint8_t*)">",0);
}

void Read(uint8_t* file_name)
{
    helper_entry_struct* filenames = file_names();
    int flag = 0; // Starting at false value, when match detected we set to true
    for (int i = 0; i < 16; i++)
    {
        if (strcmp(file_name, filenames[i].name) == 0)
        {
            flag = 1;
        }
    }

    if (flag == 0)
    {
        printf((uint8_t*)"Error: File not found \n",0);
        printf((uint8_t*)">",0);
        return;
    }
        
    printf((uint8_t*)"printing file data: \n", 0);
    Read_File(file_name);
    printf((uint8_t*)">",0);
}

void Write(uint8_t* file_name)
{
    // User has used the write command
    // Activate global variable
    initialize_write_buffer();
    write_state = 1;
    newline_received = false;
    printf((uint8_t*)"Please enter file data: \n",0);
    while (1)
    {
        // newline_received turns to true inside the Keyboard function when the user presses enter in write_state mode
        Keyboard(&empty_struct);
        if (newline_received) break;
    }
    // Now the user has used the enter key
    write_state = 0;
    newline_received = false;


    uint8_t sub_array[2000] = { (uint8_t)'\0' };
    int count = 0;
    for (int i = 0; i < 2000; i++)
    {
        if (write_file_buffer[i] != 0x0)
        {
            //if (write_file_buffer[i] == 0x0A) break;
            sub_array[count] = write_file_buffer[i];
            count++;
        }
    }

    Write_File(file_name,sub_array, count);
    initialize_write_buffer();
    helper();
}

