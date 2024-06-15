// In this file we will handle the hardware interrupts received from components such as keyboard and timer
#include <idt/isr/irq/irq.h>
#include <port/port.h>
#define MAX_COMMAND_LENGTH 128
#define VIDEO_MEMORY_ADDRESS 0xb8000
#define WHITE_ON_BLACK 0x0F
#define SCREEN_WIDTH 80

Port8Bit keyboard_port((uint8_t)0x60);
extern uint64_t up_time; // Extern means we tell the compiler that the variable is defined somwhere else
extern uint8_t x,y; // We define in kernel.cpp
char command_buffer[MAX_COMMAND_LENGTH];
extern int command_length;
extern uint16_t* VideoMemory;
void putchar(char c, int flag);
int shift_pressed = 0;
volatile bool newline_received = false;
int write_state = 0;

uint8_t scancode_to_ascii(uint8_t scancode, int shift_pressed) 
{
    static const char scancode_table[128] = 
    {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',  // 0x00 - 0x0F
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',   // 0x10 - 0x1C
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,       // 0x1D - 0x29
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0,       // 0x2A - 0x35
        ' ', 0  // 0x36 - 0x37 (Space at 0x39)
    };

    static const char scancode_table_shifted[128] = 
    {
        0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',  // 0x00 - 0x0F
        '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',   // 0x10 - 0x1C
        0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,        // 0x1D - 0x29
        '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0,        // 0x2A - 0x35
        ' ', 0  // 0x36 - 0x37 (Space at 0x39)
    };

    if (scancode < 128) {
        if (shift_pressed) {
            return scancode_table_shifted[scancode];
        } else {
            return scancode_table[scancode];
        }
    }
    return 0;
}


void Keyboard(Registers* state) 
{
    // Keyboard ISR
    uint8_t data = keyboard_port.Read();

    if (data == 0x2A || data == 0x36) 
    {

        // Left or Right Shift pressed
        shift_pressed = 1;
    } 
    else if (data == 0xAA || data == 0xB6) 
    {
        // Left or Right Shift released
        shift_pressed = 0;
    } 
    else 
    {
        // Convert scancode to ASCII
        data = scancode_to_ascii(data, shift_pressed);

        if (data == '\b') 
        {
            // Handle backspace
            if (command_length > 0 || write_file_buffer_index > 0) 
            {
                if (x > 1 || (x == 1 && y > 0)) 
                { // Ensure '>' is not deleted
                    if (x == 0 && y > 0) 
                    {
                        // Move to the end of the previous line
                        y--;
                        x = SCREEN_WIDTH - 1;
                    } 
                    else if (x > 0) 
                    {
                        x--;
                    }
                    // Clear the character from the screen and command buffer
                    VideoMemory[y * SCREEN_WIDTH + x] = (WHITE_ON_BLACK << 8) | ' ';
                    if (write_state == 1)
                    {
                        write_file_buffer_index--;
                        write_file_buffer[write_file_buffer_index] = '\0';
                    }

                    else
                    {
                        command_length--;
                        command_buffer[command_length] = '\0';
                    }
                }
            }
        } 
        else 
        {
            if (write_state == 1)
            {
                printf((uint8_t*)"Current data \n",0);
                printf(&data,0);
                // This is the state user is inputting data for the file that is creating
                char buffer[2] = {static_cast<char>(data), '\0'};
                printf((uint8_t*)buffer, 0); // Print character
                write_file_buffer[write_file_buffer_index] = buffer[0];
                write_file_buffer_index++;
                if (data == '\n') newline_received = true;

            }
            
            else
            {
                char buffer[2] = {static_cast<char>(data), '\0'};
                printf((uint8_t*)buffer, 1); // Print character
            }
        }

       
    }
    PIC_sendEOI(state->interrupt - PIC1); // Number of IRQ
}

void Timer(Registers* state)
{
    // The timer interrupt is an interrupt sent by the timer in set intervals
    // There isn't much to implement since we aren't implemeting context switching
    up_time++;
    PIC_sendEOI(state->interrupt - PIC1); // number of irq
}

void Disk(Registers* state)
{
    // This ISR will handle the disk communication
    // To communicate with the disk we first initialize it using port communication
    

    PIC_sendEOI(state->interrupt - PIC2); // finish ISR routine

}

void Populate_Irq_Entries()
{
    // The ISRHandlers array in the isr.cpp contains an array of void pointers to the ISR themselves
    

    // The current structure:
    // When an interrupt is called:
    // 1. It is searched in the idt
    // The specific ISR stub is called from within the idt (each stub defined in assembly file using macro)
    // After pushing some values, the general purpose ISR_Handler is called with the state of teh registers as input (Registers* state)

    // We populate ISRHandlers with the irqs we defined
    ISR_RegisterHandler(PIC1_BASE_IRQ, Timer);
    ISR_RegisterHandler(PIC1_BASE_IRQ + 1, Keyboard);
    ISR_RegisterHandler(PIC2_BASE_IRQ + 7, Disk); // Disk connected to IRQ 14 for (or 15) depends if IDE is primary or secondary
}
