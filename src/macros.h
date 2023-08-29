// macros for memory limit
#define SIZE_OF_ID         30
#define SIZE_OF_TEXT       100
#define SIZE_OF_MAX_TOKENS 32767

#define SIZE_OF_INT sizeof(int)

// for back-patching
#define NO_BREAK_ADDRESS  -101
#define NO_RETURN_ADDRESS -102

// macros for function
#define is_main(p) (strncmp((p)->name,"main",4)==0)

#define generate_code1(op) generate_code3(op,0,0)
#define generate_code2(op, data) generate_code3(op,0,data)

#define LOCAL_SCOPE 1

typedef enum {
  NON_T,
  VOID_T,
  INT_T
} DataType;

typedef enum {
  NOP,
  INC,
  DEC,
  NEG,
  NOT,
  ADD,
  SUB,
  MUL,
  DIV,
  MOD,
  LESS,
  LSEQ,
  GRT,
  GTEQ,
  EQCMP,
  NTEQ,
  AND,
  OR,
  CALL,
  DEL,
  JMP,
  JPT,
  JPF,
  EQ,
  LD,
  LDA,
  LDI,
  ST,
  ADBR,
  RET,
  ASS,
  ASSV,
  VAL,
  LIB,
  STOP
} OperationCode;

enum {
  FUNCTION_EXIT = 1,
  FUNCTION_INPUT,
  FUNCTION_PRINTF,
  FUNCTION_PRINTF_2
};

typedef enum {
  TOKEN_TYPE_LEFT_PAREN = '(',
  TOKEN_TYPE_RIGHT_PAREN = ')',
  TOKEN_TYPE_LEFT_BRACE = '{',
  TOKEN_TYPE_RIGHT_BRACE = '}',
  TOKEN_TYPE_LEFT_BRACKET = '[',
  TOKEN_TYPE_RIGHT_BRACKET = ']',
  TOKEN_TYPE_PLUS = '+',
  TOKEN_TYPE_MINUS = '-',
  TOKEN_TYPE_MULTI = '*',
  TOKEN_TYPE_DIVIDE = '/',
  TOKEN_TYPE_MOD = '%',
  TOKEN_TYPE_NOT = '!',
  TOKEN_TYPE_COLON = ':',
  TOKEN_TYPE_SEMICOLON = ';',
  TOKEN_TYPE_ASSIGN = '=',
  TOKEN_TYPE_NUMBER_SIGN = '#',
  TOKEN_TYPE_BACKSLASH = '\\',
  TOKEN_TYPE_COMMA = ',',
  TOKEN_TYPE_SINGLE_QUOTE = '\'',
  TOKEN_TYPE_DOUBLE_QUOTE = '"',
  TOKEN_TYPE_VOID = 150,
  TOKEN_TYPE_INT,
  TOKEN_TYPE_IF,
  TOKEN_TYPE_ELSE,
  TOKEN_TYPE_FOR,
  TOKEN_TYPE_WHILE,
  TOKEN_TYPE_DO,
  TOKEN_TYPE_SWITCH,
  TOKEN_TYPE_CASE,
  TOKEN_TYPE_DEFAULT,
  TOKEN_TYPE_BREAK, // 160
  TOKEN_TYPE_CONTINUE,
  TOKEN_TYPE_RETURN,
  TOKEN_TYPE_PRINTF,
  TOKEN_TYPE_INPUT,
  TOKEN_TYPE_EXIT,
  TOKEN_TYPE_INCREMENT,
  TOKEN_TYPE_DECREMENT,
  TOKEN_TYPE_EQAUL,
  TOKEN_TYPE_NOT_EQUAL,
  TOKEN_TYPE_LESS, // 170
  TOKEN_TYPE_LESS_EQUAL,
  TOKEN_TYPE_GREAT,
  TOKEN_TYPE_GREAT_EQUAL,
  TOKEN_TYPE_AND,
  TOKEN_TYPE_OR,
  TOKEN_TYPE_END_LIST,
  TOKEN_TYPE_IDENTIFIER,
  TOKEN_TYPE_NUMBER,
  TOKEN_TYPE_STRING,
  TOKEN_TYPE_NULL, // 180
  TOKEN_TYPE_LETTER,
  TOKEN_TYPE_DIGIT,
  TOKEN_TYPE_EOF_TOKEN,
  TOKEN_TYPE_OTHERS
} TokenType;

typedef enum {
  LITERAL_ID,
  VARIABLE_ID,
  FUNCTION_ID,
  PROTOTYPE_ID,
  ARGUMENT_ID
} SymbolType;

/**
 * Text Segment
 * -------------
 * Data Segment
 * - PPDA (Per Process Data Area)
 * - Data Area
 *  - Static Area
 *  - Heap Area
 * - Stack Area
 */
typedef enum {
  GLOBAL, // Data Area
  LOCAL   // Stack Area
} Memory;
