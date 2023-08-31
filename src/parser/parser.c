#include "parser.h"

bool compile(void) {
  init_type();
  generate_code2(CALL, -1);
  generate_code1(STOP);

  token = next_token();

  while (token.type != TOKEN_TYPE_EOF_TOKEN) {
    switch (token.type) {
      case TOKEN_TYPE_INT:
      case TOKEN_TYPE_VOID: {
        set_type();
        set_name();

        if (token.type == '(') {
          declare_function();
        } else {
          declare_variable();
        }

        break;
      }

      case TOKEN_TYPE_SEMICOLON: {
        token = next_token();
        break;
      }

      default: {
        display_error("Syntax error\n", token.text);
        token = next_token();
        break;
      }
    }
  }

  if (counter_for_error == 0) {
    backpatch_calladdress();
  }

  *INT_POINTER(memory_address(0)) = global_malloc(0);

  if (counter_for_error > 0) {
    fprintf(stderr, "%d errors occurred\n", counter_for_error);
  }

  return (bool)(counter_for_error == 0);
}

void set_address(Symbol *symbol) {
  int size = SIZE_OF_INT;

  switch (symbol->type) {
    case VARIABLE_ID: {
      if (symbol->array_length > 0) {
        size = SIZE_OF_INT * symbol->array_length;
      }

      if (is_global()) {
        symbol->address = global_malloc(size);
      } else {
        symbol->address = local_malloc(size);
      }

      break;
    }

    case FUNCTION_ID: {
      symbol->address = next_code_count();

      for (int i = 0; i <= symbol->argc; ++i) {
        (symbol + i)->address = local_malloc(size);
      }

      break;
    }

    default: {
      break;
    }
  }
}

static void declare_variable(void) {
  while (true) {
    set_array_size();
    enter(tmp_symbol, VARIABLE_ID);

    if (token.type != ',') {
      break;
    }

    token = next_token();

    set_name();
  }

  token = check_next_token(token, ';');
}

static void declare_function(void) {
  Symbol *func;

  local_address = START_LOCAL_ADDRESS;

  func = search_name(tmp_symbol.name);

  if ((func != NULL) && (func->symbol_type != FUNCTION_ID) && (func->symbol_type != PROTOTYPE_ID)) {
    // duplicate identifier
    display_error_value_with_line_number(__FILE__, __LINE__, func->name);
    func = NULL;
  }

  function_pointer = enter(tmp_symbol, FUNCTION_ID);
  token = next_token();
  open_local_table();

  switch (token.type) {
    case TOKEN_TYPE_VOID: {
      token = next_token();
      break;
    }

    case ')': {
      break;
    }

    default: {
      while (true) {
        set_type();
        set_name();
        enter(tmp_symbol, PROTOTYPE_ID);
        ++(function_pointer->argc);

        if (token.type != ',') {
          break;
        }

        token = next_token();
      }

      break;
    }
  }

  token = check_next_token(token, ')');

  if (token.type == ';') {
    function_pointer->symbol_type = PROTOTYPE_ID;
  }

  set_address(function_pointer);

  if (func != NULL) {
    check_function(func, function_pointer);
  }

  switch (token.type) {
    case ';': {
      token = next_token();
      break;
    }

    case '{': {
      if (is_main(function_pointer)) {
        set_main();
      }

      declare_function_begin();
      block(1);
      declare_function_end();
      break;
    }

    default: {
      // function have not ';' or '{', '}'
      display_error_with_line_number(__FILE__, __LINE__);
      exit(EXIT_FAILURE);
      break;
    }
  }

  close_local_table(function_pointer);

  delete_function_table(func, function_pointer);

  function_pointer = NULL;
}

static void check_function(Symbol *symbol1, Symbol *symbol2) {
  if ((symbol1->symbol_type == FUNCTION_ID) && (symbol2->symbol_type == FUNCTION_ID)) {
    // re define
    display_error_value_with_line_number(__FILE__, __LINE__, symbol1->name);
    return;
  }

  if ((symbol1->type != symbol2->type) || (symbol1->argc != symbol2->argc)) {
    // mismatch prototype
    display_error_value_with_line_number(__FILE__, __LINE__, symbol1->name);
    return;
  }
}

