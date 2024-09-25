#include <stdio.h>
#include <stdlib.h>

// Recursive Fibonacci function
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main(int argc, char *argv[]) {
    // Check if the correct number of arguments are passed
    if (argc != 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;  // Return with an error code
    }

    // Convert the argument from a string to an integer
    int number = atoi(argv[1]);

    // Check if the number is non-negative
    if (number < 0) {
        printf("Please enter a non-negative integer.\n");
        return 1;
    }

    // Calculate Fibonacci and display the result
    int result = fibonacci(number);
    printf("Fibonacci of %d is %d\n", number, result);

    return 0;
}

