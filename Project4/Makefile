CC=gcc
BUILD_DIR=build/
NAME=simplefs
TEST=test

all: libsimplefs app

libsimplefs: build_dir
	$(CC) -Wall -c -std=c99 simplefs.c -o simplefs.o
	ar -cvq libsimplefs.a simplefs.o
	ranlib libsimplefs.a

app: build_dir app.c
	$(CC) -o $(BUILD_DIR)app app.c $(NAME).c -std=c99 -lpthread -L. -lsimplefs

test: build_dir test.c
	$(CC) -o $(BUILD_DIR)$(TEST) $(TEST).c $(NAME).c -std=c99 -lpthread -L. -lsimplefs
	$(BUILD_DIR)$(TEST)
	rm $(BUILD_DIR)$(TEST)

build_dir:
	if [ ! -d $(BUILD_DIR) ]; then \
		mkdir $(BUILD_DIR); \
	fi

clean:
	rm -rf $(BUILD_DIR)
	rm -f simplefs.o libsimplefs.a

rebuild: clean all test