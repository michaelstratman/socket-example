CXXFLAGS := -g -Wall -Werror

all: socket_example

socket_example: socket_example.o

clean:
	rm -f *.o *~ socket_example
