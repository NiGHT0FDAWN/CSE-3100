#include <stdio.h>
#include <stdlib.h>

double two_d_random(int n)
{
	//Fill in code below
	//When deciding which way to go for the next step, generate a random number as follows.
	//Treat r = 0, 1, 2, 3 as up, right, down and left respectively.

	//The random walk should stop once the x coordinate or y coordinate reaches $-n$ or $n$. 
	//The function should return the fraction of the visited $(x, y)$ coordinates inside (not including) the square.
    int x = 0;
    int y = 0; //dx dy keep track of 
    int dx = 0;
    int dy = 0;
    int step = 0.0;
    int visited[2*n+1][2*n+1];
    for(int i=0; i<(2*n+1); i++){
        for(int j=0; j<(2*n+1); j++){
            visited[i][j]=0;
        }
    }
    while(x<n && x>-n && y<n && y>-n){
        int r = rand() % 4;
        if(r==0){
            dx = 0;
            dy = 1;
        }
        if(r==1){
            dx = 1;
            dy = 0;
        }
        if(r==2){
            dx = 0;
            dy = -1;
        }
        if(r==3){
            dx = -1;
            dy = 0;
        }
        visited[x+1+n][y+1+n]++;
        x+=dx;
        y+=dy;
    }
    //printf("x:%d y:%d dx:%d dy:%d\n", x,y,dx,dy);
    for(int i=0; i<(2*n+1); i++){
        for(int j=0; j<(2*n+1); j++){
            if(visited[i][j] != 0){step++;}
        }
    }
    return step*1.0/((2*n-1)*(2*n-1)*1.0);
}

//Do not change the code below
int main(int argc, char *argv[])
{
	int trials = 1000;
	int i, n, seed;
	if (argc == 2) seed = atoi(argv[1]);
	else seed = 12345;

	srand(seed);
	for(n=1; n<=64; n*=2)
	{	
		double sum = 0.;
		for(i=0; i < trials; i++)
		{
			double p = two_d_random(n);
			sum += p;
		}
		printf("%d %.3lf\n", n, sum/trials);
	}
	return 0;
}