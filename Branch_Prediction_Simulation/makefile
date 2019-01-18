P = driver
Q = BranchPredictor
CFLAGS = -Wall -Wextra -DDEBUG -g -pedantic -std=c++14

run: all
	time ./$(P) traces/short_trace1.txt

all: $(P)

debug: $(P)
	sudo gdb --args driver inputs/sample_2.txt 0101110000

$(Q).o: $(Q).cpp $(Q).h
	g++ $(CFLAGS) -c $(Q).cpp -o $(Q).o

$(P).o: $(P).cpp
	g++ $(CFLAGS) -c $(P).cpp -o $(P).o

$(P): $(Q).o $(P).o
	g++ $(CFLAGS) $(Q).o $(P).o -o driver

checkmem: all
	valgrind --tool=memcheck --leak-check=yes --num-callers=20 --track-fds=yes -v ./$(P) inputs/sample_2.txt 113

clean:
	rm -rf $(P) *.o
	rm -rf output.txt
