// Test function definitions and calls
int add(int a, int b) {
    return a + b;
}

int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main() {
    int result = 0;
    
    // Test basic function call
    result = add(5, 3);
    
    // Test recursive function call
    result = factorial(5);
    
    // Test function call with expressions
    result = add(factorial(3), fibonacci(4));
    
    // Test nested function calls
    result = add(add(1, 2), add(3, 4));
    
    return result;
}