static void declare_function_begin(void) {
  generate_code2(ADBR, 0);
  generate_code3(ST, LOCAL_SCOPE, 0);

  for (int i = function_pointer->argc; i >= 1; --i) {
    generate_code3(ST, LOCAL_SCOPE, (function_pointer + i)->address);
  }
}

static void declare_function_end(void) {
  backpatch(function_pointer->address, -local_address);

  if (last_statement != TOKEN_TYPE_RETURN) {
    if (is_main(function_pointer)) {
      generate_code2(LDI, 0);
    } else if (function_pointer->type != VOID_T){
      // require `return`
      display_error_with_line_number(__FILE__, __LINE__);
    }
  }

  backpatch_return(function_pointer->address);
  generate_code3(LD, LOCAL_SCOPE, 0);
  generate_code2(ADBR, local_address);
  generate_code1(RET);
}

static void set_main(void) {
  if ((function_pointer->type != INT_T) || (function_pointer->argc != 0)) {
    // invalid main function type
    display_error_with_line_number(__FILE__, __LINE__);
  }

  backpatch(0, function_pointer->address);
}

static void block(bool is_function) {
  TokenType type = TOKEN_TYPE_OTHERS;

  token = next_token();

  ++counter_for_nest_block;;

  if (is_function) {
    while (token.type == TOKEN_TYPE_INT) {
      set_type();
      set_name();
      declare_variable();
    }
  }

  while (token.type != '}') {
    type = token.type;
    statement();
  }

  last_statement = type;

  --counter_for_nest_block;

  token = next_token();
}

