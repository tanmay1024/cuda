#include <stdio.h>
#include <stdlib.h>

int main() {
    int arr[] = {12, 24, 36, 48, 60};

    size_t size = sizeof(arr) / sizeof(arr[0]);
    printf("Size of array: %zu\n", size);
    printf("Size of size_t: %zu\n", sizeof(size_t));
    printf("Size of int in bytes: %zu\n", sizeof(int));

}