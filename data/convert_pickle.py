import numpy as np
import pickle
import os

for name in os.listdir(os.getcwd()):
    if os.path.isdir(os.path.join(cwd, name)):
        path = os.path.join(cwd, name)  
        
        with open(path + "\parameters.pickle", "rb") as the_file:
            parameters = pickle.load(the_file)
            
        np.savez(path + "\parameters.npz", **parameters)
        