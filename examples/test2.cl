// Function with integer types
def factorial(n) {
    if n < 2 then {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

// Function with multiple variables
def quadratic(a b c x) {
    double temp1 = a * x * x;
    double temp2 = b * x;
    return temp1 + temp2 + c;
}

// External function declaration
extern sin(x);
extern cos(x);

// Function using external functions
def circle_area(radius) {
    double pi = 3.14159;
    return pi * radius * radius;
}