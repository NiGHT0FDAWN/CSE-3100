// Do not modify starter code
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_LEN 100

void commonChars(char arr[][MAX_LEN], int n) {
    int common[26];
    for (int i = 0; i < 26; i++) {
        common[i] = true;
    }

    for (int i = 0; i < n; i++) {
        int x = strlen(arr[i]);
        int c[26]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        for (int j = 0; j<x; j++){
            int uni = arr[i][j] - 'a';
            // printf("%d%c",j,arr[i][j]);
            if (common[uni]){
                c[uni]++;
                // printf("!");
            }
            // printf(" ");
        }
        for (int j = 0; j<26; j++){
            if (!common[j] || c[j]){
                continue;
            }
            common[j] = false;
            // printf("-%c ",j+'a');
        }
        // printf("\n");
    }
    
    printf("Common characters: ");
    int found = 0;
    for (int i = 0; i<26;i++){
        if (common[i]){
            if (!found){
                printf("%c",'a' + i);
                found = true;
            }
            else {
                printf(" %c",'a' + i);
            }
        }
    }
    if (!found) {
        printf("None");
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s string1 string2 ...\n", argv[0]);
        return 1;
    }

    int n = argc - 1;
    char arr[n][MAX_LEN];

    for (int i = 0; i < n; i++) {
        strcpy(arr[i], argv[i + 1]);
    }

    commonChars(arr, n);

    return 0;
}