
CFLAGS=-Wall -O3

.PHONY: all clean

TEMPS=index2.h index.h user.*
OBJ=main.o user.o
TARGETS=main generate_resources generate_template

all: $(TARGETS)

main.o: main.c main.h index.h index2.h user.h
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

generate_resources: generate_resources.o
	$(CC) $(CFLAGS) -o $@ $^

generate_template: generate_template.o
	$(CC) $(CFLAGS) -o $@ $^

index.h: resources/index.html generate_resources
	./generate_resources $< $@

index2.h: resources/index2.html generate_resources
	./generate_resources $< $@

user.o: user.c
	$(CC) $(CFLAGS) -c $< -o $@

user.c user.h: resources/user.html generate_template
	./generate_template $< user

clean:
	rm -rf $(TARGETS) $(TEMPS) *.o
