# `data`

contains all datasets that are provided with the repository

<br/><br/>

-------

## <a href='data_debug/' target='_blank'>`data_debug/`</a>

mini dataset, for debugging purposes only

<br/><br/>

-------

## <a href='data_small/' target='_blank'>`data_small/`</a>

small dataset, contains much more data. For evaluation purposes

<br/><br/>

-------

## <a href='convert_pickle.py' target='_blank'>`convert_pickle.py`</a>

converts the relevant information of `parameters.pickle` into `.npz`-format. Result is `parameters.npz` in respective data folder. Gets called by <a href='../Makefile' target='_blank'>`Makefile`</a> and converts the `parameters.pickle` files for all available data folders.