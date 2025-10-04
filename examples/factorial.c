// Factorial function using recursion
int factorial(int n) {
    if (n < 2) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    int result = factorial(5);
    return result;
}
