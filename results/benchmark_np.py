import os
import sys
import glob
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import re

def load_data(path):

    database = {}
    filenames = glob.glob(os.path.join(path, "*.npz"))
    
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

def extend_database(folder, database):
    dataset_name = ""
    m = re.search(r'/(.+?)-(\d+)/$', folder)
    dataset_name = m.group(1)
    np = int(m.group(2))
    dataset = load_data(folder)
    if bool(dataset):
        database = [
            *database,
            {
                'dataset': dataset_name,
                'np': 1,
                'impl_name': "Local",
                'exec_time': dataset["Local"]["mean_execution_time"][0]
            }
        ]
        del dataset["Local"]
        (best_impl_name,best_impl) = min(dataset.items(), key=lambda x: x[1]['mean_execution_time'][0])
        database = [ 
            *database,
            {
                'dataset': dataset_name,
                'np': np, 
                'impl_name': best_impl_name,
                'exec_time': best_impl['mean_execution_time'][0]
            }
        ]
    return database

def main():
    parent_folder = sys.argv[1]
    child_folders = glob.glob(os.path.join(parent_folder, "*/"))
    database = []
    for child_folder in child_folders:
        database = extend_database(child_folder, database)

    dataframe = pd.DataFrame(database)
    dataframe['exec_time_us_log'] = np.log2(dataframe['exec_time'] * 1e9)
    plt.figure()
    g = sns.catplot(
        data=dataframe, kind="bar",
        x="dataset", y="exec_time_us_log", hue="np",
        order=['data_debug','data_small','data_normal'],
        palette="dark", alpha=.6, height=6,
        legend_out=True
    )
    g.set_axis_labels("", "log2 of mean execution time (us)")
    g.legend.set_title("Number of processes")
    g._legend.set_bbox_to_anchor((1.2, .7))
    g.savefig(os.path.join(parent_folder,"benchmark_np.png"))

    dataset_names = dataframe['dataset'].unique()
    dataframe['speedup'] = [0.0] * len(dataframe)
    for dataset_name in dataset_names:
        dataset_local = dataframe[(dataframe['dataset'] == dataset_name) & (dataframe['np'] == 1)]
        min_local_exec_time = dataset_local['exec_time'].min()
        dataset_copy = dataframe[dataframe['dataset'] == dataset_name].copy()
        dataset_copy['speedup'] = (dataset_copy['exec_time'] / min_local_exec_time)**-1
        dataframe[dataframe['dataset'] == dataset_name] = dataset_copy

    dataframe_wo_local = dataframe[dataframe['np'] != 1]
    plt.figure()
    g = sns.lineplot(
        x='np', y='speedup', hue='dataset', hue_order=['data_debug','data_small','data_normal'],
        data=dataframe_wo_local
    ).get_figure()
    g.savefig(os.path.join(parent_folder,"benchmark_np_speedup.png"))

        
    
    
if __name__ == "__main__":
    main()