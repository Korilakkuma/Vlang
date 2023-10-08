int main(void) {
  int n, sum;

  sum = 1;
  printf("sum > ");

  while ((n = input()) != 0) {
    sum = sum + n;
    printf("    > %d\n", sum);
  }

  return 0;
}
