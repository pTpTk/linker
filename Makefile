all: $(wildcard *.cpp) $(wildcard *.h)
	g++ -g $(wildcard *.cpp) -o link

debug:
	g++ -DDEBUG -g $(wildcard *.cpp) -o link

output:
	# ./link crt.o test.o output.x
	./link crt.o ../test/main.o -ltest output.x

test: output
	chmod +x output.x
	./output.x