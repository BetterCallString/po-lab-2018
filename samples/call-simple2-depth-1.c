// OPT: "--cs_depth=1"
int bar() {
  return 100;
}

int foo() {
  return bar();
}

int main() {
  return foo();
}
