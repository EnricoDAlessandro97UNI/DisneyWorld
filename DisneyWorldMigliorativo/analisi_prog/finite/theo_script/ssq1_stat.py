
# print(prev_mu)
l =  0.023148
print('lambda = ', l)
mu = 0.05
es = 20.0
r = l/mu
print('rho = ', r)

tq =  (r*es)/(1-r)
ts = tq + es
print('E(Tq) = {0} E(Ts) = {1}'.format(tq, ts))
print('E(Nq) = {0} E(Ns) = {1}'.format(tq*l, ts*l))
