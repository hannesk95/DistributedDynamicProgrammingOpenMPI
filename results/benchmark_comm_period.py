import os
import glob
import numpy as np
import matplotlib.pyplot as plt
import argparse
import re

def atoi(text):
    return int(text) if text.isdigit() else text

def natural_keys(text):
    '''
    alist.sort(key=natural_keys) sorts in human order
    http://nedbatchelder.com/blog/200712/human_sorting.html
    (See Toothy's implementation in the comments)
    '''
    return [ atoi(c) for c in re.split(r'(\d+)', text) ]

def load_data(path):

    database = {}
    filenames = sorted(glob.glob(os.path.join(path, "*.npz")), key = natural_keys)
    
    for file in filenames:
        data = np.load(file)
        key = file.split("/")[-1]
        key = key.split(".")[0]
        key = key.split("\\")[-1]
        key = key.split("_")[-1]
        database[key] = {}

        for count, properties in enumerate(data.files):
            value = data[properties].astype(float)
            database[key][properties] = value

    return database

def autolabel(ax, rects):
    for rect in rects:
        height = rect.get_height()
        ax.annotate('{:.4f}'.format(height),
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3),
                    textcoords="offset points",
                    ha='center', va='bottom', fontsize=8)

def plot_mean_exec_time(mean_times, x, width, path, comm):

    fig, ax = plt.subplots(figsize=(16,8))
    plt.rcParams['xtick.labelsize']=16
    plt.rcParams['ytick.labelsize']=16

    rects = []
    for i in range(mean_times.keys().__len__()):
        rects.append(ax.bar(x + (i * 0.12 - 0.24), mean_times[list(mean_times.keys())[i]].values(), width, label='Scheme: ' + list(mean_times.keys())[i]))
    
    ax.set_ylabel('Mean execution time (in seconds)', fontsize=16)
    ax.set_title('Benchmark comparison among communication schemes', fontsize=20)
    ax.set_xticks(x)
    ax.set_xticklabels(comm)
    ax.set_xlabel('Communication frequency (in epochs)', fontsize=16)
    ax.legend(fontsize='x-large')    
    
    for rect in rects:
        autolabel(ax, rect)
    
    fig.tight_layout()
    plt.grid()
    plt.savefig(os.path.join(path, 'benchmark_distr.png'))
    print("[INFO] The benchmark visualization plot was successfully stored to: " + path)

def plot_var_exec_time(var_times, x, width, path, comm):

    fig, ax = plt.subplots(figsize=(16,8))
    plt.rcParams['xtick.labelsize']=16
    plt.rcParams['ytick.labelsize']=16

    rects = []
    for i in range(var_times.keys().__len__()):
        rects.append(ax.bar(x + (i * 0.12 - 0.24), var_times[list(var_times.keys())[i]].values(), width, label='Scheme: ' + list(var_times.keys())[i]))
    
    ax.set_ylabel('Variance of execution time (in milliseconds)', fontsize=16)
    ax.set_title('Variance execution time comparison among communication schemes', fontsize=20)
    ax.set_xticks(x)
    ax.set_xticklabels(comm)
    ax.set_xlabel('Communication frequency (in epochs)', fontsize=16)
    ax.legend(fontsize='x-large')    
    
    for rect in rects:
        autolabel(ax, rect)
    
    fig.tight_layout()
    plt.grid()
    plt.savefig(os.path.join(path, 'var_distr.png'))
    print("[INFO] The variance visualization plot was successfully stored to: " + path)

        
def visualize(path, data):

    mean_times = {}
    var_times = {}
    comm = []
    labels = []
    
    for label in list(data.keys()):
        result = label.split("-")
        
        
        if any(x in label for x in ["Local", "local"]):
            continue
        else:                
            comm.append(result[-1])
            mean_times[result[0]] = {}
            var_times[result[0]] = {}
    
    comm = np.array(comm)
    comm = sorted(np.unique(comm), key = natural_keys)
    labels = list(mean_times.keys())    
    
    for label in list(data.keys()):
        scheme = label.split("-")[0]
        freq = label.split("-")[-1]
        
        if(scheme == freq):
            continue
        
        mean_times[scheme][freq] = data[label]['mean_execution_time'][0]
        var_times[scheme][freq] = data[label]['var_execution_time'][0] * 1000
        
    x = np.arange(len(comm))
    width = 0.1

    plot_mean_exec_time(mean_times, x, width, path, comm)
    plot_var_exec_time(var_times, x, width, path, comm)
    
    
    
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("path", type=str)
    args = parser.parse_args()    

    result_dir = os.path.join(os.getcwd(), args.path)
 
    data = load_data(result_dir)
    visualize(result_dir, data)
    
if __name__ == "__main__":
    main()