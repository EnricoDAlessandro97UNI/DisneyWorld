
# print(prev_mu)
l =  1.7*10**(-3)
print('lambda = ', l)
mu = 0.05
es = 20.0
r = l/mu
print('rho = ', r)

tq =  (r*es)/(1-r)
ts = tq + es
print('E(Tq) = {0} E(Ts) = {1}'.format(tq, ts))
