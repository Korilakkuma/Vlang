#include "symbol_table.h"

Symbol *enter(Symbol symbol, SymbolType type) {
  if (((type == VARIABLE_ID) || (type == ARGUMENT_ID)) && (symbol.type == VOID_T)) {
    display_error_with_line_number(__FILE__, __LINE__);
    symbol.type = INT_T;
  }

  symbol.symbol_type = type;

  check_name(symbol);

  if (symbol_table_pointer > SIZE_OF_TABLE) {
    display_error_with_line_number(__FILE__, __LINE__);
    exit(EXIT_FAILURE);
  }

  int table_number = ++symbol_table_pointer;

  symbol_table[table_number] = symbol;

  if (type == ARGUMENT_ID) {
    symbol_table[table_number].memory = LOCAL;
  }

  if (type == VARIABLE_ID) {
    set_address(&symbol_table[table_number]);
  }

  if (type == FUNCTION_ID) {
    symbol_table[table_number].address = -table_number;
  }

  return &symbol_table[table_number];
}

void open_local_table(void) {
  start_local_address = symbol_table_pointer + 1;
}

void close_local_table(Symbol *symbol) {
  symbol_table_pointer = start_local_address - 1 + symbol->argc;
  start_local_address = SIZE_OF_LOCAL_TABLE;
}

Symbol *search_name(char *s) {
  int i;

  for (i = symbol_table_pointer; i >= SIZE_OF_LOCAL_TABLE; --i) {
    if (strcmp(symbol_table[i].name, s) == 0) {
      return symbol_table + i;
    }
  }

  while (i >= GLOBAL_TABLE_ADDRESS) {
    if ((symbol_table[i].symbol_type != ARGUMENT_ID) && (strcmp(symbol_table[i].name, s) == 0)) {
      return symbol_table + i;
    }

    --i;
  }

  return NULL;
}

Symbol *search(char *s) {
  static Symbol *target_pointer = NULL;
  static Symbol tmp = { "unknown", VARIABLE_ID, INT_T, 0, 0, 0, 0 };

  Symbol *target = search_name(s);

  if (target == NULL) {
    // display_error_value_with_line_number(__FILE__, __LINE__, s);

    if (target_pointer == NULL) {
      target_pointer = enter(tmp, VARIABLE_ID);
    }

    target = target_pointer;
  }

  return target;
}

void delete_function_table(Symbol *symbol1, Symbol *symobol2) {
  if (symbol1 == NULL) {
    return;
  }

  if ((symbol1->type != symobol2->type) || (symbol1->argc != symobol2->argc)) {
    return;
  }

  if ((symbol1->symbol_type == PROTOTYPE_ID) && (symobol2->symbol_type == FUNCTION_ID)) {
    for (int i = 0; i < symobol2->argc; ++i) {
      *(symbol1 + i) = *(symobol2 + i);
    }
  }

  symbol_table_pointer -= (symobol2->argc + 1);

  return;
}

bool is_local(Symbol *symbol) {
  return (bool)(symbol->memory == LOCAL);
}

Symbol *get_symbol_table_pointer(int index) {
  return &symbol_table[index];
}

static void check_name(Symbol symbol) {
  Symbol *tmp;

  if ((symbol.symbol_type != ARGUMENT_ID) && (symbol.symbol_type != VARIABLE_ID)) {
    return;
  }

  if ((tmp = search_name(symbol.name)) == NULL) {
    return;
  }

  if (symbol.symbol_type == ARGUMENT_ID) {
    ++counter_for_nest_block;
  }

  if (tmp->memory >= counter_for_nest_block) {
    display_error_value_with_line_number(__FILE__, __LINE__, symbol.name);
  }
}
