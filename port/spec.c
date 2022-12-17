#include "bdd-for-c.h"

#include <stdio.h>
#include "prelude.c"

/*
typedef int64_t s64;
typedef int32_t s32;
typedef uint8_t u8;
typedef uint64_t u64;
typedef int8_t s8;
typedef uint16_t u16;
*/
#define x64_ret 0xc3


/*
void buffer_append_u8(Buffer* buffer, u8 value)
{
        
        assert(buffer->occupied + sizeof(value) <= buffer->capacity);
        buffer->memory[buffer->occupied] = value;
        buffer->occupied  += sizeof(value);
};

void buffer_append_s32(Buffer* buffer, s32 value)
{
        
        assert(buffer->occupied + sizeof(value) <= buffer->capacity);
        u8* first_non_occupied_address = buffer->memory + buffer->occupied;
        s32* target = (s32*)first_non_occupied_address;
        *target = value;
        buffer->occupied  += sizeof(value);
};
*/

typedef s32 (*constant_s32)();

typedef enum
{
        MOD_Displacement_0 = 0b00,
        MOD_Displacement_s8 = 0b01,
        MOD_Displacement_s32 = 0b10,
        MOD_Register = 0b11,
        
        
}MOD;

typedef enum
{
        REX   = 0b01000000,
        REX_W = 0b01001000, // 0 = Operand size determined by CS.D; 1 = 64 Bit Operand Size
        REX_R = 0b01000100, // Extension of the ModR/M reg field
        REX_X = 0b01000010, // Extension of the SIB index field
        REX_B = 0b01000001, 
}REX_BYTE;

typedef enum
{
        Operand_Type_Register = 1,
}Operand_Type;

typedef struct
{
        u8 index;
        
}Register;

typedef struct
{
        Operand_Type type;
        union
        {
                Register reg;
                s8 imm8;
        };
        
}Operand;

#define define_register(reg_name, reg_index) \
const Operand reg_name = { .type = Operand_Type_Register, .reg = { .index = (reg_index) } };

define_register(rax, 0);
define_register(rcx, 1);
define_register(rdx, 2);
define_register(rbx, 3);
define_register(rsp, 4);
define_register(rbp, 5);
define_register(rsi, 6);
define_register(rdi, 7);

define_register(r8,  8);
define_register(r9,  9);
define_register(r10, 10);
define_register(r11, 11);
define_register(r12, 12);
define_register(r13, 13);
define_register(r14, 14);
define_register(r15, 15);
#undef define_register

/*
const Operand rax = {
        .type = Operand_Type_Register,
        .reg =  {.index = 0b0000},
};
*/


const Operand eax = {
        .type = Operand_Type_Register,
        .reg =  {.index = 0b000},
};

/*
const Operand rdi = {
        .type = Operand_Type_Register,
        .reg =  {.index = 0b0100},
};
*/


const Operand edi = {
        .type = Operand_Type_Register,
        .reg =  {.index = 0b111},
};


typedef enum
{
        mov = 1,
        
}X64_Mnemonic;

typedef struct
{
        X64_Mnemonic mnemonic;
        Operand operands[2];
        
}Instruction;

typedef enum
{
        Instruction_Extension_Type_Register = 1,
        Instruction_Extension_Type_Op_Code,
        Instruction_Extension_Type_Plus_Register,
}Instruction_Extension_Type;

typedef enum
{
        Operand_Encoding_Type_Register = 1,
        Operand_Encoding_Type_Register_Memory,
}Operand_Encoding_Type;

typedef struct
{
        u16 op_code;
        Instruction_Extension_Type extension_type;
        Operand_Encoding_Type operand_encoding_types[2];
        
}Instruction_Encoding;

void encode(Buffer* buffer, Instruction instruction)
{
        Instruction_Encoding encoding  = {
                .op_code = 0x89,
                .extension_type = Instruction_Extension_Type_Register,
                .operand_encoding_types = {
                        Operand_Encoding_Type_Register_Memory,
                        Operand_Encoding_Type_Register,
                },
        };
        buffer_append_u8(buffer, REX_W);
        buffer_append_u8(buffer, (u8)encoding.op_code);
        u8 mod_r_m = {
                (MOD_Register << 6) |
                (instruction.operands[0].reg.index << 3) |
                (instruction.operands[1].reg.index)  
        };
        
        buffer_append_u8(buffer, mod_r_m);
        
}

void
buffer_append_sub_rsp_imm_8(
                            Buffer *buffer,
                            s8 value
                            ) {
        
        buffer_append_u8(buffer, 0x48);
        buffer_append_u8(buffer, 0x83);
        buffer_append_u8(buffer, 0xEC);
        buffer_append_u8(buffer, value);
}

