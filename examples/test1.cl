// Simple arithmetic expression
def add(a b) {
    return a + b;
}

// Variable declarations and assignments
def testVariables() {
    double x = 5.0;
    double y = 3.14;
    double result = x * y;
    return result;
}

// Conditional logic
def max(a b) {
    if a > b then {
        return a;
    } else {
        return b;
    }
}

// More complex function
def calculate(x y z) {
    double temp = x + y;
    if temp > z then {
        return temp * 2.0;
    } else {
        return temp - z;
    }
}