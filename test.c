#include <stdio.h>

int main()
{
    printf("-->");
    char s[] = "new string!";
    int i=sizeof(s)-1;
    printf("%c",s[i]);
}