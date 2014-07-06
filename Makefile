
CFLAGS=-Wall -O3

.PHONY: all clean

TEMPS=index2.h index.h newuser.h user.c user.h
OBJ=main.o user.o http.o utils.o db.o
DBOBJ=dbmain.o db.o
TARGETS=dbmain main generate_resources generate_template

all: $(TARGETS)

main.o: main.c $(TEMPS)
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

dbmain: $(DBOBJ)
	$(CC) $(CFLAGS) -o $@ $^

generate_resources: generate_resources.o
	$(CC) $(CFLAGS) -o $@ $^

generate_template: generate_template.o
	$(CC) $(CFLAGS) -o $@ $^

index.h: resources/index.html generate_resources
	./generate_resources $< $@

index2.h: resources/index2.html generate_resources
	./generate_resources $< $@

newuser.h: resources/newuser.html generate_resources
	./generate_resources $< $@

user.o: user.c
	$(CC) $(CFLAGS) -c $< -o $@

#http.o: http.c
#	$(CC) $(CFLAGS) -c $< -o $@

user.c user.h: resources/user.html generate_template
	./generate_template $< user

clean:
	rm -rf $(TARGETS) $(TEMPS) *.o