static void statement(void) {
  TokenType type;
  DataType return_type = function_pointer->type;

  int val;

  int LB_TOP;
  int LB_EXP2;
  int LB_EXP3;
  int LB_BODY;
  int LB_ELSE;
  int LB_END;
  int LB_TBL;

  type = token.type;

  if ((type == TOKEN_TYPE_WHILE) || (type == TOKEN_TYPE_DO) || (type == TOKEN_TYPE_SWITCH)) {
    begin_continue_or_break(type);
  }

  switch (type) {
    case TOKEN_TYPE_BREAK: {
      if (counter_for_loop == 0) {
        // invalid `break`
        display_error_with_line_number(__FILE__, __LINE__);
      } else {
        generate_code2(JMP, NO_BREAK_ADDRESS);
        loop_nests[counter_for_loop].has_break = true;
      }

      token = check_next_token(next_token(), ';');

      break;
    }

    case TOKEN_TYPE_CONTINUE: {
      generate_code2(JMP, get_looptop());
      token = check_next_token(next_token(), ';');
      break;
    }

    case TOKEN_TYPE_CASE: {
      token = next_token();
      get_const(NULL);

      check_expression(0, ':');

      if (!get_const(&val)) {
        // not const expression
        display_error_with_line_number(__FILE__, __LINE__);
      } else if (counter_for_switch == 0) {
        // not `switch`
        display_error_with_line_number(__FILE__, __LINE__);
      } else {
        for (int i = switch_nests[counter_for_switch].start_case_list; i <= counter_for_case; i++) {
          if (cases[i].value == val) {
            // duplicate `case`
            display_error_with_line_number(__FILE__, __LINE__);
            break;
          }
        }

        increment(&counter_for_case, SIZE_OF_CASE, "`case` labels exceeds.");

        cases[counter_for_case].value = val;
        cases[counter_for_case].address = next_code_count();
      }

      statement();

      break;
    }

    case TOKEN_TYPE_DEFAULT: {
      if (counter_for_switch == 0) {
        // not `switch`
        display_error_with_line_number(__FILE__, __LINE__);
      } else if (switch_nests[counter_for_switch].default_address != -1) {
        // duplicate `default`
        display_error_with_line_number(__FILE__, __LINE__);
      } else {
        switch_nests[counter_for_switch].default_address = next_code_count();
      }

      token = check_next_token(next_token(), ':');

      statement();

      break;
    }

    case TOKEN_TYPE_FOR: {
      token = check_next_token(next_token(), '(');

      if (token.type == ';') {
        token = next_token();
      } else {
        check_expression(0, ';');
        remove_value();
      }

      UP_LABEL(LB_EXP2);

      if (token.type == ';') {
        generate_code2(LDI, 1);
        token = next_token();
      } else {
        check_expression(0, ';');
      }

      JPF_DOWN(LB_END);
      JMP_DOWN(LB_BODY);

      begin_continue_or_break(type);

      UP_LABEL(LB_EXP3);

      if (token.type == ')') {
        token = next_token();
      } else {
        check_expression(0, ')');
        remove_value();
      }

      JMP_UP(LB_EXP2);

      DOWN_LABEL(LB_BODY);
      statement();
      JMP_UP(LB_EXP3);

      DOWN_LABEL(LB_END);

      break;
    }

    case TOKEN_TYPE_IF: {
      token = next_token();

      check_expression('(', ')');
      JPF_DOWN(LB_ELSE);
      statement();

      if (token.type != TOKEN_TYPE_ELSE) {
        DOWN_LABEL(LB_ELSE);
        break;
      }

      JMP_DOWN(LB_END);
      DOWN_LABEL(LB_ELSE);

      token = next_token();

      statement();

      DOWN_LABEL(LB_END);

      break;
    }

    case TOKEN_TYPE_WHILE: {
      token = next_token();

      UP_LABEL(LB_TOP);
      check_expression('(', ')');
      JPF_DOWN(LB_END);
      statement();
      JMP_UP(LB_TOP);
      DOWN_LABEL(LB_END);

      break;
    }

    case TOKEN_TYPE_DO: {
      token = next_token();

      UP_LABEL(LB_TOP);

      statement();

      if (token.type == TOKEN_TYPE_WHILE) {
        token = next_token();

        check_expression('(', ')');

        token = check_next_token(token, ';');

        JPT_UP(LB_TOP);
      } else {
        // not `while`
        display_error_with_line_number(__FILE__, __LINE__);
      }

      break;
    }

    case TOKEN_TYPE_SWITCH: {
      token = next_token();

      check_expression('(', ')');
      JMP_DOWN(LB_TBL);
      begin_switch();
      statement();
      JMP_DOWN(LB_END);
      DOWN_LABEL(LB_TBL);
      end_switch();
      DOWN_LABEL(LB_END);

      break;
    }

    case TOKEN_TYPE_RETURN: {
      token = next_token();

      if (token.type == ';') {
        if (return_type != VOID_T) {
          // not value
          display_error_with_line_number(__FILE__, __LINE__);
        }
      } else {
        expression();

        if (return_type == VOID_T) {
          // has value
          display_error_with_line_number(__FILE__, __LINE__);
        }
      }

      generate_code2(JMP, NO_RETURN_ADDRESS);

      token = check_next_token(token, ';');

      break;
    }

    case TOKEN_TYPE_PRINTF:
    case TOKEN_TYPE_EXIT: {
      call_system_function(type);
      token = check_next_token(token, ';');
      break;
    }

    case TOKEN_TYPE_INPUT: {
      check_expression(0, ';');
      remove_value();
      break;
    }

    case TOKEN_TYPE_INCREMENT:
    case TOKEN_TYPE_DECREMENT: {
      check_expression(0, ';');
      remove_value();
      break;
    }

    case TOKEN_TYPE_IDENTIFIER: {
      Symbol *symbol = search(token.text);

      if (((symbol->symbol_type == FUNCTION_ID) || (symbol->symbol_type == PROTOTYPE_ID)) && (symbol->type == VOID_T)) {
        call_function(symbol);
        token = check_next_token(token, ';');
      } else {
        check_expression(0, ';');
        remove_value();
      }

      break;
    }

    case TOKEN_TYPE_LEFT_BRACE: {
      block(0);
      break;
    }

    case TOKEN_TYPE_SEMICOLON: {
      token = next_token();
      break;
    }

    case TOKEN_TYPE_EOF_TOKEN: {
      display_error_with_line_number(__FILE__, __LINE__);
      exit(EXIT_FAILURE);
      break;
    }

    default: {
      display_error_value_with_line_number(__FILE__, __LINE__, token.text);
      token = next_token();
      break;
    }

    if ((type == TOKEN_TYPE_FOR) || (type == TOKEN_TYPE_WHILE) || (type == TOKEN_TYPE_DO) || (type == TOKEN_TYPE_SWITCH)) {
      end_continue_or_break();
    }
  }
}

