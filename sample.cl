// Sample C-like language program for testing the LLVM IR generator

// Function to calculate factorial
int factorial(int n) {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

// Main function
int main() {
    int i;
    int result;

    for (i = 1; i <= 5; i = i + 1) {
        result = factorial(i);
        // Just return the last computed factorial for now
    }

    return result;
}