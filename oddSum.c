#include <stdio.h>
#include <stdlib.h>

int oddSumHelp(int count, int bound, int value)
{
	static int nums[1000];
    static int u = 0;
    static int t = 1;

	if (t == 1){
		u = 0; t = 0;
	}

    if (bound % 2 == 0){
		bound--;
	}

    if (count == 0 && value == 0){
        for (int i = u - 1; i >= 0; i--)
            printf("%d%c", nums[i], i ? ' ' : '\n');
        t = 1;
        return 1;
    }
    if (count <= 0 || value <= 0 || bound <= 0){
        if (t) t = 1;
        return 0;
    }

    int min = count * count;
    int max = count * (2 * bound - count + 1) / 2;
    if (value < min || value > max){
		return 0;
	}

    if (value >= bound) {
        nums[u++] = bound;
        if (oddSumHelp(count - 1, bound - 2, value - bound)){
            return 1;
		}
        u--;
    }
	if (oddSumHelp(count, bound - 2, value))
        return 1;

    return 0;
}

void oddSum(int count, int bound, int value)
{
    	if(value <= 0 || count <= 0 || bound <= 0) return;
    
    	if(bound % 2 == 0) {bound -= 1;}

    	if(!oddSumHelp(count, bound, value)) printf("No solutions.\n");
	else printf("\n");
}

int main(int argc, char *argv[])
{
	if(argc != 4) return -1;

	int count = atoi(argv[1]);
	int bound = atoi(argv[2]);
	int value = atoi(argv[3]);

	//oddSum(12,30,200);
	//oddSum(10,20,100);
	//oddSum(20,20,200);
	oddSum(count, bound, value);
	return 0;
}