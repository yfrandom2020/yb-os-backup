#pragma once
#include <types.h>
typedef unsigned int size_t;
int strcmp(uint8_t* str1, uint8_t* str2);
void *memset(void *ptr, int value, size_t num);
uint8_t* check_partial(uint8_t* keyword, uint8_t* data);
void printf(uint8_t* ptr, int flag);
void printfHex16(uint16_t key);
void printfHex32(uint32_t key);
