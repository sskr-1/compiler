// Example with if-else and while
int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

int sum_to_n(int n) {
    int sum = 0;
    int i = 1;
    
    while (i < n) {
        sum = sum + i;
        i = i + 1;
    }
    
    return sum;
}

int main() {
    int x = max(10, 20);
    int y = sum_to_n(100);
    return x + y;
}
