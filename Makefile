CC = g++
CFLAGS = -Wall -std=c++11 -D_DEFAULT_SOURCE -Wno-missing-braces -s -O1 -D_DEFAULT_SOURCE -I/usr/local/include -I. -I${HOME}/raylib/src -I${HOME}/raylib/src/external -L. -L/usr/local/lib -L${HOME}/raylib/src -L${HOME}/raylib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -DPLATFORM_DESKTOP

game: main.cpp
	$(CC) -o $@ $^ $(CFLAGS)
clean:
	rm -f $(OBJS) game
