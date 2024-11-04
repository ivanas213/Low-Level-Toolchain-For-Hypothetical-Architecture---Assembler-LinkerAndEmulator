vscExecutable: main.cpp
	gcc -fdiagnostics-color=always -g -o ${@} $(^)
	