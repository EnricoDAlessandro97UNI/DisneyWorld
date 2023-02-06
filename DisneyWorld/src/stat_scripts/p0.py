# Program to calculate p0

import math

SERVERS = 4
LAMBDA = 4.0
MU = 1.5
ES = 0.1667
RO = 0.666667

def fact(n):
    return math.factorial(n)

if __name__ == '__main__':
    
    sum = 0
    temp  = 0
    p0 = 0.0

    for i in range(0, SERVERS):
        temp = math.pow(SERVERS*RO, i)
        sum += temp / fact(i)
    
    temp = math.pow(SERVERS*RO, SERVERS)
    temp = temp / (fact(SERVERS) * (1-RO))

    sum += math.pow(SERVERS*RO, SERVERS) / (fact(SERVERS) * (1-RO))

    p0 = 1 / sum

    print("\nCalcolo di p0 con: ")
    print(f"\n  --> lambda = {LAMBDA}");
    print(f"\n  --> servers = {SERVERS}");
    print(f"\n  --> mu = {MU}");
    print(f"\n\np0: {p0:.6f}\n\n");