static void begin_continue_or_break(TokenType type) {
  increment(&counter_for_loop, SIZE_OF_LOOP, "nests exceed");

  loop_nests[counter_for_loop].type = type;
  loop_nests[counter_for_loop].top = next_code_count();
  loop_nests[counter_for_loop].has_break = false;
}

static void end_continue_or_break(void) {
  if (loop_nests[counter_for_loop].has_break) {
    backpatch_break(loop_nests[counter_for_loop].top);
  }

  --counter_for_loop;
}

static int get_looptop(void) {
  for (int i = counter_for_loop; i > 0; --i) {
    if (loop_nests[i].type != TOKEN_TYPE_SWITCH) {
      return loop_nests[i].top;
    }
  }

  // not `continue`
  display_error_with_line_number(__FILE__, __LINE__);

  return 0;
}

static void begin_switch(void) {
  increment(&counter_for_switch, SIZE_OF_SWITCH, "switch nests exceed");

  switch_nests[counter_for_switch].default_address = -1;
  switch_nests[counter_for_switch].start_case_list = counter_for_case + 1;
}

static void end_switch(void) {
  int start = switch_nests[counter_for_switch].start_case_list;

  for (int i = start; i <= counter_for_case; ++i) {
    generate_code2(EQCMP, cases[i].value);
    generate_code2(JPT, cases[i].address);
  }

  generate_code1(DEL);

  if (switch_nests[counter_for_switch].default_address != -1) {
    generate_code2(JMP, switch_nests[counter_for_switch].default_address);
  }

  counter_for_case = start - 1;

  --counter_for_switch;
}

static void expression(void) {
  term(2);

  if (token.type == '=') {
    to_left_value();
    token = next_token();
    expression();
    generate_code1(ASSV);
  }
}

static void check_expression(TokenType type1, TokenType type2) {
  if (type1 != 0) {
    token = check_next_token(token, type1);
  }

  expression();

  if (type2 != 0) {
    token = check_next_token(token, type2);
  }
}

static void term(int n) {
  TokenType type;

  if (n == 8) {
    factor();
    return;
  }

  term(n + 1);

  while (n == operation_order(token.type)) {
    type = token.type;
    token = next_token();
    term(n + 1);
    generate_code_binary(type);
  }
}

