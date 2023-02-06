# Formule KP e legge di Little per una M/M/1

import math

ES = 20
#RO = 1.66668
RO = 0.0346
def calculate_TQ():
    return ((RO*ES)/(1-RO))

def calculate_NQ():
    return (math.pow(RO,2)/(1-RO))

def calculate_TS():
    return (calculate_TQ() + ES)

def calculate_NS():
    return (RO/(1-RO))

if __name__ == '__main__':
    
    TQ = calculate_TQ()
    NQ = calculate_NQ()
    TS = calculate_TS()
    NS = calculate_NS() 

    print("\nCalcolo E(Tq), E(Nq), E(Ts) ed E(Ns) per una M/M/1 dati E(S) e ro:")

    print(f"\n  - E(Tq) = {TQ:.6f}")
    print(f"\n  - E(Nq) = {NQ:.6f}")
    print(f"\n  - E(Ts) = {TS:.6f}")
    print(f"\n  - E(Ns) = {NS:.6f}")
    
    print("\n")