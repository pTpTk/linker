all: $(wildcard *.cpp) $(wildcard *.h)
	g++ -g $(wildcard *.cpp) -o link

debug:
	g++ -DDEBUG -g $(wildcard *.cpp) -o link

test:
	./link crt.o test.o output.x