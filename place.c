#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  print_elements(&argv[1], argc - 1);

  return 0;
}
