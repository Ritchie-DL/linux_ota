#ifndef __MY_TYPES_H
#define __MY_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if defined (__cplusplus)
extern "C" {
#endif

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

typedef unsigned int uint;
typedef volatile unsigned int vuint;
typedef void* (*pthread_fun)(void*);

typedef char HI_CHAR;
typedef uint32_t HI_U32;
typedef int HI_S32;
typedef uint8_t HI_U8;
typedef unsigned long HI_UL;
typedef void HI_VOID;
typedef bool HI_BOOL;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#ifndef MIN
#define MIN(a,b)            (((a)<(b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b)            (((a)>(b)) ? (a) : (b))
#endif

#ifndef FALSE
	#define	FALSE 				(0)
#endif

#ifndef TRUE
	#define	TRUE 				(1)
#endif

#ifndef SUCCESS
	#define SUCCESS             (0)
#endif

#ifndef FAILURE
	#define FAILURE             (-1)
#endif

#ifndef HI_FALSE
#define HI_FALSE                FALSE
#endif

#ifndef HI_SUCCESS
#define HI_SUCCESS              SUCCESS
#endif

#ifndef HI_FAILURE
#define HI_FAILURE              FAILURE
#endif

/* Thread error codes */
#ifndef THREAD_SUCCESS
#define THREAD_SUCCESS          (void *)0
#endif

#ifndef THREAD_FAILURE
#define THREAD_FAILURE          (void *)-1
#endif

enum {
    RESET = 0,
    SET = 1
};

enum {
    OFF = 0,
    ON = 1
};

enum{
    LOW = 0,
    HIGH = 1
};

typedef enum {
    PIN0 = 0x01,
    PIN1 = 0x02,
    PIN2 = 0x04,
    PIN3 = 0x08,
    PIN4 = 0x10,
    PIN5 = 0x20,
    PIN6 = 0x40,
    PIN7 = 0x80
} pins_bit_t;

enum {
    BIT0 = 0x01,
    BIT1 = 0x02,
    BIT2 = 0x04,
    BIT3 = 0x08,
    BIT4 = 0x10,
    BIT5 = 0x20,
    BIT6 = 0x40,
    BIT7 = 0x80
} ;

typedef struct {
    uint8_t cmd;
	uint32_t length;
	uint8_t *data;
} blob_data_t;

#if defined (__cplusplus)
}
#endif

#endif

