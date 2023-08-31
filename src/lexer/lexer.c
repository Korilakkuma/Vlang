#include "lexer.h"

Token next_token(void) {
  Token token = { TOKEN_TYPE_NULL, "", 0 };

  static int ch = ' ';

  bool has_error = false;

  char *p = token.text;

  char *p_id   = p + (sizeof(char) * SIZE_OF_ID);
  char *p_text = p + (sizeof(char) * SIZE_OF_TEXT);

  char tmp[100];
  char *end_of_tmp = tmp + 100;

  while (isspace(ch)) {
    ch = next_char();
  }

  if (ch == EOF) {
    token.type = TOKEN_TYPE_EOF_TOKEN;
    return token;
  }

  switch (types[ch]) {
    case TOKEN_TYPE_LETTER: {
      while ((types[ch] == TOKEN_TYPE_LETTER) || (types[ch] == TOKEN_TYPE_DIGIT)) {
        if (p < p_id) {
          *p++ = ch;
        }

        ch = next_char();
      }

      *p = '\0';
      break;
    }

    case TOKEN_TYPE_DIGIT: {
      int value_as_int = 0;

      for (int value_as_int = 0; types[ch] == TOKEN_TYPE_DIGIT; ch = next_char()) {
        value_as_int = (10 * value_as_int) + (ch - '0');
      }

      token.type = TOKEN_TYPE_NUMBER;
      token.value = value_as_int;

      sprintf(token.text, "%d", value_as_int);
      break;
    }

    case TOKEN_TYPE_SINGLE_QUOTE: {
      int count = 0;

      for (ch = next_char(); ((ch != EOF) && (ch != '\n') && (ch != '\'')); ch = next_char()) {
        if (ch == '\\') {
          if ((ch = next_char()) == 'n') {
            ch = '\n';
          }
        }

        if (++count == 1) {
          token.value = ch;
        }
      }

      if (count != 1) {
        has_error = true;
      }

      if (ch == '\'') {
        ch = next_char();
      } else {
        has_error = true;
      }

      if (has_error) {
        display_single_error("Invalid string literal");
      }

      token.type = TOKEN_TYPE_NUMBER;

      sprintf(token.text, "'%c'", token.value);
      break;
    }

    case TOKEN_TYPE_DOUBLE_QUOTE: {
      int n;

      p = tmp;

      for (ch = next_char(); ((ch != EOF) && (ch != '\n') && (ch != '"')); ch = next_char()) {
        if (has_error) {
          ch = next_char();
          continue;
        }

        if ((n = is_kanji(ch)) > 0) {
          while (n--) {
            if (p < end_of_tmp) {
              *p++ = ch;
              ch = next_char();
            } else {
              has_error = true;
            }
          }

          continue;
        }

        if (p >= p_text) {
          has_error = true;
        } else {
          *p++ = ch;
        }
      }

      *p = '\0';

      if (has_error) {
        display_single_error("Too long string literal");
      }

      if (ch == '"') {
        ch = next_char();
      } else {
        display_single_error("Require double quotation closing");
      }

      token.type  = TOKEN_TYPE_STRING;
      token.value = string_malloc(tmp);

      token.text[0] = '\"';

      strncat((token.text + 1), tmp, 29);

      if (strlen(token.text) <= 29) {
        strcat(token.text, "\"");
      }

      break;
    }

    default: {
      int n;

      if ((ch < 0) || (ch > 127)) {
        // invalid character
        display_error_with_line_number(__FILE__, __LINE__);
      }

      if ((n = is_kanji(ch)) > 0) {
        while (n--) {
          *p++ = ch;
          ch = next_char();
        }
      } else {
        *p++ = ch;
        ch = next_char();
      }

      if (is_ope2(*(p - 1), ch)) {
        *p++ = ch;
        ch = next_char();
      }

      *p = '\0';

      break;
    }
  }

  if (token.type == TOKEN_TYPE_NULL) {
    token = set_token_type(token);
  }

  if (token.type == TOKEN_TYPE_OTHERS) {
    display_error("Invalid token (%s)\n", token.text);
  }

  return token;
}

Token check_next_token(Token token, TokenType type) {
  char error_message[100];

  if (token.type == type) {
    return next_token();
  }

  sprintf(error_message, "not %c before %s ", type, token.text);
  display_single_error(error_message);

  return token;
}

void open_file(char *filename) {
  if ((finput = fopen(filename, "r")) == NULL) {
    display_error_value_with_line_number(__FILE__, __LINE__, filename);
    exit(EXIT_FAILURE);
  }

  line_number = 1;
}

int get_line_number(void) {
  return line_number;
}