static void factor(void) {
  Symbol *symbol;
  TokenType type = token.type;

  switch (type) {
    case TOKEN_TYPE_PLUS:
    case TOKEN_TYPE_MINUS:
    case TOKEN_TYPE_NOT:
    case TOKEN_TYPE_INCREMENT:
    case TOKEN_TYPE_DECREMENT: {
      token = next_token();

      factor();

      if ((type == TOKEN_TYPE_INCREMENT) || (type == TOKEN_TYPE_DECREMENT)) {
        to_left_value();
      }

      generate_code_unary(type);

      break;
    }

    case TOKEN_TYPE_NUMBER: {
      generate_code2(LDI, token.value);
      token = next_token();
      break;
    }

    case TOKEN_TYPE_LEFT_PAREN: {
      check_expression('(', ')');
      break;
    }

    case TOKEN_TYPE_PRINTF:
    case TOKEN_TYPE_INPUT:
    case TOKEN_TYPE_EXIT: {
      if (type != TOKEN_TYPE_INPUT) {
        display_error_value_with_line_number(__FILE__, __LINE__, token.text);
      }

      call_system_function(type);
      break;
   }

    case TOKEN_TYPE_IDENTIFIER: {
      symbol = search(token.text);

      switch (symbol->symbol_type) {
        case FUNCTION_ID:
        case PROTOTYPE_ID: {
          if (symbol->type == VOID_T) {
            display_error_value_with_line_number(__FILE__, __LINE__, symbol->name);
          }

          call_function(symbol);
          break;
        }

        case VARIABLE_ID:
        case ARGUMENT_ID: {
          if (symbol->array_length == 0) {
            generate_code3(LD, is_local(symbol), symbol->address);
            token = next_token();
          } else {
            token = next_token();

            if (token.type == '[') {
              generate_code3(LDA, is_local(symbol), symbol->address);
              check_expression('[', ']');
              generate_code2(LDI, SIZE_OF_INT);
              generate_code1(MUL);
              generate_code1(ADD);
              generate_code1(VAL);
            } else {
              // not array index
              display_error_with_line_number(__FILE__, __LINE__);
            }
          }

          if (token.type == TOKEN_TYPE_INCREMENT) {
            to_left_value();

            generate_code1(INC);
            generate_code2(LDI, 1);
            generate_code1(SUB);

            token = next_token();
          }

          if (token.type == TOKEN_TYPE_DECREMENT) {
            to_left_value();

            generate_code1(DEC);
            generate_code2(LDI, 1);
            generate_code1(ADD);

            token = next_token();
          }

          break;
        }

        default: {
          break;
        }
      }

      break;
    }

    default: {
      display_error_value_with_line_number(__FILE__, __LINE__, token.text);
      exit(EXIT_FAILURE);
    }
  }
}

static void call_function(Symbol *symbol) {
  int argc = 0;

  token = check_next_token(next_token(), '(');

  if (token.type != ')') {
    while (true) {
      expression();

      ++argc;

      if (token.type != ',') {
        break;
      }

      token = next_token();
    }
  }

  token = check_next_token(token, ')');

  if (argc != function_pointer->argc) {
    // mismatch the number of arguments
    display_error_with_line_number(__FILE__, __LINE__);
  }

  generate_code2(CALL, function_pointer->address);
}

static void call_system_function(TokenType type) {
  int function_type = 0;

  char *format;

  token = check_next_token(next_token(), '(');

  switch (type) {
    case TOKEN_TYPE_EXIT: {
      function_type = FUNCTION_EXIT;;
      expression();
      break;
    }

    case TOKEN_TYPE_INPUT: {
      function_type = FUNCTION_INPUT;
      break;
    }

    case TOKEN_TYPE_PRINTF: {
      if (token.type != TOKEN_TYPE_STRING) {
        // invalid the 1st argument
        display_error_with_line_number(__FILE__, __LINE__);
      }

      generate_code2(LDI, token.value);

      format = memory_address(token.value);
      token = next_token();

      if (token.type != ',') {
        function_type = FUNCTION_PRINTF;
      } else {
        function_type = FUNCTION_PRINTF_2;

        token = next_token();

        expression();

        if (token.type == ',') {
          display_single_error("`printf` has max 2 arguments");
        }

        if (!check_format(format)) {
          display_single_error("invalid formant");
        }
      }

      break;
    }

    default: {
      break;
    }
  }

  token = check_next_token(token, ')');

  generate_code2(LIB, function_type);
}

