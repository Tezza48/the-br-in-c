SHELL=cmd

SRC = $(wildcard ./src/*.c ./src/**/*.c)

clean: ./dist/game.exe
	rm ./dist/game.exe

build:
	@echo "Starting now"
	@echo $(SRC)
	@echo "done now"

	cp ./assets/* -r ./dist
	gcc -fdiagnostics-color=always -O0 -g -std=c17 $(SRC) -I./include -L./lib -Wall -lmingw32 -lSDL2main -lSDL2 -o ./dist/game

run: build ./dist/game.exe
	./dist/game.exe