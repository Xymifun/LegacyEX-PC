#ifndef NN_TYPES_H_
#define NN_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#include <wut_types.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef float f32;
typedef double f64;

#define ALIGNVAR(x) __attribute__((aligned(x)))

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif
