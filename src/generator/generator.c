#include "generator.h"

int execute(void) {
  int start_local_memory_address = MEMORY_AS_INT(0);

  program_counter = 0;
  base_register = SIZE_OF_MEMORY;
  stack_pointer = STACK_BOTTOM;

  while (true) {
    if ((program_counter < 0) || (code_table_pointer < program_counter)) {
      char buf[32];

      sprintf(buf, "%d", program_counter);
      display_error_value_with_line_number(__FILE__, __LINE__, buf);
      exit(EXIT_FAILURE);
    }

    int opcode  = code_table[program_counter].opcode;
    int opdata  = code_table[program_counter].opdata;
    int address = (code_table[program_counter].flag & 0x01) ? (base_register + opdata) : opdata;

    ++program_counter;

    if (stack_pointer >= STACK_TOP) {
      runtime_error("Stack Overflow");
    }

    if (stack_pointer < STACK_BOTTOM) {
      runtime_error("Stack Underflow");
    }

    switch (opcode) {
      case DEL: {
        --stack_pointer;
        break;
      }

      case STOP: {
        if (stack_pointer >= 0) {
          return POP();
        }

        return 0;
      }

      case JMP: {
        program_counter = opdata;
        break;
      }

      case JPT: {
        if (stack[--stack_pointer]) {
          program_counter = opdata;
        }

        break;
      }

      case JPF: {
        if (!stack[--stack_pointer]) {
          program_counter = opdata;
        }

        break;
      }

      case LIB: {
        if (opdata == FUNCTION_EXIT) {
          return POP();
        }

        call_library(opdata);

        break;
      }

      case LD: {
        PUSH(*(int *)(uintptr_t)address);
        // PUSH(MEMORY_AS_INT(address));
        break;
      }

      case LDA: {
        PUSH(address);
        break;
      }

      case LDI: {
        PUSH(opdata);
        break;
      }

      case ST: {
        ASSIGN(address, stack[stack_pointer - 1]);
        --stack_pointer;
        break;
      }

      case ADBR: {
        base_register += opdata;

        if (base_register < start_local_memory_address) {
          // stack overflow
          display_error_with_line_number(__FILE__, __LINE__);
          exit(EXIT_FAILURE);
        }

        break;
      }

      case NOP: {
        ++program_counter;
        break;
      }

      case ASS: {
        ASSIGN(stack[stack_pointer - 2], stack[stack_pointer - 1]);
        stack_pointer -= 2;
        break;
      }

      case ASSV: {
        ASSIGN(stack[stack_pointer - 2], stack[stack_pointer - 1]);
        stack[stack_pointer - 2] = stack[stack_pointer - 1];
        --stack_pointer;
        break;
      }

      case VAL: {
        stack[stack_pointer - 1] = MEMORY_AS_INT(stack[stack_pointer - 1]);
        break;
      }

      case EQCMP: {
        if (opdata == stack[stack_pointer - 1]) {
          stack[stack_pointer - 1] = 1;
        } else {
          PUSH(0);
        }

        break;
      }

      case CALL: {
        PUSH(program_counter);
        program_counter = opdata;
        break;
      }

      case RET: {
        program_counter = POP();
        break;
      }

      case INC: {
        INC(+1);
        break;
      }

      case DEC: {
        INC(-1);
        break;
      }

      case NOT: {
        UNARY_OPERATOR(!);
        break;
      }

      case NEG: {
        UNARY_OPERATOR(-);
        break;
      }

      case ADD: {
        BINARY_OPERATOR(+);
        break;
      }

      case SUB: {
        BINARY_OPERATOR(-);
        break;
      }

      case MUL: {
        BINARY_OPERATOR(*);
        break;
      }

      case DIV: {
        IS_ZERO();
        BINARY_OPERATOR(/);
        break;
      }

      case MOD: {
        IS_ZERO();
        BINARY_OPERATOR(%);
        break;
      }

      case LESS: {
        BINARY_OPERATOR(<);
        break;
      }

      case LSEQ: {
        BINARY_OPERATOR(<=);
        break;
      }

      case GRT: {
        BINARY_OPERATOR(>);
        break;
      }

      case GTEQ: {
        BINARY_OPERATOR(>=);
        break;
      }

      case EQ: {
        BINARY_OPERATOR(==);
        break;
      }

      case NTEQ: {
        BINARY_OPERATOR(!=);
        break;
      }

      case AND: {
        BINARY_OPERATOR(&&);
        break;
      }

      case OR: {
        BINARY_OPERATOR(||);
        break;
      }

      default: {
        char buf[32];

        sprintf(buf, "%d", (program_counter - 1));
        display_error_value_with_line_number(__FILE__, __LINE__, buf);
        exit(EXIT_FAILURE);
      }
    }
  }
}

