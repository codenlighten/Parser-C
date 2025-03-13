// Test cases for if/else and while statements
int main() {
    int x = 10;
    int y = 5;
    
    // Test if-else with comparison operators
    if (x > y) {
        x = x + 1;
    } else {
        y = y + 1;
    }
    
    // Test while loop with multiple statements
    while (x > 0) {
        y = y + x;
        x = x - 1;
    }
    
    // Test nested if-else
    if (y > 20) {
        if (x == 0) {
            y = y + 5;
        } else {
            y = y - 5;
        }
    }
    
    // Test while with if condition
    while (y > 0) {
        if (y > 10) {
            y = y - 2;
        } else {
            y = y - 1;
        }
    }
    
    return x + y;
}
