CC = g++
CFLAGS = -std=c++11 -D_DEFAULT_SOURCE -Wno-missing-braces -s -O1 -D_DEFAULT_SOURCE -I/usr/local/include -I. -I/home/game/raylib/src -I/home/game/raylib/src/external -L. -L/usr/local/lib -L/home/game/raylib/src -L/home/game/raylib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -DPLATFORM_DESKTOP

all: main.cpp character.o boss.o
	$(CC) -o game main.cpp character.o boss.o $(CFLAGS)

character.o: character.cpp
	$(CC) -o character.o -c character.cpp $(CFLAGS)

boss.o: boss.cpp
	$(CC) -o boss.o -c boss.cpp $(CFLAGS)

clean:
	rm -f $(OBJS) game
	rm -f *.o
