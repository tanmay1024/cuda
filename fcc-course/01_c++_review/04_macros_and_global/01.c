#include <stdio.h>
#include <stdlib.h>

#define PI 3.14159
#define AREA(r) (PI * r * r)

#ifndef radius
#define radius 7
#endif


#if radius > 10
#define radius10
#elif radius < 5
#define radius 5
#else
#define radius 7
#endif

int main(){
    printf("Area of circle with radius %d is: %.2f\n", radius, AREA(radius));
}