void dump(void) {
  char buf[30] = "";

  for (int n = 0; n <= code_table_pointer; ++n) {
    OperationCode opcode = code_table[n].opcode;

    printf("%04d: ", n);

    sprintf(buf, "%d", code_table[n].opdata);

    if (code_table[n].flag) {
      if ((opcode == LD) || (opcode == LDA) || (opcode == ST)) {
        strcat(buf, "[b]");
      }
    }

    switch (opcode) {
      case RET:
      case ASS:
      case ASSV:
      case NOT:
      case INC:
      case DEC:
      case NEG:
      case ADD:
      case SUB:
      case MUL:
      case DIV:
      case MOD:
      case LESS:
      case LSEQ:
      case GRT:
      case GTEQ:
      case EQ:
      case NTEQ:
      case AND:
      case OR:
      case VAL:
      case DEL:
      case NOP:
      case STOP: {
        printf("%-5s\n", opcodes[opcode]);

        if ((opcode == RET) || (opcode == STOP)) {
          printf("\n");
        }

        break;
      }

      case LD:
      case LDA:
      case LDI:
      case ST:
      case CALL:
      case ADBR:
      case LIB:
      case JMP:
      case JPT:
      case JPF:
      case EQCMP: {
        printf("%-5s %-11s\n", opcodes[opcode], buf);
        break;
      }

      default: {
        printf("Illegal code (#%d, code = %d)\n", (program_counter - 1), opcode);
        exit(EXIT_FAILURE);
      }
    }
  }
}

int generate_code3(OperationCode opcode, int flag, int data) {
  if (const_fold(opcode)) {
    return code_table_pointer;
  }

  if (++code_table_pointer > SIZE_OF_CODE) {
    display_error_with_line_number(__FILE__, __LINE__);
    exit(EXIT_FAILURE);
  }

  code_table[code_table_pointer].opcode = opcode;
  code_table[code_table_pointer].flag   = flag;
  code_table[code_table_pointer].opdata = data;

  return code_table_pointer;
}

void generate_code_unary(TokenType type) {
  OperationCode opcode = NOP;

  switch (type) {
    case TOKEN_TYPE_PLUS: {
      return;
    }

    case TOKEN_TYPE_MINUS: {
      opcode = NEG;
      break;
    }

    case TOKEN_TYPE_NOT: {
      opcode = NOT;
      break;
    }

    case TOKEN_TYPE_INCREMENT: {
      opcode = INC;
      break;
    }

    case TOKEN_TYPE_DECREMENT: {
      opcode = DEC;
      break;
    }

    default: {
      break;
    }
  }

  generate_code1(opcode);
}

