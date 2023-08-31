#include "../types.h"

#define START_LOCAL_ADDRESS (1 * SIZE_OF_INT)

#define INT_POINTER(p) (int *)(p)

#define SIZE_OF_LOOP   20
#define SIZE_OF_SWITCH 10
#define SIZE_OF_CASE   100

#define UP_LABEL(pos)   pos=next_code_count()
#define JMP_UP(pos)     generate_code2(JMP,pos)
#define JPT_UP(pos)     generate_code2(JPT,pos)
#define JMP_DOWN(pos)   pos=generate_code2(JMP,0)
#define JPF_DOWN(pos)   pos=generate_code2(JPF,0)
#define DOWN_LABEL(pos) backpatch(pos,next_code_count())

extern void init_type(void);
extern Token next_token(void);
extern void backpatch(int n, int address);
extern void backpatch_calladdress(void);
extern void backpatch_return(int function_address);
extern void backpatch_break(int looptop);
extern char *memory_address(int index);
extern int global_malloc(int size);
extern Symbol *enter(Symbol symbol, SymbolType type);
extern Token check_next_token(Token token, TokenType type);
extern void to_left_value(void);
extern int generate_code3(OperationCode op_code, int flag, int data);
extern void generate_code_unary(TokenType type);
extern void generate_code_binary(TokenType type);
extern Symbol *search(char *token_text);
extern Symbol *search_name(char *symbol_name);
extern bool is_local(Symbol *symbol);
extern void open_local_table(void);
extern void close_local_table(Symbol *symbol);
extern void remove_value(void);
extern bool get_const(int *var);
extern void delete_function_table(Symbol *symbol1, Symbol *symobol2);
extern int next_code_count(void);

int counter_for_nest_block = 0;
int counter_for_error      = 0;

static struct {
  TokenType type;
  int top;
  bool has_break;
} loop_nests[SIZE_OF_LOOP];

static struct {
  int default_address;
  int start_case_list;
} switch_nests[SIZE_OF_SWITCH];

static struct {
  int value;
  int address;
} cases[SIZE_OF_CASE];

static Token token;
static TokenType last_statement;

static Symbol *function_pointer = NULL;
static Symbol tmp_symbol = { "", LITERAL_ID, NON_T, 0, GLOBAL, 0, 0, 0 };

static int local_address;

static int counter_for_loop   = 0;
static int counter_for_switch = 0;
static int counter_for_case   = 0;

static void declare_variable(void);
static void declare_function(void);
static void check_expression(TokenType type1, TokenType type2);
static void check_function(Symbol *symbol1, Symbol *symbol2);
static void declare_function_begin(void);
static void declare_function_end(void);
static void set_main(void);
static void block(bool is_function);
static void statement(void);
static int get_looptop(void);
static void begin_continue_or_break(TokenType type);
static void end_continue_or_break(void);
static void begin_switch(void);
static void end_switch(void);
static void call_function(Symbol *symbol);
static void call_system_function(TokenType type);
static void set_type(void);
static void set_name(void);
static void set_array_size(void);

static void expression(void);
static void term(int n);
static void factor(void);

static int operation_order(TokenType type);
static int local_malloc(int size);
static char *symbol_name_malloc(const char *name);
static void increment(int *var, int size, const char *error_message);
static bool is_global(void);
static bool check_format(char *format);
