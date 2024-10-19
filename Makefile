CC=gcc
SOURCE=main.c
OBJ=$(SOURCE:.c=.o)
EXE=bush

all: $(EXE) clean execute

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $@

.o: .c
	$(CC) -c $< -o $@

clean:
	rm -rf $(OBJ)

execute:
	./$(EXE)

execute_with_params:
	./$(EXE) script.sh
