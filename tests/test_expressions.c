// Test cases for binary operators and operator precedence
int test1() {
    return 1 + 2;  // Basic addition
}

int test2() {
    return 3 * 4;  // Basic multiplication
}

int test3() {
    return 2 + 3 * 4;  // Operator precedence (* before +)
}

int test4() {
    return (2 + 3) * 4;  // Parentheses override precedence
}

int test5() {
    return 10 - 2 * 3;  // Subtraction and multiplication
}

int test6() {
    return 20 / 5 + 2;  // Division and addition
}

int test7() {
    return 2 * (3 + 4 * (5 + 1));  // Complex nested expression
}
