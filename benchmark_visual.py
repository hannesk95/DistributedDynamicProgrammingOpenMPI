import os
import glob
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns


def load_data(path):

    database = {}
    filenames = glob.glob(os.path.join(path, "*.npz"))

    for file in filenames:
        data = np.load(file)
        key = file.split("/")[-1]
        database[key] = {}

        for count, properties in enumerate(data.files):
            value = data[properties].astype(float)
            database[key][properties] = value

    return database

def plot_benchmark(data):

    #Visualization goes here





def main():

    result_dir = "results"

    data = load_data(os.path.join(os.getcwd(), result_dir))

    plot_benchmark(data['26VI_Processor_Impl_Distr_42-100.npz'])


if __name__ == "__main__":
    main()

