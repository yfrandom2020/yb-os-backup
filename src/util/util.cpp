#include <util/util.h>
int strcmp(uint8_t* str1, uint8_t* str2) 
{
    // Util - Compare between two strings and return wether they are equal or not    
    while (*str1 && (*str1 == *str2)) 
    {
        str1++;
        str2++;
    }
    return *(uint8_t *)str1 - *(uint8_t*)str2; // 0 if equal
}



void *memset(void *ptr, int value, size_t num) 
{
    // Fill memory with values
    // In: 1) pointer to address 2) value to put 3) how many bytes
    unsigned char *p = (unsigned char *)ptr;
    unsigned char v = (unsigned char)value;

    // Fill given address with given value
    for (size_t i = 0; i < num; i++) 
    {
        *p++ = v;
    }

    return ptr;
}

uint8_t* check_partial(uint8_t* keyword, uint8_t* data)
{
    // Util function - check if data (input from user) contains a specific keyword such as "read" or "write"
    int i = 0;
    for (i = 0; keyword[i] != '\0'; i++)
    {
        if (keyword[i] != data[i]) return (uint8_t*)0;
    }
    return (uint8_t*)&data[i+1];
}
