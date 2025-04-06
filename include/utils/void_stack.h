#pragma once

#define STK_CANARY_PROTECTION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#ifdef STK_CANARY_PROTECTION
    #define STK_ON_CANARY_PROTECTION(...)  __VA_ARGS__
#else
    #define STK_ON_CANARY_PROTECTION(...)
#endif

typedef uint64_t can_type;

enum StkError
{
    STK_NO_ERROR                   = 0,
    STK_ERROR_REALLOC              = 1,
    STK_ERROR_POP                  = 2,
    STK_ERROR_CALLOC               = 3,
    STK_ERROR_NEGATIVE_ELEM_SIZE   = 4,
    STK_ERROR_SECOND_CTOR          = 5,
    STK_ERROR_NULL_PTR_STK         = 6,
    STK_ERROR_NULL_PTR_DATA        = 7,
    STK_ERROR_SIZE_BIGGER_CAPACITY = 8,
    STK_ERROR_NO_POISON            = 9,
    STK_ERROR_POISON               = 10,
    STK_ERROR_LEFT_CANARY          = 11,
    STK_ERROR_RIGHT_CANARY         = 12,
    STK_ERROR_LEFT_STR_CANARY      = 13,
    STK_ERROR_RIGHT_STR_CANARY     = 14,
    STK_ERROR_DATA_HASH            = 15,
    STK_ERROR_STR_HASH             = 16,
    STK_ERROR_NEGATIVE_CAPACITY    = 17
};

struct Stack
{
    STK_ON_CANARY_PROTECTION(can_type canary_left;)

    void* data;
    size_t size;
    size_t capacity;
    size_t elem_size;
    uint64_t stk_hash;
    uint64_t data_hash;

    STK_ON_CANARY_PROTECTION(can_type canary_right;)
};

enum StkError    stack_ctor         (struct Stack *stk, size_t elem_size);
enum StkError    stack_dtor         (struct Stack *stk);
enum StkError    stack_push         (struct Stack *stk, const void* elem);
void*            stack_pop          (struct Stack *stk, enum StkError* error);
void             stk_print_error    (enum StkError error);
void             stack_set_log_file (FILE* file);
enum StkError    stk_resize         (struct Stack* stk, size_t new_capacity);
const char*      stk_get_error      (enum StkError error);
enum StkError    stk_verifier       (struct Stack* stk);
enum StkError    stk_realloc_down   (struct Stack *stk);
enum StkError    stk_realloc_up     (struct Stack* stk);
uint32_t         get_hash           (const uint8_t* key, size_t length);
void             stack_dump         (struct Stack *stk);
void             set_left_canary    (void* new_ptr);
void             set_right_canary   (struct Stack* stk, void* new_ptr, size_t new_capacity);
void             set_data           (struct Stack* stk, void* new_ptr);
