
CFLAGS=-Wall -O3

.PHONY: all clean

TEMPS=index2.c index2.h index.c index.h newuser.c newuser.h user.c user.h configs.h
OBJ=main.o http.o utils.o db.o user.o index.o index2.o newuser.o
DBOBJ=dbmain.o db.o
TARGETS=dbmain main generate_resources generate_template generate_config

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

generate_config: generate_config.o
	$(CC) $(CFLAGS) -o $@ $^

#user.o: user.c
#	$(CC) $(CFLAGS) -c $< -o $@
#
#user.o: user.c
#	$(CC) $(CFLAGS) -c $< -o $@
#
#user.o: user.c
#	$(CC) $(CFLAGS) -c $< -o $@

index.c index.h: resources/index.html generate_template
	./generate_template $< index

index2.c index2.h: resources/index2.html generate_template
	./generate_template $< index2

newuser.c newuser.h: resources/newuser.html generate_template
	./generate_template $< newuser

configs.h: configs.csv generate_config
	./generate_config $<

#http.o: http.c
#	$(CC) $(CFLAGS) -c $< -o $@

user.c user.h: resources/user.html generate_template
	./generate_template $< user

clean:
	rm -rf $(TARGETS) $(TEMPS) *.o
