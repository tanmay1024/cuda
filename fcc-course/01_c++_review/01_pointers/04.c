#include <stdio.h>
#include <stdlib.h>

int main(){

    int* ptr = NULL;
    printf("1. Initial ptr value: %p\n", (void*)ptr);

    if (ptr == NULL){
        printf("2. Ptr is NULL, cannot dereference \n");
    }

    ptr = malloc(sizeof(int));
    if (ptr == NULL){
        printf("3. Memory allocation failed\n");
        return 1;
    }

    
}