#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

// 这里的计算平方只是假设，可以想象成是一个需要耗时1秒的复杂计算
int calc_sqr(int a)
{
    sleep(1);
    return a * a;
}

// 对一个数组中的数分别求平方然后再汇总求和
int main(int argc, char const *argv[])
{
    int items[5] = {1, 2, 3, 4, 5};
    int sqrs[5];

    struct timeval start_t, end_t;
    gettimeofday(&start_t, NULL);

    // 计算items中每个数的平方，并存储到sqrs中
    for (int i = 0; i < 5; i++)
    {
        sqrs[i] = calc_sqr(items[i]);
    }

    gettimeofday(&end_t, NULL);
    // 输出计算耗时
    printf("time = %ld秒\n", end_t.tv_sec - start_t.tv_sec);

    // 累加所有的平方值
    int sum = 0;
    for (int i = 0; i < 5; i++)
    {
        sum += sqrs[i];
    }
    printf("sum = %d\n", sum);

    return 0;
}