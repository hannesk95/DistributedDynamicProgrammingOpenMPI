# High Performance Computing in Python and C++, Chair of Data Processing, WS20/21, Group 1

###########
Group members:
###########
- Stümke, Daniel: 			daniel.stuemke@tum.de
- Christoph, Alexander:     alexander.christoph@tum.de
- Kiechle, Johannes: 		johannes.kiechle@tum.de

# Daniels Branch

Wir können hier verschiedene VI Implementierungen und Kommunikationsstrategien ausprobieren indem einfach eine von `VI_Processor_Base` abgeleitete Klasse implementiert wird (siehe als Beispiel `VI_Processor_Impl_Local` oder `VI_Processor_Impl_Distr_01`). 

# Datenimport

Die Daten werden im Numpy Format eingelesen. Dafür müssen die `parameters.pickle` Dateien in `parameters.npz` konvertiert werden. Dafür ließt man das Parameter-Dictionary ein und speichert es dann wie folgt ab `np.savez('/some/path/parameters.npz', **parameters)`.

# Idee zur Evaluation verschiedener Implementierungen

Wenn mehrere Implementierungen verglichen werden sollen können wir in der `main` Funktion eine Liste erstellen welche mehrere Konkrete Implementierungen enthält. Dann Messen wir iterativ für jede Implementierung die Berechnungszeit (jeweils ~20 mal) und vergleichen die mittlere Ausführungsdauer und deren Standardabweichung.

