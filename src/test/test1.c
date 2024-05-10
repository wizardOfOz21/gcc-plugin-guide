#include <stdio.h>
#include <stdlib.h>

int foo(int a, int b)
{
    int c = a + a;
    c = 5;
    return 0;
}

int main()
{
    int a = 1;
    int b = 5;
    b = a;

    return 0;
}
