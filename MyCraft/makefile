PLATFORM_FLAG = -D APPLE

all:
	g++ -Wall -c -o Block.o Block.cpp $(PLATFORM_FLAG)
	g++ -Wall -c -o common.o common.cpp $(PLATFORM_FLAG)
	g++ -Wall -c -o Render.o Render.cpp $(PLATFORM_FLAG)
	g++ -Wall -c -o texture.o texture.cpp $(PLATFORM_FLAG)
	g++ -Wall -c -o Player.o Player.cpp $(PLATFORM_FLAG)
	g++ -Wall -c -o World.o World.cpp $(PLATFORM_FLAG)
	g++ -Wall -c -o Map.o Map.cpp $(PLATFORM_FLAG)
	g++ -Wall -c -o File.o File.cpp $(PLATFORM_FLAG)
	g++ -Wall main.cpp Block.o Render.o common.o texture.o Player.o World.o Map.o File.o -o main -framework OpenGL -framework GLUT -framework CoreFoundation -L./ -lSOIL $(PLATFORM_FLAG)

clear:
	rm -f *.o *~
