#include <stdio.h>

int main()
{
    printf("-->");
    char s[] = "new string!";
    int i=sizeof(s)-1;
    printf("%c",s[i]);
    printf("abcds\n");
    int c[26] = {0,0,0};
    for (int j = 0; j<sizeof(c);j++){
        printf("%d ",c[j]);
    }
    printf("\n");
}