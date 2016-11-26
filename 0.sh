#!/bin/bash
clear
read -p "LISTANDO OS NÚCLEOS [Enter]"
cat /proc/cpuinfo
read -p "RESUMO DOS NÚCLEOS [Enter]"
lscpu
read -p "Topologia do computador [Enter]"
lstopo
read -p "fim"