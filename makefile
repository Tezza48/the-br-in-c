SHELL=cmd

SRC = $(wildcard ./src/*.c ./src/**/*.c)

build:
	cp ./assets/* -r ./dist
	gcc -fdiagnostics-color=always -O0 -g -std=c17 $(SRC) -I./include -L./lib -Wall -lmingw32 -lSDL2main -lSDL2 -o ./dist/game -include ./src/settings.h

clean: ./dist/game.exe
	rm ./dist/game.exe

run: build ./dist/game.exe
	./dist/game.exe