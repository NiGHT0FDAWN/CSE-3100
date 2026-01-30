#include <stdio.h>

int main(void)
{
    double in;
    double total = 0.0;
    int count = 0;
    while (scanf("%lf", &in) == 1) {
        count++;
        total += in;
        double average = total / count;
        
        printf("Total=%.6f Average=%.6f\n", total, average);
    }
}