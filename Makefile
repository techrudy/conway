LIBS = -Llib -lSDL2main -lSDL2

all:
	gcc src/main.c -o app -Iinclude ${LIBS}
	./app
	rm app