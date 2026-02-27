#include <stdio.h>
typedef struct {
    int id;
    int points;
} Questions;

int main() {
    Questions * q1;
    q1->id = 1;
    q1->points = 3;

    printf("%d\n",q1->points);
    return 0;
}