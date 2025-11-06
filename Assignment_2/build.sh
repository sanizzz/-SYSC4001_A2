if [ ! -d "bin" ]; then
    mkdir bin
else
	rm bin/*
fi
if [ ! -d "output_files" ]; then
    mkdir output_files
fi
g++ -std=c++17 -g -O0 -I . -o bin/interrupts interrupts_101307214_101306172.cpp