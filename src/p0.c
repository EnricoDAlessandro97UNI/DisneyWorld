#include <stdio.h>
#include <math.h>

#define SERVERS 4

int fact(int number) {
    int factorial = 1;
    for (int i=1; i<=number; i++) {
        factorial = factorial * i;
    }
    return factorial;
}

int main() {

    double lambda = 4.0;
    double mu = 1.5;
    double es = 0.1667;
    double ro = 0.666667;

    double sum = 0;
    double temp = 0;

    double p0 = 0.0;

    for (int i=0; i<SERVERS; i++) {
        temp = SERVERS * ro;
        temp = pow(temp, i);
        sum += temp / fact(i);
    }

    temp = pow(SERVERS*ro, SERVERS);
    temp = temp / (fact(SERVERS)*(1-ro));

    sum += (pow((SERVERS * ro), SERVERS)) / (fact(SERVERS) * (1-ro));

    p0 = 1 / sum;

    printf("\np0: %f\n\n", p0);
}
