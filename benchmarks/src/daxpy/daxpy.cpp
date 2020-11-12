#include <cstdio>
#include <stdlib.h>
#include <random>
#include <m5ops.h>

int main(int argc, char * argv[])
{
    const int N = (argc > 1) ? atoi(argv[1]) : 16384;
    printf("%s %d\n", argv[0], N);
    double X[N], Y[N], alpha = 0.5;
    std::random_device rd; std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1, 2);
    for (int i = 0; i < N; ++i)
    {
        X[i] = dis(gen);
        Y[i] = dis(gen);
    }
    // Start of daxpy loop
    m5_dump_reset_stats(0, 0);
    for (int i = 0; i < N; ++i)
    {
        Y[i] = alpha * X[i] + Y[i];
    }
    m5_dump_reset_stats(0, 0);
    // End of daxpy loop
    double sum = 0;
    for (int i = 0; i < N; ++i)
    {
        sum += Y[i];
    }
    printf("%lf\n", sum);
    return 0;
}
