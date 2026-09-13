#include <stdio.h>
#include <stdlib.h>

typedef struct {
    float x;
    float y;

} Point;

int main(){
    Point p = {1.1, 2.3};
    printf("size of Point: %zu\n", sizeof(Point));
    
}