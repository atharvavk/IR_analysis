#include <stdio.h>

int main()
{
    int a = 10, b = 20, c = 0;
    // printf("a = %d , b = %d , c = %d\n", a, b, c);
    while (b > a)
    {
        a++;
        if (c < a)
        {
            c++;
        }
    }
    a = 96;
    if (b == a)
    {
        c++;
    }
    while (b > a)
    {
        a++;
        if (c < a)
        {
            c++;
        }
    }
    return 0;
}