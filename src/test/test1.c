#include <stdio.h>
#include <stdlib.h>

int foo(int a, int b)
{
    int c = a + a;
    return 0;
}

struct A {
    int a;
};

int main()
{
    struct A a = {1}; 
    struct A *pa = &a;

    int b = pa->a;

    return 0;
}
