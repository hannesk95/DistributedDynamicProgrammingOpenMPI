import numpy as np
import pickle
import os

for name in os.listdir(os.getcwd()):
    if os.path.isdir(os.path.join(os.getcwd(), name)):
        path = os.path.join(os.getcwd(), name)  
        
        with open(os.path.join(path,"parameters.pickle"), "rb") as the_file:
            parameters = pickle.load(the_file)
            
        np.savez(os.path.join(path,"parameters.npz"), **parameters)