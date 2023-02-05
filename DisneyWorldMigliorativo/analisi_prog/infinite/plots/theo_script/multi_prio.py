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
			l1 = eval(row[1])
			l2 = eval(row[2])
			p1 = eval(row[3])
			p2 = eval(row[4])
			mu = eval(row[5])
			esi = eval(row[6])
			es = esi / m
			r1 = l1/(m*mu)
			r2 = l2/(m*mu)
			r = (l1+l2)/(m*mu)
			print("rho1: ", r1)
			print("rho2: ", r2)
			print("rho: ", r)
			print("es: ", es)
			
			
			
			etq1 = (pQueue(m, r)*es)/(1 - r1)
			ets1 = etq1 + esi
			
			etq2 = (pQueue(m, r)*es)/((1-r1)*(1 - r))
			ets2 = etq2 + esi

			etq = p1*etq1 + p2*etq2

			etsg = p1*etq1 + p2*etq2 + esi

			print('-'*40)
			print('E(Tq1) = {0} E(Ts1) = {1}'.format(etq1, ets1))
			print('E(Nq1) = {0} E(Ns1) = {1}'.format(etq1*l1, ets1*l1))
			print('-'*40)
			print('E(Tq2) = {0} E(Ts2) = {1}'.format(etq2, ets2))                        
			print('E(Nq2) = {0} E(Ns2) = {1}'.format(etq2*l2, ets2*l2))
			print('-'*40)
			print('E(Tq) = {0}'.format(etq))
		print("servers: ", servers)
		print('='*40)
		print('E(Tsg) = {0}'.format(etsg))
		print('='*40)

if __name__ == '__main__':
	print("FASCIA 1")
	calc("multi_values_f1.txt")
	print("FASCIA 2")
	calc("multi_values_f2.txt")
	
	
