CC=gcc
EXEC1=findtopk
EXEC2=findtopk_m
EXEC3=findtopk_t

compile: compile1 compile2 compile3

compile1: build-dir
	$(CC) -Wall -o build/$(EXEC1).out findtopk.c

compile2: build-dir
	$(CC) -Wall -o build/$(EXEC2).out findtopk_mqueue.c -lrt

compile3: build-dir
	$(CC) -Wall -o build/$(EXEC3).out findtopk_thread.c -lpthread

build-dir:
	if [ ! -d ./build/ ]; then \
		mkdir build; \
	fi
	if [ ! -d ./outputs/ ]; then \
		mkdir outputs; \
	fi

test: test1 test2 test3

test1: compile1
	build/$(EXEC1).out 4 2 inputs/input1.txt inputs/input2.txt outputs/out1.txt

test2: compile2
	build/$(EXEC2).out 4 2 inputs/input1.txt inputs/input2.txt outputs/out2.txt

test3: compile3
	build/$(EXEC3).out 4 2 inputs/input1.txt inputs/input2.txt outputs/out3.txt

measure: measure1 measure2 measure3

measure1: compile1
	time build/$(EXEC1).out 1000 3 inputs/rand1.txt inputs/rand2.txt inputs/rand3.txt outputs/out1.txt

measure2: compile2
	time build/$(EXEC2).out 1000 3 inputs/rand1.txt inputs/rand2.txt inputs/rand3.txt outputs/out2.txt

measure3: compile3
	time build/$(EXEC3).out 1000 3 inputs/rand1.txt inputs/rand2.txt inputs/rand3.txt outputs/out3.txt

rebuild: clean compile

clean:
	rm -rf build/