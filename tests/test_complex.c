int main() {
    // Basic variable declarations
    int a = 5;
    int b = 3;
    int c = 2;
    
    // Complex expression with variables and operators
    int result = (a * b + c) * (a + b) / c;
    
    // Reassignment with expression
    a = result + b * c;
    
    // Final calculation
    return a - (b + c) * 2;
}
