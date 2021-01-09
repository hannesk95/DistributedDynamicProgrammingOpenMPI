# High Performance Computing in Python and C++, Chair of Data Processing, WS20/21, Group 1

###########
Gruppen Mitglieder:
###########
- Stümke, Daniel: 			daniel.stuemke@tum.de
- Christoph, Alexander:     alexander.christoph@tum.de
- Kiechle, Johannes: 		johannes.kiechle@tum.de
- Mustermann, Max:

# To Do's

## Dienstag 12.01

- einigen auf eine Async_VI Implementierung (preferable Daniels)
- Ordnerstruktur absprechen
- Absprache wer welches Kommunikationsprotokoll testet
- Rücksprache GIT

## Kommmunikationsprotokolle zu evaluieren:

- Scatter + Gather
- Scatter + Reduce
- Send + Recv (blockierend) + mit Status + Probe
- Send + Recv (nicht blockierend) + mit Status + Probe
- Allreduce? - Allgather? (falls es das gibt)

- <a href="https://princetonuniversity.github.io/PUbootcamp/sessions/parallel-programming/Intro_PP_bootcamp_2018.pdf" target="_blank">`Princeton Bootcamp Kommunikationsprotokolle`</a>

## Generelles

- sample code vom Martin zum Laufen bringen (siehe `OpenMPI installieren`)
- Numpy Arrays direkt in C++ einlesen - <a href="https://github.com/rogersce/cnpy" target="_blank">`cnpy by Carl Rogers`</a> -> Beispiel unter <a href="data_import/main.cpp" target="_blank">`main.cpp`</a>
- pickle direkt in C++ einlesen
- einheitliche Implementation der `Asynchronous Value Iteration` in <a href="data_import/lib/inc/Async_VI.h" target="_blank">`inc/`</a> bzw. <a href="data_import/lib/src/Async_VI.cpp" target="_blank">`src/`</a>. Implementierung muss dort dementsprechend geändert/angepasst werden. Aufruf/Testing bereits in <a href="data_import/main.cpp" target="_blank">`main.cpp`</a> enthalten

# OpenMPI installieren

- installer Dateien sind im Ordner <a href="install_files/" target="_blank">`install_files/`</a> abgelegt und müssen in der richtigen Reihenfolge installiert werden (auf der WSL). Danach ist Version `2.2.1` installiert
```cmd
cd install_files
sudo dpkg -i libhwloc5_1.11.9-1_amd64.deb
sudo dpkg -i libopenmpi2_2.1.1-8_amd64.deb
sudo dpkg -i openmpi-common_2.1.1-8_all.deb
sudo dpkg -i openmpi-bin_2.1.1-8_amd64.deb
```

- Versionen auf den Eikon Rechnern
    - `hpc01` - 2.2.1
    - `hpc02` - 4.0.3
    - `hpc03` - 2.2.1
    - `hpc04` - nicht erreichbar
    - `hpc05` - 2.2.1
    - `hpc06` - 2.2.1
    - `hpc07` - 2.2.1
    - `hpc08` - 2.2.1
    - `hpc09` - 2.2.1
    - `hpc10` - 2.2.1
    - `hpc11` - 2.2.1
    - `hpc12` - 2.2.1
    - `hpc13` - 2.2.1
    - `hpc14` - nicht erreichbar
    - `hpc15` - 2.2.1

# Implementiertes

- <a href="data/" target="_blank">`data/`</a>
- <a href="data_import/" target="_blank">`data_import/`</a>
