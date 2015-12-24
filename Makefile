
all: test_sphere

test_sphere: test_sphere.cpp
	g++ -o exec test_sphere.cpp -lm -lglut -lGLU -lsfml-window -lsfml-graphics -lsfml-system -lGLEW -lGL -lSOIL

clean:
	rm -f *.o exec
