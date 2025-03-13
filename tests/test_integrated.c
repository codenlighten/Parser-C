// Comprehensive test of all implemented features
int main() {
    // Variable declarations and initializations
    int sum = 0;
    int i = 1;
    int max = 10;
    
    // While loop with compound expression
    while (i <= max) {
        // If-else with binary operators
        if ((i * 2) > 15) {
            sum = sum + (i * 2);
        } else {
            sum = sum + i;
        }
        
        // Nested if with comparison
        if (sum > 50) {
            if (i < 8) {
                sum = sum - 10;
            } else {
                sum = sum - 5;
            }
        }
        
        i = i + 1;
    }
    
    // Final calculation using multiple operators
    return sum * 2 + (max - i);
}
