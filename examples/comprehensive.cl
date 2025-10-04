// Comprehensive test demonstrating all features
def fibonacci(n) {
    if n < 2 then {
        return 1;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

def factorial(n) {
    if n < 2 then {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

def testArithmetic() {
    double x = 10.5;
    double y = 3.5;
    double sum = x + y;
    double product = x * y;
    double ratio = x / y;
    return sum + product - ratio;
}

def max(a b) {
    if a > b then {
        return a;
    } else {
        return b;
    }
}

def min(a b) {
    if a < b then {
        return a;
    } else {
        return b;
    }
}

// External function declaration
extern printf(format);

def main() {
    double fib_result = fibonacci(5);
    double fact_result = factorial(5);
    double max_result = max(fib_result, fact_result);
    return max_result;
}