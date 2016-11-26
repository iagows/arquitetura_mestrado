#!/bin/bash
objdump -d main.o > dump.assembly
nano dump.assembly
read -p "[Enter]"