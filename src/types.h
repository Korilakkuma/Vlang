#include "includes.h"
#include "macros.h"

typedef struct {
  TokenType type;
  char text[SIZE_OF_TEXT];
  int value;
} Token;

typedef struct {
  char *name;
  SymbolType symbol_type;
  DataType type;
  int array_length;
  Memory memory;
  char argc;
  int address;
  int block_nest;
} Symbol;

extern void display_error(const char *error_message1, const char *error_message2);
extern void display_single_error(const char *error_message);
extern void display_formatted_error(const char *format, int id);
extern void display_error_with_line_number(const char *filename, int line_number);
extern void display_error_value_with_line_number(const char *filename, int line_number, const char *value);
