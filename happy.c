#include <stdio.h>
#include <stdlib.h>
int step(int m){
    int pwr;
    int tot = 0;
    while(m>0){
        pwr = m%10;
        m/=10;
        tot += pwr*pwr;
    }
    return tot;
}

int main()
{
	int n;

	printf("n = ");
	scanf("%d", &n);

	int m = n;
    while(n!=4 && n!=1){
        n = step(n);
        printf("%d\n",n);
    }
    if(n==1) printf("%d is a happy number.\n", m);
	else printf("%d is NOT a happy number.\n", m);
	return 0;
}