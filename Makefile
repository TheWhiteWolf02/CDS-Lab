FLAGS1=-O0 -Wall -pthread
FLAGS2=-O0 -pthread -lssl -lcrypto

# CXX=icpc
CC=gcc
RM=rm -f

EXEC1=himeno
EXEC2=password_bf

all: $(EXEC1) $(EXEC2)

$(EXEC1):
	$(CC) $(FLAGS1) $(EXEC1).c -c -o $(EXEC1).o
	$(CC) $(FLAGS1) $(EXEC1).o -o $(EXEC1)

$(EXEC2):
	$(CC) $(FLAGS2) password_bf.c -c -o password_bf.o
	$(CC) password_bf.o -o $(EXEC2) $(FLAGS2)

clean:
	$(RM) $(EXEC1).o $(EXEC1)
	$(RM) $(EXEC2).o $(EXEC2)
