#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int comp(const void *a, const void *b) {
        char *sa = *(const char **)a;
        char *sb = *(const char **)b;
        int ca = 0, cb = 0;
        for (char *p = sa; *p; p++)
                if (islower((char)*p)) {
                    ca++;
                }
        for (char *p = sb; *p; p++)
                if (islower((char)*p)) {
                    cb++;
                }
        return ca - cb;
}

void print_elements(char **elems, int count) {
    for (int i = 0; i < count; i++) {
        printf("%s\n", elems[i]);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        perror("Usage : ./place <strings>");
        exit(1);
    }
    qsort(&argv[1], argc - 1, sizeof(char *), comp);
    print_elements(&argv[1], argc - 1);

    return 0;
}
