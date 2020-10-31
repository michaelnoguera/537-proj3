# See README for notes about project organization

CFLAGS=-std=c99 -Wall -pedantic
SCAN_BUILD_DIR=scan-build-out

.PHONY:clean test all scan-build scan-view

all: 537make

# build executable
537make: main.o exec.o graph.o bintree.o linkedlist.o makefileparser.o queue.o
	gcc -pthread -o 537make main.o exec.o graph.o bintree.o linkedlist.o makefileparser.o queue.o

main.o: main.c graph.h exec.h makefilerule.h makefileparser.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

exec.o: exec.c exec.h linkedlist.h graph.h makefilerule.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

graph.o: graph.c graph.h bintree.h linkedlist.h makefilerule.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

linkedlist.o: linkedlist.c linkedlist.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

makefilerule.o: makefilerule.c makefilerule.h linkedlist.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

makefileparser.o: makefileparser.c makefileparser.h makefilerule.h queue.h graph.h
ifeq ($(DEBUG),true)
	gcc -pthread -g -c -o $@ $< $(CFLAGS)
else
	gcc -pthread -c -o $@ $< $(CFLAGS)
endif

bintree.o: bintree.c bintree.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

queue.o: queue.c queue.h
ifeq ($(DEBUG),true)
	gcc -pthread -g -c -o $@ $< $(CFLAGS)
else
	gcc -pthread -c -o $@ $< $(CFLAGS)
endif

# Run test framework
test: all
	@bash test.sh

# Clean files
clean:
	rm -f *.o
	rm -f 537make

# Run the Clang Static Analyzer
scan-build: clean
	scan-build -o $(SCAN_BUILD_DIR) make

# View the one scan avaialble
scan-view: scan-build
	xdg-open $(SCAN_BUILD_DIR)/*/index.html 