void generate_code_binary(TokenType type) {
  OperationCode opcode = NOP;

  switch (type) {
    case TOKEN_TYPE_PLUS: {
      opcode = ADD;
      break;
    }

    case TOKEN_TYPE_MINUS: {
      opcode = SUB;
      break;
    }

    case TOKEN_TYPE_MULTI: {
      opcode = MUL;
      break;
    }

    case TOKEN_TYPE_DIVIDE: {
      opcode = DIV;
      break;
    }

    case TOKEN_TYPE_LESS: {
      opcode = LESS;
      break;
    }

    case TOKEN_TYPE_LESS_EQUAL: {
      opcode = LSEQ;
      break;
    }

    case TOKEN_TYPE_GREAT: {
      opcode = GRT;
      break;
    }

    case TOKEN_TYPE_GREAT_EQUAL: {
      opcode = GTEQ;
      break;
    }

    case TOKEN_TYPE_EQAUL: {
      opcode = EQ;
      break;
    }

    case TOKEN_TYPE_NOT_EQUAL: {
      opcode = NTEQ;
      break;
    }

    case TOKEN_TYPE_AND: {
      opcode = AND;
      break;
    }

    case TOKEN_TYPE_OR: {
      opcode = OR;
      break;
    }

    case TOKEN_TYPE_MOD: {
      opcode = MOD;
      break;
    }

    default: {
      break;
    }
  }

  generate_code1(opcode);
}

void to_left_value(void) {
  switch (code_table[code_table_pointer].opcode) {
    case VAL: {
      --code_table_pointer;
      break;
    }

    case LD: {
      code_table[code_table_pointer].opcode = LDA;
      break;
    }

    default: {
      // invalid left value
      display_error_with_line_number(__FILE__, __LINE__);
      break;
    }
  }
}

void remove_value(void) {
  if (code_table[code_table_pointer].opcode == ASSV) {
    code_table[code_table_pointer].opcode = ASS;
  } else {
    generate_code1(DEL);
  }
}

char *memory_address(int index) {
  return memories + index;
}

int global_malloc(int size) {
  if (size <= 0) {
    return global_address;
  }

  // alignment
  global_address = ((global_address + 3) / 4) * 4;

  if ((global_address + size) > SIZE_OF_MEMORY) {
    display_error_with_line_number(__FILE__, __LINE__);
    exit(EXIT_FAILURE);
  }

  memset((memories + global_address), 0, size);

  global_address += size;

  return global_address - size;
}

int string_malloc(char *s) {
  if (s == NULL) {
    s = "";
  }

  int address = global_malloc(strlen(s) + 1);

  strcpy(memory_address(address), s);

  return address;
}

int next_code_count(void) {
  return code_table_pointer + 1;
}

bool get_const(int *var) {
  static int c = -99;

  if (var == NULL) {
    c = next_code_count();
    return true;
  }

  if ((code_table_pointer == c) && (code_table[code_table_pointer].opcode == LDI)) {
    *var = code_table[code_table_pointer].opdata;

    --code_table_pointer;

    return true;
  }

  return false;
}

void backpatch(int code_table_index, int address) {
  // HACK:
  if (code_table[code_table_index].opcode == LDI) {
    if (address < 0) {
      return;
    }
  }

  code_table[code_table_index].opdata = address;
}

void backpatch_calladdress(void) {
  if (code_table[0].opdata < 0) {
    // no main function
    display_error_with_line_number(__FILE__, __LINE__);
  }

  for (int i = 2; i < code_table_pointer; ++i) {
    int data = code_table[i].opdata;

    if ((code_table[i].opcode == CALL) && (data < 0)) {
      code_table[i].opdata = get_symbol_table_pointer(-data)->address;

      if (code_table[i].opdata < 0) {
        // not defined
        display_error_value_with_line_number(__FILE__, __LINE__, (get_symbol_table_pointer(-data)->name));
      }
    }
  }
}

void backpatch_return(int function_address) {
  for (int i = code_table_pointer; i >= function_address; --i) {
    if (!is_code(i, JMP, NO_RETURN_ADDRESS)) {
      break;
    }

    --code_table_pointer;
  }

  for (int i = code_table_pointer; i >= function_address; --i) {
    if (is_code(i, JMP, NO_RETURN_ADDRESS)) {
      code_table[i].opdata = code_table_pointer + 1;
    }
  }
}

