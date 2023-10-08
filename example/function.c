int add(int a, int b);

int main(void) {
  int result;

  result = add(10, 20);

  printf("%d\n", result);

  return 0;
}

int add(int a, int b) {
  int sum;

  sum = a + b;

  return sum;
}
