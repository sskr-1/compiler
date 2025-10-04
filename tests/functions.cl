// Function examples in C-like language
int add(int x, int y) {
    return x + y;
}

float multiply(float a, float b) {
    return a * b;
}

void print_hello() {
    // This function doesn't return anything
    int dummy = 42;
}

int main() {
    int result1 = add(10, 20);
    float result2 = multiply(3.14, 2.0);
    print_hello();
    
    return result1;
}