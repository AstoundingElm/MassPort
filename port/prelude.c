#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/mman.h>
#include <stdlib.h>
typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct {
        u8 *memory;
        u64 occupied;
        u64 capacity;
} Buffer;


Buffer make_buffer(u64 capacity, s32 permission_flags)
{
        
        u8* memory = mmap(0, capacity, PROT_READ | PROT_WRITE | PROT_EXEC,  permission_flags, 0,0);
        return (const Buffer)
        {
                .memory = memory,
                
                .capacity = capacity,
                .occupied = 0,
        };
        
}

#define define_buffer_append(Type) \
void \
buffer_append_##Type( \
Buffer *buffer, \
Type value \
) { \
assert(buffer->occupied + sizeof(Type) <= buffer->capacity); \
Type *target = (Type *)(buffer->memory + buffer->occupied); \
*target = value; \
buffer->occupied += sizeof(Type); \
}

define_buffer_append(s8)
define_buffer_append(s16)
define_buffer_append(s32)
define_buffer_append(s64)

define_buffer_append(u8)
define_buffer_append(u16)
define_buffer_append(u32)
define_buffer_append(u64)
#undef define_buffer_append