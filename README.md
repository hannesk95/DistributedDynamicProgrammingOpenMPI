# High Performance Computing in Python and C++, Chair of Data Processing, WS20/21, Group 1

# To Do's

## Kommmunikationsprotokolle zu evaluieren:

- AllGather + AllReduce --> VI_Processor_Impl_Distr_01
- Send + Receive (blocking)
- Send + Receive (non-blocking)

> <a href="https://princetonuniversity.github.io/PUbootcamp/sessions/parallel-programming/Intro_PP_bootcamp_2018.pdf" target="_blank">`Princeton Bootcamp Kommunikationsprotokolle`</a>

## Anlegen eines Branches für Evaluierungsprogramm

Das Evaluierungsprogramm soll nur dazu dienen die performanteste Implementierung zu ermitteln. Für die eigentliche Value Iteration Aufgabe erstellen wir im Master-Branch eine `main.cpp` Datei welcher wir z.B. via Kommandozeilenargumenten den Ordner zu den Daten sowie den Pfad in dem die Ergebnisse gespeichert werden sollen angeben.

## Erstellen des Hauptprogrammes

- Einlesen des Datenordner Pfades und des Resultateordner Pfades via Kommandozeilenargument
- Optimal Values und optimal Strategy im Numpy Format im Resultateordner abspeichern, sodass es mit dem Script `data_demonstration.py` aus Hausaufgabe 2 verwendet werden kann.

## Erweitern des Projekts mit mehr Protokollen

Wir können hier verschiedene VI Implementierungen und Kommunikationsstrategien ausprobieren indem einfach eine von <a href="src/vi_processor_base.h" target="_blank">` VI_Processor_Base`</a> abgeleitete Klasse implementiert wird (siehe als Beispiel <a href="src/vi_processor_impl_local.cpp" target="_blank">`VI_Processor_Impl_Local`</a> oder <a href="src/vi_processor_impl_distr_01.cpp" target="_blank">`VI_Processor_Impl_Distr_01`</a>). 

###########
Gruppen Mitglieder:
###########
- Stümke, Daniel:           daniel.stuemke@tum.de
- Christoph, Alexander:     alexander.christoph@tum.de
- Kiechle, Johannes:        johannes.kiechle@tum.de
- Gottwald, Martin:         martin.gottwald@tum.de
- Hein, Alice:              alice.hein@tum.de

# Repo-Struktur

- <a href="data/" target="_blank">`data/`</a> - enthält Datensätze
- <a href="lib/" target="_blank">`lib/`</a> - enthält alle benötigten Lirbraries
- <a href="src/" target="_blank">`src/`</a> - enthält implementierte Skripte
- <a href="CMakeLists.txt" target="_blank">`CMakeLists.txt`</a> - verantwortlich für Erstellen der `Makefile` zum Kompilieren
- <a href="Makefile" targeT="Makefile">`Makefile`</a> - verantwortlich für alle Interaktion mit dem Projekt
- <a href="hostfile" target="_blank">`hostfile`</a> - enthält alle Hosts auf denen das verteilte Rechnen laufen soll
- <a href="main.cpp" target="_blank">`main.cpp`</a> - Hauptskript, ruft Implementationen in <a href="src/" target="_blank">`src/`</a> auf und führt den Task aus

# Datenimport

Die Daten werden durch die <a href="Makefile" target="_blank">`Makefile`</a> während des Kompilierens mittels <a href="data/convert_pickle.py" target="_blank">`convert_pickle.py`</a> aus dem `.pickle`-Format in ein `.npz`-Format umgewandelt, welches durch <a href="https://github.com/rogersce/cnpy" target="_blank">`cnpy by Carl Rogers`</a> eingelesen werden.

# Evaluation verschiedener Implementierungen

Wenn mehrere Implementierungen verglichen werden sollen wird in der <a href="main.cpp" target="_blank">`main.cpp`</a> eine Liste erstellt welche mehrere konkrete Implementierungen enthält. Dann Messen wir iterativ für jede Implementierung die Berechnungszeit (jeweils ~20 mal) und vergleichen die mittlere Ausführungsdauer und eventuell auch deren Standardabweichung.

# Programm ausführen

Meldet euch über ssh auf `hpc05` an. Wechselt in das `LRZ-Home` Verzeichnis und clont das Repository via:

```text
# auf hpc05 wechseln
ssh hpc05

# in LRZ-Home wechseln
cd lrz-nashome

# Repo clonen
git clone https://gitlab.ldv.ei.tum.de/cpp-lk20/gruppe1.git
# bzw. neuesten Stand pullen (wenn Projekt schon vorhanden)
cd gruppe1
git pull origin master
# in Master wechseln
git checkout master
```

Dann kann das Projekt kompiliert und ausgeführt werden via:

```text
# kompilieren
make compile

# in build Ordner wechseln
cd build

# ausführen
mpirun -np 6 -hostfile ../hostfile ./MPI_Project.exe
```

Überprüft, dass die `~/.ssh/config` richtig ist:
```text
# Put this file in ~/.ssh/ with the name 'config'

# Matches hpc01 hpc02 and so on, %h gets the actual match, e.g. hpc06, and completes the host name
# A wildcard is possible (e.g. hpc*), but this disables the tab completion

Host hpc01 hpc02 hpc03 hpc04 hpc05 hpc06 hpc07 hpc08 hpc09 hpc10 hpc11 hpc12 hpc13 hpc14 hpc15
  HostName %h.clients.eikon.tum.de

# Configuration for all hosts (matches always)

Host *
  User DeinUserName
  ForwardX11 yes
  Compression yes
```

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