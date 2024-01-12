#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int num = 0;

void *threedTest()
{
    for (int i = 0; i < 100; i++)
    {
        num++;
    }
}

int main()
{
    pthread_t t1;
    pthread_t t2;
    pthread_create(&t1, NULL, &threedTest, NULL);
    pthread_create(&t2, NULL, &threedTest, NULL);

    pthread_join(t2, NULL);
    pthread_join(t1, NULL);

    printf("test %d", num);

    return 0;
}