// Arithmetic operations in C-like language
int main() {
    int a = 20;
    int b = 4;
    
    int add = a + b;
    int sub = a - b;
    int mul = a * b;
    int div = a / b;
    int mod = a % b;
    
    // Comparison operations
    bool eq = (a == b);
    bool ne = (a != b);
    bool lt = (a < b);
    bool gt = (a > b);
    bool le = (a <= b);
    bool ge = (a >= b);
    
    // Logical operations
    bool and_result = (a > 10) && (b < 10);
    bool or_result = (a < 10) || (b > 10);
    bool not_result = !(a == b);
    
    return add + sub + mul + div + mod;
}