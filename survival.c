#include <stdio.h>
#include <stdlib.h>

typedef struct particle {
    int mass;
    struct particle *next;
} ParticleSet;

void addToSet(ParticleSet **p, int v) {
    ParticleSet *new = malloc(sizeof(ParticleSet));
    new->mass = v;
    new->next = *p;
    *p = new;
}

int removeFromSet(ParticleSet **p) {
    ParticleSet *t = *p;
    int v = t->mass;
    *p = t->next;
    return v;
    free(t);
}
int isSetEmpty(ParticleSet *p) {
    if (p == NULL){
        return 1;
    } else {
        return 0;
    }
}

int topOfTheSet(ParticleSet *p) {
    return p->mass;
}

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

void freeSet(ParticleSet **b) {
    while (*b != NULL) {
        ParticleSet *t = *b;
        *b = (*b)->next;
        free(t);
    }
}

ParticleSet *fight(int *particles, int count) {
    ParticleSet *stack = NULL;
    for (int i = 0; i < count; i++) {
        int p = particles[i];
        if (p>0){
            addToSet(&stack, p);
        } else {
            while (!isSetEmpty(stack) && topOfTheSet(stack) > 0 && topOfTheSet(stack) < -p){
                removeFromSet(&stack);
            }
            if (!isSetEmpty(stack) && topOfTheSet(stack) > 0 && topOfTheSet(stack) == -p){
                removeFromSet(&stack);
            } else if (isSetEmpty(stack) || topOfTheSet(stack) < 0){
                addToSet(&stack,p);
            }
        }
    }
    return stack;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        // fprintf(stderr, "Usage: ./survival <particle values>\n");
        // exit(1);
        exit(1);
    }

    int count = argc - 1;
    int *particles = malloc(count * sizeof(int));

    for (int i = 0; i < count; i++) {
        particles[i] = atoi(argv[i + 1]);
    }

    ParticleSet *b = fight(particles, count);
    printSet(b);
    free(particles);
    free(b);
    return 0;
}
