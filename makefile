CC = g++ -std=c++14
ODIR = ./build
IDIR += ./LNES_EMU_HW/Include
OBJ_ = $(wildcard $(ODIR)/*.o)

all: main
	$(CC) $(OBJ_) -o program.a

main:
	$(CC) -c ./app/main.cpp -o ./$(ODIR)/main.o -I$(IDIR)


.PHONY: clean
clean: 
	rm ./build/*
	rm program.exe