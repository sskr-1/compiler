// Loop examples in C-like language
int main() {
    int sum = 0;
    int i = 0;
    
    // While loop
    while (i < 10) {
        sum = sum + i;
        i = i + 1;
    }
    
    // For loop
    int product = 1;
    for (int j = 1; j <= 5; j = j + 1) {
        product = product * j;
    }
    
    return sum + product;
}