static void set_type(void) {
  tmp_symbol.array_length = 0;
  tmp_symbol.argc = 0;
  tmp_symbol.address = 0;

  tmp_symbol.block_nest = counter_for_nest_block;

  switch (token.type) {
    case TOKEN_TYPE_INT: {
      tmp_symbol.type = INT_T;
      break;
    }

    case TOKEN_TYPE_VOID: {
      tmp_symbol.type = VOID_T;
      break;
    }

    default: {
      // invalid type
      display_error_value_with_line_number(__FILE__, __LINE__, token.text);
      tmp_symbol.type = INT_T;
      break;
    }
  }

  token = next_token();
}

static void set_name(void) {
  if (token.type == TOKEN_TYPE_IDENTIFIER) {
    tmp_symbol.name = symbol_name_malloc(token.text);
    token = next_token();
  } else {
    display_error_value_with_line_number(__FILE__, __LINE__, token.text);
    tmp_symbol.name = "unknown";
  }
}

static void set_array_size(void) {
  tmp_symbol.array_length = 0;

  if (token.type != '[') {
    return;
  }

  token = next_token();

  if (token.type == ']') {
    display_error_with_line_number(__FILE__, __LINE__);
    token = next_token();
    tmp_symbol.array_length = 1;
    return;
  }

  get_const(NULL);
  check_expression(0, ']');

  if (get_const(&(tmp_symbol.array_length))) {
    if (tmp_symbol.array_length <= 0) {
      tmp_symbol.array_length = 1;
      display_error_with_line_number(__FILE__, __LINE__);
    }
  } else {
    // not const int
    display_error_with_line_number(__FILE__, __LINE__);
  }

  if (token.type == '[') {
    // multiple dimensions
    display_error_value_with_line_number(__FILE__, __LINE__, token.text);
  }
}

static int operation_order(TokenType type) {
  switch (type) {
    case TOKEN_TYPE_MULTI:
    case TOKEN_TYPE_DIVIDE:
    case TOKEN_TYPE_MOD: {
      return 7;
    }

    case TOKEN_TYPE_PLUS:
    case TOKEN_TYPE_MINUS: {
      return 6;
    }

    case TOKEN_TYPE_LESS:
    case TOKEN_TYPE_LESS_EQUAL:
    case TOKEN_TYPE_GREAT:
    case TOKEN_TYPE_GREAT_EQUAL: {
      return 5;
    }

    case TOKEN_TYPE_EQAUL:
    case TOKEN_TYPE_NOT_EQUAL: {
      return 4;
    }

    case TOKEN_TYPE_AND: {
      return 3;
    }

    case TOKEN_TYPE_OR: {
      return 2;
    }

    case TOKEN_TYPE_ASSIGN: {
      return 1;
    }

    default: {
      return 0;
    }
  }

  return -1;
}

static bool is_global(void) {
  return (bool)(counter_for_nest_block == 0);
}

static int local_malloc(int size) {
  if (size < 0) {
    size = 0;
  }

  local_address += size;

  return local_address - size;
}

static char *symbol_name_malloc(const char *name) {
  char *p = (char *)malloc(strlen(name) + 1);

  if (p == NULL) {
    display_single_error("Out of memory (symbol_name_malloc)");
    exit(EXIT_FAILURE);
  }

  strcpy(p, name);

  return p;
}

static void increment(int *var, int size, const char *error_message) {
  if (*var >= size) {
    display_formatted_error(error_message, size);
    return;
  }

  ++(*var);
}

static bool check_format(char *format) {
  char *p;

  while ((p = strstr(format, "%%")) != NULL) {
    *(p + 1) = '\1';
    *p = *(p + 1);
  }

  if ((p = strchr(format, '%')) == NULL) {
    return false;
  }

  ++p;

  if (*p == '-') {
    ++p;
  }

  while (isdigit(*p)) {
    ++p;
  }

  if (*p == '\0') {
    return false;
  }

  if (strchr("cdioxX", *p) == NULL) {
    return false;
  }

  if (strchr(p, '%') != NULL) {
    return false;
  }

  for (p = format; *p; ++p) {
    if (*p == '\1') {
      *p = '%';
    }
  }

  return true;
}
