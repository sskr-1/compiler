// Fibonacci using iterative approach
int fibonacci(int n) {
    int a = 0;
    int b = 1;
    int i = 0;
    
    while (i < n) {
        int temp = a + b;
        a = b;
        b = temp;
        i = i + 1;
    }
    
    return a;
}

int main() {
    int result = fibonacci(10);
    return result;
}
