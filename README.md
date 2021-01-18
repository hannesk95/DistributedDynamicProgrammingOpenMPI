# High Performance Computing für Maschinelle Intelligenz (C++ / Python)
### Lehrstuhl für Datenverarbeitung | Wintersemester 2020 / 2021 | Gruppe 1

---

## Inhaltsverzeichnis

- [Allgemeines zu diesem Projekt](#algemeines)
- [Repository-Struktur](#struktur)
- [Kommmunikationsprotokolle](#protokolle)
- [Anlegen eines Branches für Evaluierungsprogramm](#eval)
- [Erstellen des Hauptprogrammes](#hauptprogramm)
- [Erweitern des Projekts mit Protokollen](#erweitern)
- [Datenimport](#datenimport)
- [Evaluation verschiedener Implementierungen](#eval)
- [Programm ausführen](#ausführung)
- [Benchmark Vergleich](#vergleich)
- [OpenMPI installieren](#install)
- [Gruppen Mitglieder](#mitglieder) 
- [Anhang](#anhang)

---

## Allgemeines zu diesem Projekt

In diesem Projekt wird eine asynchrone Value Iteration unter Zuhilfenahme von OpenMPI vorgestellt. Dabei werden verschiedene Strategien implementiert und anschließend einem Benchmark Vergleich unterzogen.

---

## Repository-Struktur

- <a href="data/" target="_blank">`data/`</a> - enthält Datensätze
- <a href="lib/" target="_blank">`lib/`</a> - enthält alle benötigten Lirbraries
- <a href="results/" target="_blank">`results/`</a> - enthält die Resultate der Implementierungen
- <a href="src/" target="_blank">`src/`</a> - enthält implementierte Skripte
- <a href="CMakeLists.txt" target="_blank">`CMakeLists.txt`</a> - verantwortlich für Erstellen der `Makefile` zum Kompilieren
- <a href="Makefile" targeT="Makefile">`Makefile`</a> - verantwortlich für alle Interaktion mit dem Projekt
- <a href="hostfile" target="_blank">`hostfile`</a> - enthält alle Hosts auf denen das verteilte Rechnen laufen soll
- <a href="main.cpp" target="_blank">`main.cpp`</a> - Hauptskript, ruft Implementationen in <a href="src/" target="_blank">`src/`</a> auf und führt den Task aus

---

## Kommmunikationsprotokolle:

| Name                          | OpenMPI Funktionen                  |
| ----------------------------- | ----------------------------------- |
| `VI_Processor_Impl_Distr_01`  | `Allgather`, `Allreduce`, `Gatherv` |
| `VI_Processor_Impl_Distr_02`  | `Send`, `Receive`                   |
| `VI_Processor_Impl_Distr_04`  | `Isend`, `Ireceive`, `Igatherv`     |
| `VI_Processor_Impl_Distr_05`  | `Igather`, `Ibrcast`, `Igatherv`    |
| `VI_Processor_Impl_Distr_42`  | `Sendrecv`                          |

Umfrangreiche Dokumenation zu möglichen OpenMPI Kommunicationsprotokollen und deren Funktionen: [Princeton Bootcamp Kommunikationsprotokolle](https://princetonuniversity.github.io/PUbootcamp/sessions/parallel-programming/Intro_PP_bootcamp_2018.pdf)

---

## Anlegen eines Branches für Evaluierungsprogramm

Das Evaluierungsprogramm soll nur dazu dienen die performanteste Implementierung zu ermitteln. Für die eigentliche Value Iteration Aufgabe erstellen wir im Master-Branch eine `main.cpp` Datei welcher wir z.B. via Kommandozeilenargumenten den Ordner zu den Daten sowie den Pfad in dem die Ergebnisse gespeichert werden sollen angeben.

---

## Erstellen des Hauptprogrammes

- Einlesen des Datenordner Pfades und des Resultateordner Pfades via Kommandozeilenargument
- Optimal Values und optimal Strategy im Numpy Format im Resultateordner abspeichern, sodass es mit dem Script `data_demonstration.py` aus Hausaufgabe 2 verwendet werden kann.

---

## Erweitern des Projekts mit Protokollen

Wir können hier verschiedene VI Implementierungen und Kommunikationsstrategien ausprobieren indem einfach eine von <a href="src/vi_processor_base.h" target="_blank">` VI_Processor_Base`</a> abgeleitete Klasse implementiert wird (siehe als Beispiel <a href="src/vi_processor_impl_local.cpp" target="_blank">`VI_Processor_Impl_Local`</a> oder <a href="src/vi_processor_impl_distr_01.cpp" target="_blank">`VI_Processor_Impl_Distr_01`</a>). 



---

## Datenimport

Die Daten werden durch die <a href="Makefile" target="_blank">`Makefile`</a> während des Kompilierens mittels <a href="data/convert_pickle.py" target="_blank">`convert_pickle.py`</a> aus dem `.pickle`-Format in ein `.npz`-Format umgewandelt, welches durch die Bibliothek <a href="https://github.com/rogersce/cnpy" target="_blank">`"cnpy" by Carl Rogers`</a> eingelesen werden.

---

## Evaluation verschiedener Implementierungen

Wenn mehrere Implementierungen verglichen werden sollen wird in der <a href="main.cpp" target="_blank">`main.cpp`</a> eine Liste erstellt welche mehrere konkrete Implementierungen enthält. Dann Messen wir iterativ für jede Implementierung die Berechnungszeit (jeweils ~20 mal) und vergleichen die mittlere Ausführungsdauer und eventuell auch deren Standardabweichung.

---

## Programm ausführen

1. Melde dich über ssh auf einem HPC Rechner (Bsp: `hpc05`) an
2. Wechsele in das `LRZ-Home` Verzeichnis
3. Klone das Repository

```text
# Auf HPC Rechner anmelden
ssh hpc05

# In das LRZ-Home Verzeichnis wechseln
cd lrz-nashome

# Repository klonen
git clone https://gitlab.ldv.ei.tum.de/cpp-lk20/gruppe1.git

# Bzw. neuesten Stand pullen (wenn Projekt schon vorhanden ist)
cd gruppe1
git pull origin master

# In Master Branch wechseln
git checkout master
```

Dann kann das Projekt kompiliert und ausgeführt werden. Hierfür gibt es mehrere Möglichkeiten:

```text
1.  

1.1 # Kompilieren
    make compile

1.2 # In build Ordner wechseln
    cd build

1.3 # Ausführen (verteiltes rechnen auf mehrern PC's)
    mpirun -np 6 -hostfile ../hostfile ./MPI_Project.exe

--------------------------------------------------------

2.  # Kompilieren und ausführen 
    # (lokales rechnen auf Localhost, 2 Prozesse)
    make run_mpi_local

--------------------------------------------------------

3.  # Kompilieren und ausführen 
    # (verteiltes rechnen auf mehreren PC's)
    make run_mpi_distr
```

Hinweis: Überprüfe, ob die `~/.ssh/config` richtig ist:
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
## Benchmark Vergleich

In unten stehender Grafik ist ein Benchmark Vergleich über alle implementierten Kommunikationsprotokolle zu sehen.

<h2 align="center">
  <img src="results/benchmark_distr.png" alt="resulting barnsley fern" width="600px" />
</h2>

---

## OpenMPI installieren

Installer Dateien sind im Ordner <a href="install_files/" target="_blank">`install_files/`</a> abgelegt und müssen in nachfolgender Reihenfolge installiert werden (Linux Umgebung: Ubuntu, WSL2, etc.). Anschließend ist Version `2.2.1` installiert.
```cmd
1. cd install_files
2. sudo dpkg -i libhwloc5_1.11.9-1_amd64.deb
3. sudo dpkg -i libopenmpi2_2.1.1-8_amd64.deb
4. sudo dpkg -i openmpi-common_2.1.1-8_all.deb
5. sudo dpkg -i openmpi-bin_2.1.1-8_amd64.deb
```
---

## Gruppen Mitglieder:

- Stümke, Daniel:             daniel.stuemke@tum.de
- Christoph, Alexander:       alexander.christoph@tum.de
- Kiechle, Johannes:          johannes.kiechle@tum.de
- Gottwald, Martin (Dozent):  martin.gottwald@tum.de
- Hein, Alice (Dozentin):     alice.hein@tum.de

---

## Anhang

- OpenMPI Versionen auf den Eikon Rechnern

| HPC Rechner          | OpenMPI Version        |
| -------------------- | ---------------------- |
| `hpc01`              | `2.2.1`                |
| `hpc02`              | `4.0.3`                |
| `hpc03`              | `2.2.1`                |
| `hpc04`              | `nicht erreichbar`     |
| `hpc05`              | `2.2.1`                |
| `hpc06`              | `2.2.1`                |
| `hpc07`              | `2.2.1`                |
| `hpc08`              | `2.2.1`                |
| `hpc09`              | `2.2.1`                |
| `hpc10`              | `2.2.1`                |
| `hpc11`              | `2.2.1`                |
| `hpc12`              | `2.2.1`                |
| `hpc13`              | `2.2.1`                |
| `hpc14`              | `nicht erreichbar`     |
| `hpc15`              | `2.2.1`                |
