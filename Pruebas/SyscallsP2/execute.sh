#!/bin/bash

echo "Ingrese el número de proceso que desea ejecutar:"
read proceso

echo "$proceso" | sudo tee /proc/luis_mem_process > /dev/null

echo "Mostrando el contenido de /proc/luis_mem_process en tiempo real. Presione Ctrl+C para salir."
watch -n 1 sudo cat /proc/luis_mem_process

