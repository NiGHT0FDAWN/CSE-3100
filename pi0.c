#include <stdio.h>
#include <stdlib.h>
int pw(int p){
    int ret = 1;
    while(p>0){
        p--;
        ret *= 16;
    }
    return ret;
}
int main()
{
	int n, i;

	printf("n = ");
	scanf("%d", &n);
	double pi = 0.;
	for(i=0;i<=n;i++){
        double ret = (4.0/(8*i+1) - 2.0/(8*i+4) - 1.0/(8*i+5) - 1.0/(8*i+6)) * (1.0/pw(i));
        pi += ret;
    }
	printf("PI = %.10f\n", pi);
	return 0;
}