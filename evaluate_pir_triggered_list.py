# -*- coding: utf-8 -*-
"""
Created on Sun May 16 13:18:01 2021

@author: Martin
"""
import numpy as np
import matplotlib.pyplot as plt

ls = []
counter = 0
with open('pir_triggered.txt', 'r') as f:
    for line in f:
        counter += 1
        ls.append(line.rstrip()[-1])


ls = np.asarray(ls)
fix, ax = plt.subplots()
ax.hist(np.sort(ls))
ax.set_xlabel("PIR-Modules")
ax.set_ylabel("# triggered")