void init_type(void) {
  for (int i = 0; i < 256; i++) {
    types[i] = TOKEN_TYPE_OTHERS;
  }

  for (int i = '0'; i <= '9'; i++) {
    types[i] = TOKEN_TYPE_DIGIT;
  }

  for (int i = 'A'; i <= 'Z'; i++) {
    types[i] = TOKEN_TYPE_LETTER;
  }

  for (int i = 'a'; i <= 'z'; i++) {
    types[i] = TOKEN_TYPE_LETTER;
  }

  types['_']  = TOKEN_TYPE_LETTER;
  types['(']  = TOKEN_TYPE_LEFT_PAREN;
  types[')']  = TOKEN_TYPE_RIGHT_PAREN;
  types['{']  = TOKEN_TYPE_LEFT_BRACE;
  types['}']  = TOKEN_TYPE_RIGHT_BRACE;
  types['[']  = TOKEN_TYPE_LEFT_BRACKET;
  types[']']  = TOKEN_TYPE_RIGHT_BRACKET;
  types['<']  = TOKEN_TYPE_LESS;
  types['>']  = TOKEN_TYPE_GREAT;
  types['+']  = TOKEN_TYPE_PLUS;
  types['-']  = TOKEN_TYPE_MINUS;
  types['*']  = TOKEN_TYPE_MULTI;
  types['/']  = TOKEN_TYPE_DIVIDE;
  types['%']  = TOKEN_TYPE_MOD;
  types['!']  = TOKEN_TYPE_NOT;
  types[':']  = TOKEN_TYPE_COLON;
  types[';']  = TOKEN_TYPE_SEMICOLON;
  types['=']  = TOKEN_TYPE_ASSIGN;
  types['#']  = TOKEN_TYPE_NUMBER_SIGN;
  types['\\'] = TOKEN_TYPE_BACKSLASH;
  types[',']  = TOKEN_TYPE_COMMA;
  types['\''] = TOKEN_TYPE_SINGLE_QUOTE;
  types['"']  = TOKEN_TYPE_DOUBLE_QUOTE;
  types['<']  = TOKEN_TYPE_LESS;
  types['>']  = TOKEN_TYPE_GREAT;
}

static int next_char(void) {
  static int ch1 = 1;
  static int quot = 0;

  int ch2;

  if (ch1 == EOF) {
    return ch1;
  }

  if (ch1 == '\n') {
    ++line_number;
  }

  ch1 = fgetc(finput);

  if (ch1 == EOF) {
    fclose(finput);
    return ch1;
  }

  if (quot) {
    if ((ch1 == quot) || (ch1 == '\n')) {
      quot = 0;
    }

    return ch1;
  }

  if ((ch1 == '\'') || (ch1 == '"')) {
    quot = ch1;
  } else if (ch1 == '/') {
    switch (ch2 = fgetc(finput)) {
      case '/': {
        while (((ch1 = fgetc(finput)) != EOF) && (ch1 != '\n')) {
          ;
        }

        return ch1;
      }

      case '*': {
        for (ch1 = 0; (ch2 = fgetc(finput)) != EOF; ch1 = ch2) {
          if (ch2 == '\n') {
            ++line_number;
          }

          if ((ch1 == '*') && (ch2 == '/')) {
            ch1 = ' ';
            return ch1;
          }
        }

        // invalid comment
        display_error_with_line_number(__FILE__, __LINE__);
        exit(EXIT_FAILURE);
      }
    }

    ungetc(ch2, finput);
  } else if (ch1 == '*') {
    if ((ch2 = fgetc(finput)) == '/') {
      display_error_with_line_number(__FILE__, __LINE__);
      exit(EXIT_FAILURE);
    }

    ungetc(ch2, finput);
  }

  return ch1;
}

static Token set_token_type(Token token) {
  token.type = TOKEN_TYPE_OTHERS;

  for (int i = 0; keyword_table[i].type != TOKEN_TYPE_END_LIST; ++i) {
    if (strcmp(token.text, keyword_table[i].text) == 0) {
      token.type = keyword_table[i].type;
      return token;
    }
  }

  int ch = token.text[0];

  if (types[ch] == TOKEN_TYPE_LETTER) {
    token.type = TOKEN_TYPE_IDENTIFIER;
  } else if (types[ch] == TOKEN_TYPE_DIGIT) {
    token.type = TOKEN_TYPE_NUMBER;
  }

  return token;
}

static inline bool is_ope2(const int c1, const int c2) {
  char s[] = "    ";

  s[1] = c1;
  s[2] = c2;

  return (bool)(strstr(" ++ -- <= >= == != && || ", s) != NULL);
}
