CC = gcc

# THIS IS COMMENT
INC = .
FLAGS = -Wall
OUTPUT = -o
STATIC = -static
RM = rm -f
#----------------------------------------------

all: project

project: interpreter.o
	$(CC) interpreter.o $(OUTPUT) project


interpreter.o: interpreter.h interpreter.c
	$(CC) $(FLAGS) -I$(INC) -c interpreter.c

clean: 
	-@$(RM) *.o project 
run:
	@echo "Running program ..."; \
	./project

