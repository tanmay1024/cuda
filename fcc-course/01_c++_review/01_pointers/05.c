#include <stdio.h>
#include <stdlib.h>

int main(){
    int arr[] = {12, 24, 36, 48, 60};
    int* ptr = arr;

    printf("Position one: %d\n", *ptr);

    for (int i = 0; i < 5; i++){
        printf("%d ", *ptr);
        printf("%p\n ", ptr);
        ptr++;
    }

}