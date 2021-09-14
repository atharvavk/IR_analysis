#include <stdio.h>


void print2(){
   return;
}

void print(){
    // print2();
    int a = 6;
    int b = 9;
    if(a+3 == b +2)
        print();
    else
        print2();
}

int main()
{
    void (*ptr)(void) = &print2;
    (*ptr)();
    print();
}

