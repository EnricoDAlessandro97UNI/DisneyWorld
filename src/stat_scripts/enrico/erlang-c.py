# Formula Erlang-C e legge di Little per una M/M/m

import math
#BLOCK 1
#SERVERS = 20
#ESI = 120
#ES = ESI/SERVERS
#RO = 0.992071428571428

#BLOCK 3
#SERVERS = 5
#ESI = 10
#ES = ESI/SERVERS
#RO = 0.01384

#BLOCK 4
SERVERS = 7
ESI = 60
ES = ESI/SERVERS
RO = 0.021599

def fact(n):
    return math.factorial(n)

def calculate_p0():

    sum = 0
    temp  = 0
    p0 = 0.0

    for i in range(0, SERVERS):
        temp = math.pow(SERVERS*RO, i)
        sum += temp / fact(i)
    
    temp = math.pow(SERVERS*RO, SERVERS)
    temp = temp / (fact(SERVERS) * (1-RO))

    sum += temp

    p0 = 1 / sum

    return p0

def calculate_PQ():
    return ((math.pow(SERVERS*RO,SERVERS)/(fact(SERVERS)*(1-RO))) * calculate_p0())

def calculate_TQ():
    return ((calculate_PQ()*ES) / (1-RO))

def calculate_NQ():
    return (calculate_PQ() * (RO/(1-RO)))

def calculate_TS():
    return (calculate_TQ() + SERVERS*ES)

def calculate_NS():
    return (calculate_PQ() * (RO/(1-RO)) + SERVERS*RO)

if __name__ == '__main__':
    
    p0 = calculate_p0()
    PQ = calculate_PQ()
    TQ = calculate_TQ()
    NQ = calculate_NQ()
    TS = calculate_TS()
    NS = calculate_NS() 

    print("\nCalcolo E(Tq), E(Nq), E(Ts) ed E(Ns) per una M/M/m dati E(S) e ro:")

    print(f"\n  - p0 = {p0:}")
    print(f"\n  - PQ = {PQ:.6f}")
    print(f"\n  - E(Tq) = {TQ:.6f}")
    print(f"\n  - E(Nq) = {NQ:.6f}")
    print(f"\n  - E(Ts) = {TS:.6f}")
    print(f"\n  - E(Ns) = {NS:.6f}")
    
    print("\n")