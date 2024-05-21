#include <stdio.h>

#define length 256

int main()
{

    int number = 111;
    long number2 = 9999999999999;
    char array[length];

    array[0] = (number >> 0) & 0xFF;
    array[1] = (number >> 8) & 0xFF;

    int newNumber = 111;
    printf("number %i \n", number);

    printf("array \n");
    for (int i = 0; i < 2; i++)
    {
        printf("index %i -> %c \n", i, array[i]);
    }

    newNumber = array[0];
    newNumber |= array[1] >> 8;

    printf("new number %i \n", newNumber);

    // for (int i = 0; i < length; i++)
    // {
    //     array[i] = (number2 >> (8 * i)) & 0xff;
    // }

    // for (int i = 0; i < 8; i++)
    // {
    //     x[i + 8] = (number2 >> (8 * i));
    // }

    // for (int i = 0; i < length; i++)
    // {
    //     printf("%d ", x[i]);
    // }

    printf("%s \n", "end");

    return 0;
}