void backpatch_break(int looptop) {
  for (int i = code_table_pointer; i >= looptop; --i) {
    if (!is_code(i, JMP, NO_BREAK_ADDRESS)) {
      break;
    }

    --code_table_pointer;
  }

  for (int i = code_table_pointer; i >= looptop; --i) {
    if (is_code(i, JMP, NO_BREAK_ADDRESS)) {
      code_table[i].opdata = code_table_pointer + 1;
    }
  }
}

static void call_library(int function_type) {
  const int BUFFER_SIZE = 40;

  int data = 0;

  char number_as_string[BUFFER_SIZE];

  if (function_type != FUNCTION_INPUT) {
    data = POP();
  }

  switch (function_type) {
    case FUNCTION_INPUT: {
      fgets(number_as_string, BUFFER_SIZE, stdin);
      stack[stack_pointer++] = (int)strtol(number_as_string, NULL, 10);
      break;
    }

    case FUNCTION_PRINTF: {
      printf("%s", memory_address(data));
      fflush(stdout);
      break;
    }

    case FUNCTION_PRINTF_2: {
      printf(memory_address(stack[--stack_pointer]), data);
      fflush(stdout);
      break;
    }

    default: {
      break;
    }
  }
}

static bool const_fold(OperationCode opcode) {
  if ((code_table_pointer <= 1) || (code_table[code_table_pointer].opcode != LDI)) {
    return false;
  }

  if (opcode == NOT) {
    code_table[code_table_pointer].opdata = !code_table[code_table_pointer].opdata;
    return true;
  }

  if (opcode == NEG) {
    code_table[code_table_pointer].opdata = -code_table[code_table_pointer].opdata;
    return true;
  }

  if ((code_table[code_table_pointer - 1].opcode == LDI) && is_binary_opcode(opcode)) {
    code_table[code_table_pointer - 1].opdata = binary_expression(opcode, code_table[code_table_pointer - 1].opcode, code_table[code_table_pointer].opdata);
    --code_table_pointer;
    return true;
  }

  return false;
}

static bool is_binary_opcode(OperationCode opcode) {
  switch (opcode) {
    case ADD:
    case SUB:
    case MUL:
    case DIV:
    case MOD:
    case LESS:
    case LSEQ:
    case GRT:
    case GTEQ:
    case EQ:
    case NTEQ:
    case AND:
    case OR: {
      return true;
    }

    default: {
      return false;
    }
  }
}

static int binary_expression(OperationCode opcode, int data1, int data2) {
  if (((opcode == DIV) || (opcode == MOD)) && (data2 == 0)) {
    data2 = 1;
    display_error_with_line_number(__FILE__, __LINE__);
  }

  switch (opcode) {
    case ADD: {
      data1 = data1 + data2;
      break;
    }

    case SUB: {
      data1 = data1 - data2;
      break;
    }

    case MUL: {
      data1 = data1 * data2;
      break;
    }

    case DIV: {
      data1 = data1 / data2;
      break;
    }

    case MOD: {
      data1 = data1 % data2;
      break;
    }

    case LESS: {
      data1 = data1 < data2;
      break;
    }

    case LSEQ: {
      data1 = data1 <= data2;
      break;
    }

    case GRT: {
      data1 = data1 > data2;
      break;
    }

    case GTEQ: {
      data1 = data1 >= data2;
      break;
    }

    case EQ: {
      data1 = data1 == data2;
      break;
    }

    case NTEQ: {
      data1 = data1 != data2;
      break;
    }

    case AND: {
      data1 = data1 && data2;
      break;
    }

    case OR: {
      data1 = data1 || data2;
      break;
    }

    default: {
      break;
    }
  }

  return data1;
}

static bool is_code(int code_table_index, OperationCode opcode, int data) {
  return (bool)((code_table[code_table_index].opcode == opcode) && (code_table[code_table_index].opdata == data));
}

static void runtime_error(char *error_message) {
  fprintf(stderr, "Runtime error (#%d) %s\n", (program_counter - 1), error_message);
  exit(EXIT_FAILURE);
}
