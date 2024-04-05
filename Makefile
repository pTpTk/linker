all: $(wildcard *.cpp) $(wildcard *.h)
	g++ -g $(wildcard *.cpp) -o link

debug:
	g++ -DDEBUG -g $(wildcard *.cpp) -o link

test:
	# ./link ../assembler/output.o output.x
	./link ../test/main.o output.x