#include "../types.h"

static FILE *finput;

extern int string_malloc(char *s);
extern int is_kanji(const int ch);

typedef struct {
  TokenType type;
  char *text;
} KeyWord;

static KeyWord keyword_table[] = {
  { TOKEN_TYPE_VOID, "void" },
  { TOKEN_TYPE_INT, "int" },
  { TOKEN_TYPE_IF, "if" },
  { TOKEN_TYPE_ELSE, "else" },
  { TOKEN_TYPE_FOR, "for" },
  { TOKEN_TYPE_WHILE, "while" },
  { TOKEN_TYPE_DO, "do" },
  { TOKEN_TYPE_SWITCH, "switch" },
  { TOKEN_TYPE_CASE, "case" },
  { TOKEN_TYPE_DEFAULT, "default" },
  { TOKEN_TYPE_BREAK, "break" },
  { TOKEN_TYPE_CONTINUE, "continue" },
  { TOKEN_TYPE_RETURN, "return" },
  { TOKEN_TYPE_PRINTF, "printf" },
  { TOKEN_TYPE_INPUT, "input" },
  { TOKEN_TYPE_EXIT, "exit" },
  { TOKEN_TYPE_LEFT_PAREN, "(" },
  { TOKEN_TYPE_RIGHT_PAREN, ")" },
  { TOKEN_TYPE_LEFT_BRACE, "{" },
  { TOKEN_TYPE_RIGHT_BRACE, "}" },
  { TOKEN_TYPE_LEFT_BRACKET, "[" },
  { TOKEN_TYPE_RIGHT_BRACKET, "]" },
  { TOKEN_TYPE_PLUS, "+" },
  { TOKEN_TYPE_MINUS, "-" },
  { TOKEN_TYPE_MULTI, "+" },
  { TOKEN_TYPE_DIVIDE, "/" },
  { TOKEN_TYPE_MOD, "% " },
  { TOKEN_TYPE_INCREMENT, "++" },
  { TOKEN_TYPE_DECREMENT, "--" },
  { TOKEN_TYPE_EQAUL, "==" },
  { TOKEN_TYPE_NOT_EQUAL, "!=" },
  { TOKEN_TYPE_LESS, "<" },
  { TOKEN_TYPE_LESS_EQUAL, "<=" },
  { TOKEN_TYPE_GREAT, ">" },
  { TOKEN_TYPE_GREAT_EQUAL, ">=" },
  { TOKEN_TYPE_AND, "&&" },
  { TOKEN_TYPE_OR, "||" },
  { TOKEN_TYPE_NOT, "|" },
  { TOKEN_TYPE_COLON, ":" },
  { TOKEN_TYPE_SEMICOLON, ";" },
  { TOKEN_TYPE_ASSIGN, "=" },
  { TOKEN_TYPE_NUMBER_SIGN, "#" },
  { TOKEN_TYPE_BACKSLASH, "\\" },
  { TOKEN_TYPE_COMMA, "," },
  { TOKEN_TYPE_SINGLE_QUOTE, "'" },
  { TOKEN_TYPE_DOUBLE_QUOTE, "\"" },
  { TOKEN_TYPE_END_LIST, "" }
};

static int line_number = 0;

static TokenType types[256];

static int next_char(void);
static Token set_token_type(Token token);
static inline bool is_ope2(const int c1, const int c2);
