## `data_debug`

mini dataset, for debugging purposes only

<br/><br/>

-------

## <a href='convert_pickle.py' target='_blank'>`convert_pickle.py`</a>

converts the relevant information of <a href='parameters.pickle' target='_blank'>`parameters.pickle`</a> into `.npy`-format. Result is <a href='parameters.npy' target='_blank'>`parameters.npy`</a>. Gets called by <a href='../../data_import/Makefile' target='_blank'>`Makefile`</a>

<br/><br/>

-------

## <a href='J_star_alpha_0_99_iter_1000.npy' target='_blank'>`J_star_alpha_0_99_iter_1000.npy`</a>

`J_star` (= values) to verify correctness of `Async_VI`

<br/><br/>

-------

## <a href='P_data.npy' target='_blank'>`P_data.npy`</a>

`data` values for `probability` matrix

<br/><br/>

-------

## <a href='P_indices.npy' target='_blank'>`P_indices.npy`</a>

`indices` for `probability` matrix

<br/><br/>

-------

## <a href='P_indptr.npy' target='_blank'>`P_indptr.npy`</a>

`index pointer` for `probability` matrix

<br/><br/>

-------

## <a href='P_shape.npy' target='_blank'>`P_shape.npy`</a>

`shape` for `probability` matrix

<br/><br/>

-------

## <a href='parameters.npy' target='_blank'>`parameters.npy`</a>

`parameters` in `.npy`-format. Content is equivalent as <a href='parameters.pickle' target='_blank'>`parameters.pickle`</a>. However only variables `number_stars`, `max_controls`, `NS`

<br/><br/>

-------

## <a href='parameters.pickle' target='_blank'>`parameters.pickle`</a>

containing all other parameters that are needed for `Value Iteration` --> `number of stars`, `fuel capacity`, `max number of actions`, `max jump range`, `confusion distance` (insecurity of decision) and `number of states`

<br/><br/>

-------

## <a href='pi_star_alpha_0_99_iter_1000.npy' target='_blank'>`pi_star_alpha_0_99_iter_1000.npy`</a>

`PI_star` (= policy) to verify correctness of `Async_VI`

<br/><br/>

-------

## <a href='star_graph_data.npy' target='_blank'>`star_graph_data.npy`</a>

`data` values for `star_graph` matrix

<br/><br/>

-------

## <a href='star_graph_indices.npy' target='_blank'>`star_graph_indices.npy`</a>

`indices` for `star_graph` matrix

<br/><br/>

-------

## <a href='star_graph_indptr.npy' target='_blank'>`star_graph_indptr.npy`</a>

`index pointer` for `star_graph` matrix

<br/><br/>

-------

## <a href='star_graph_shape.npy' target='_blank'>`star_graph_shape.npy`</a>

`shape` for `star_graph` matrix

<br/><br/>

-------

## <a href='stars.npy' target='_blank'>`stars.npy`</a>

information at which `stars` you can refuel