#include "../types.h"

#define SIZE_OF_TABLE        1000
#define SIZE_OF_LOCAL_TABLE  9999
#define GLOBAL_TABLE_ADDRESS 1

extern int counter_for_nest_block;
extern void set_address(Symbol *symbol);

static Symbol symbol_table[SIZE_OF_TABLE];

static int start_local_address = SIZE_OF_LOCAL_TABLE;
static int symbol_table_pointer = 0;

static void check_name(Symbol symbol);
