import math


def pzero(m, r):
    # calculate p(0)
    summ = 0
    for i in range(0 , m):
        summ += ((m*r)**i)/math.factorial(i)

    p0 = (summ + ((m * r)**m) / (math.factorial(m)*(1 - r)))**(-1)
    print("p0 : ", p0)

    return p0


def pQueue(m, r):
    pq = (((m * r)**m) / (math.factorial(m)*(1 - r))) * pzero(m, r)
    print("pq : ", pq)
    return pq


def calc(fname):
	etsg = 0
	servers = []
	with open(fname, "r") as fd:
		fd.readline() # consuma la riga intestazione
		for line in fd:
			row = line.split(' ')

			m = eval(row[0])
			servers.append(m)
			l = eval(row[1])
			mu = eval(row[2])
			esi = eval(row[3])
			es = esi / m
			r = l/(m*mu)
			print("rho: ", r)
			print("es: ", es)

			etq = (pQueue(m, r)*es)/(1 - r)
			ets = etq + esi

			etsg = etsg + ets

			print('E(Tq) = {0} E(Ts) = {1}'.format(etq, ets))            
			print('E(Nq) = {0} E(Ns) = {1}'.format(etq*l, ets*l))
			print('-'*40)
		print("servers: ", servers)
		print('='*40)
		print('E(Tsg) = {0}'.format(etsg))
		print('='*40)

if __name__ == '__main__':
	print("FASCIA 1")
	calc("msq_values_f1.txt")
	print("\n\n\n")
	print("FASCIA 2")
	calc("msq_values_f2.txt")
	
