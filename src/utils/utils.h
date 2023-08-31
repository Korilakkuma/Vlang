#include "../types.h"

#define NUMBER_OF_ERROR_MESSAGES 10

typedef enum {
  UNKNOWN,
  SHIFT_JIS = 'S',
  EUCJP = 'E',
  UTF8 = 'U'
} ENCODING;

extern int counter_for_error;

extern int get_line_number(void);