void
buffer_append_add_rsp_imm_8(
                            Buffer *buffer,
                            s8 value
                            ) {
        
        buffer_append_u8(buffer, 0x48);
        buffer_append_u8(buffer, 0x83);
        buffer_append_u8(buffer, 0xc4);
        buffer_append_u8(buffer, value);
}

void
buffer_append_mov_to_stack_offset_imm_32(
                                         Buffer *buffer,
                                         s8 offset,
                                         s32 value
                                         ) {
        
        buffer_append_u8(buffer, 0xc7);
        buffer_append_u8(buffer, 0x44);
        buffer_append_u8(buffer, 0x24);
        buffer_append_u8(buffer, offset);
        buffer_append_s32(buffer, value);
}
void
buffer_append_add_to_ecx_value_at_stack_offset(
                                               Buffer *buffer,
                                               s8 value
                                               ) {
        
        buffer_append_u8(buffer, 0x03);
        buffer_append_u8(buffer, 0x4C);
        buffer_append_u8(buffer, 0x24);
        buffer_append_u8(buffer, value);
}

void
buffer_append_add_to_edi_value_at_stack_offset(
                                               Buffer *buffer,
                                               s8 value
                                               ) {
        //buffer_append_u8(buffer, 0x48);
        buffer_append_u8(buffer, 0x03);
        buffer_append_u8(buffer, 0x7C);
        buffer_append_u8(buffer, 0x24);
        buffer_append_u8(buffer, value);
}

void
buffer_append_mov_to_rcx_from_rax(
                                  Buffer *buffer
                                  ) {
        
        buffer_append_u8(buffer, 0x48);
        buffer_append_u8(buffer, 0x89);
        buffer_append_u8(buffer, 0xc8);
}

void
buffer_append_mov_to_rax_from_edi(
                                  Buffer *buffer
                                  ) {
        
        encode(buffer, (Instruction)  {mov, {edi, rax}});
        //buffer_append_u8(buffer, 0x89);
        //buffer_append_u8(buffer, 0xF8);
}

constant_s32
make_constant_s32(
                  s32 value
                  ) {
        
        Buffer buffer = make_buffer(1024, MAP_PRIVATE | MAP_ANONYMOUS );
        
        buffer_append_u8(&buffer, 0x48);
        buffer_append_u8(&buffer, 0xc7);
        buffer_append_u8(&buffer, 0xc0);
        buffer_append_s32(&buffer, value);
        buffer_append_u8(&buffer, x64_ret);
        
        return (constant_s32)buffer.memory;
}

typedef s64 (*identity_s64)();

identity_s64
make_identity_s64() {
        
        Buffer buffer = make_buffer(1024, MAP_PRIVATE | MAP_ANONYMOUS );
        
        encode(&buffer, (Instruction) {mov, {edi, eax}});
        //buffer_append_u8(&buffer, 0x48);
        //buffer_append_u8(&buffer, 0x89);
        //buffer_append_u8(&buffer, 0xF8);
        buffer_append_u8(&buffer, x64_ret);
        
        return (identity_s64)buffer.memory;
}

typedef s64 (*increment_s64)();

increment_s64
make_increment_s64() {
        
        Buffer buffer = make_buffer(1024, MAP_PRIVATE | MAP_ANONYMOUS );
        buffer_append_sub_rsp_imm_8(&buffer, 24);
        buffer_append_mov_to_stack_offset_imm_32(&buffer, 20, 1);
        buffer_append_add_to_edi_value_at_stack_offset(&buffer, 20);
        
        //buffer_append_mov_to_rax_from_edi(&buffer);
        encode(&buffer, (Instruction) {mov, { edi, eax}});
        buffer_append_add_rsp_imm_8(&buffer, 24);
        buffer_append_u8(&buffer, x64_ret);
        return (increment_s64)buffer.memory;
}

spec("mass") {
        
        it("should create function that will return 42") {
                
                constant_s32 the_answer = make_constant_s32(42);
                s32 result = the_answer();
                check(result == 42);
                check(the_answer() == 42);
        }
        it("should create function that will return 21") {
                
                constant_s32 not_the_answer = make_constant_s32(21);
                s32 result = not_the_answer();
                check(result == 21);
        }
        it("should create function that returns s64 value that was passed") {
                
                identity_s64 id_s64 = make_identity_s64();
                s64 result = id_s64(42);
                
                check(result == 42);
                
        }
        
        it("should create function increments s64 value passed to it") {
                
                identity_s64 inc_s64 = make_increment_s64();
                s64 result = inc_s64(42);
                printf("%ld", result);
                check(result == 43);
        }
        
}
