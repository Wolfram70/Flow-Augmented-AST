#Graphing the time complexity of the traversal and analysis of the FAAST

import numpy as np
import matplotlib.pyplot as plt
plt.rcParams.update({
    "text.usetex": True,
    "font.family": "serif",
    "font.serif": "Computer Modern",
})

t = np.arange(0, 10, 0.01)
B = t
W = t * np.log(t)
A = 0.9 * W
plt.plot(t, B, color='c', linewidth=1.5, label='Best-Case')
plt.plot(t, A, color='k', linewidth=1.5, label='Average-Case')
plt.plot(t, W, color='r', linewidth=1.5, label='Worst-Case')
plt.tick_params(left = False, right = False, labelleft = False, labelbottom = False, bottom = False)
plt.xlim(t[0], t[-1])
plt.xlabel('$n$')
plt.ylabel('$\mathcal{O}(n)$', rotation = 0, labelpad = 15)
plt.legend()
plt.show()