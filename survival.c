#include <stdio.h>
#include <stdlib.h>

typedef struct particle {
    int mass;
    struct particle *next;
} ParticleSet;

void addToSet(ParticleSet **p, int v) {
    v->next = p[0];
}

int removeFromSet(ParticleSet **p) {
}
int isSetEmpty(ParticleSet *p) {}

int topOfTheSet(ParticleSet *p) {}

void printSet(ParticleSet *p) {
    if (isSetEmpty(p)) {
        printf("Empty\n");
        return;
    }
    while (p != NULL) {
        printf("%d ", p->mass);
        p = p->next;
    }
    printf("\n");
}

void freeSet(ParticleSet **b) {}

ParticleSet *fight(int *particles, int count) {}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: ./survival <particle values>\n");
        exit(1);
    }

    int count = argc - 1;
    int *particles = malloc(count * sizeof(int));

    for (int i = 0; i < count; i++) {
        particles[i] = atoi(argv[i + 1]);
    }

    ParticleSet *b = fight(particles, count);
    printSet(b);
    
    return 0;
}
