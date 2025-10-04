extern int putchar(int c);

int add(int a, int b) {
  int c = a + b;
  return c;
}

int main() {
  int x = add(40, 2);
  if (x == 42) {
    putchar(65);
  } else {
    putchar(66);
  }
  return x;
}
