#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int num = 0;
pthread_mutex_t mutex;

void *threedTest()
{
    for (int i = 0; i < 2000000; i++)
    {
        pthread_mutex_lock(&mutex);
        num++;
        pthread_mutex_unlock(&mutex);
    }
}

int main()
{
    pthread_mutex_init(&mutex, NULL);

    pthread_t t1;
    pthread_t t2;
    pthread_create(&t1, NULL, &threedTest, NULL);
    pthread_create(&t2, NULL, &threedTest, NULL);

    pthread_join(t2, NULL);
    pthread_join(t1, NULL);

    pthread_mutex_destroy(&mutex);

    printf("test %d", num);

    return 0;
}