# Miprof.Shell

Codigo de ejecucion

g++ -std=c++17 -Wall -o mishell main.cpp Shell.cpp Miprof.cpp Utils.cpp
Luego se ejecuta
./mishell

Comandos:
Puedes ejecutar cualquier comando del sistema, igual que en bash

Ejemplos de los demas comandos
Comando exit: 
mishell:$ exit

Comandos con pipes: 
mishell:$ ps aux | sort -nr -k 4 | head -5
mishell:$ cat archivo.txt | grep error | wc -l

Comando personalizados miprof:

Ejecuta el comando, muestra métricas en pantalla:
mishell:$ miprof ejec ls -l

Ejecuta el comando y guarda los resultados en un archivo (si existe, agrega al final):
mishell:$ miprof ejecsave resultados.txt ls -l

Ejecuta el comando con un tiempo máximo de N segundos.
Si se excede, lo mata automáticamente:
mishell:$ miprof maxtiempo 3 sleep 10

