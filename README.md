# hpc_basic
start of HPC main project

# To Do's

## Kommmunikationsprotokolle zu evaluieren:

- Scatter + Gather
- Scatter + Reduce
- Send + Recv (blockierend) + mit Status + Probe
- Send + Recv (nicht blockierend) + mit Status + Probe
- Allreduce? - Allgather? (falls es das gibt)

- <a href="https://princetonuniversity.github.io/PUbootcamp/sessions/parallel-programming/Intro_PP_bootcamp_2018.pdf" target="_blank">`Princeton Bootcamp Kommunikationsprotokolle`</a>

## Generelles

- sample code vom Martin zum Laufen bringen
- Numpy Arrays direkt in C++ einlesen - <a href="https://github.com/rogersce/cnpy" target="_blank">`cnpy by Carl Rogers`</a> -> Beispiel unter <a href="data_import/main.cpp" target="_blank">`main.cpp`</a>
- pickle direkt in C++ einlesen
- einheitliche Implementation der `Asynchronous Value Iteration` in <a href="data_import/lib/inc/Async_VI.h" target="_blank">`inc/`</a> bzw. <a href="data_import/lib/src/Async_VI.cpp" target="_blank">`src/`</a>. Implementierung muss dort dementsprechend geändert/angepasst werden. Aufruf/Testing bereits in <a href="data_import/main.cpp" target="_blank">`main.cpp`</a> enthalten

# Implementiertes

- <a href="data/" target="_blank">`data/`</a>
- <a href="data_import/" target="_blank">`data_import/`</a>