#include "../types.h"

#define SIZE_OF_CODE 20000

#define STACK_TOP     100
#define STACK_BOTTOM  0

#define SIZE_OF_MEMORY 0xffff

// #define MEMORY_AS_INT(n)      (*(int *)(memories+n))
#define MEMORY_AS_INT(n)      (*(volatile int *)(uintptr_t)(memories+n))
#define IS_ZERO()             if(stack[stack_pointer - 1]==0)runtime_error("0 division error")
#define UNARY_OPERATOR(op)    stack[stack_pointer - 1] = op stack[stack_pointer - 1]
#define BINARY_OPERATOR(op)   stack[stack_pointer - 2] = stack[stack_pointer - 2] op stack[stack_pointer - 1], --stack_pointer
#define INC(data)             MEMORY_AS_INT(stack[stack_pointer - 1])+=(data),stack[stack_pointer - 1]=MEMORY_AS_INT(stack[stack_pointer - 1])
#define ASSIGN(address, data) MEMORY_AS_INT(address)=data

#define PUSH(data) stack[stack_pointer++] = data
#define POP()      stack[--stack_pointer]

extern Symbol *get_symbol_table_pointer(int index);

typedef struct {
  unsigned char opcode;
  unsigned char flag;
  int opdata;
} Instruction;

static Instruction code_table[SIZE_OF_CODE];

static int code_table_pointer = -1;

static int stack[STACK_TOP];
static int stack_pointer = STACK_BOTTOM;

static char memories[SIZE_OF_MEMORY];

static int global_address = 1 * SIZE_OF_INT;

static int program_counter;
static int base_register;

static void call_library(int function_type);
static bool const_fold(OperationCode opcode);
static bool is_binary_opcode(OperationCode opcode);
static int binary_expression(OperationCode opcode, int data1, int data2);
static void runtime_error(char *error_message);
static bool is_code(int code_table_index, OperationCode opcode, int data);

// #ifdef DEBUG_CODE
static char *opcodes[] = {
  "NOP",
  "INC",
  "DEC",
  "NEG",
  "NOT",
  "ADD",
  "SUB",
  "MUL",
  "DIV",
  "MOD",
  "LESS",
  "LSEQ",
  "GRT",
  "GTEQ",
  "EQ",
  "NTEQ",
  "AND",
  "OR",
  "CALL",
  "DEL",
  "JMP",
  "JPT",
  "JPF",
  "EQCMP",
  "LD",
  "LDA",
  "LDI",
  "ST",
  "ADBR",
  "RET",
  "ASS",
  "ASSV",
  "VAL",
  "LIB",
  "STOP"
};
// #endif
