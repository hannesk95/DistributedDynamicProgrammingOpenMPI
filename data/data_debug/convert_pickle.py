import numpy as np
import pickle


with open("parameters.pickle", "rb") as the_file:
    parameters = pickle.load(the_file)

fuel_capacity, number_stars, max_controls, max_jump_range, confusion_distance, NS = parameters.values()

parameters_array = np.array([number_stars, max_controls, NS])
np.save("parameters.npy", parameters_array)