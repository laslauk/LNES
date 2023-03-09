CC = g++ -std=c++14
ODIR = ./build
IDIR += ./LNES_EMU_HW/Include
IDIR += ./libraries/include
IDIR += ./libraries/lib/x64
IDIR += /usr/include/SDL2
IDIR += /usr/lib/x86_64-linux-gnu/
OBJ_ = $(wildcard $(ODIR)/*.o)

all: main memory cpu_6502 bus stack PPU CARTRIDGE lnesrenderer
	$(CC) $(OBJ_) -o program.a -Wall   -lSDL2 -lSDL2_image

main: memory cpu_6502 bus PPU CARTRIDGE PPU lnesrenderer
	$(CC) -c ./app/main.cpp -o ./$(ODIR)/main.o -I$(IDIR) -Wall 


memory: ./LNES_EMU_HW/Include/memory.hh bus 
	$(CC) -c ./LNES_EMU_HW/memory.cpp -o ./$(ODIR)/memory.o -I$(IDIR) -Wall


cpu_6502: ./LNES_EMU_HW/Include/cpu_6502.hh bus stack 
	$(CC) -c ./LNES_EMU_HW/cpu_6502.cpp -o ./$(ODIR)/cpu_6502.o -I$(IDIR) -Wall


bus: ./LNES_EMU_HW/Include/bus.hh
	$(CC) -c ./LNES_EMU_HW/bus.cpp -o ./$(ODIR)/bus.o -I$(IDIR) -Wall

stack:  ./LNES_EMU_HW/Include/stack.hh
	$(CC) -c ./LNES_EMU_HW/stack.cpp -o ./$(ODIR)/stack.o -I$(IDIR) -Wall

PPU:  ./LNES_EMU_HW/Include/PPU.hh
	$(CC) -c ./LNES_EMU_HW/PPU.cpp -o ./$(ODIR)/PPU.o -I$(IDIR) -Wall


CARTRIDGE:  ./LNES_EMU_HW/Include/cartridge.hh
	$(CC) -c ./LNES_EMU_HW/cartridge.cpp -o ./$(ODIR)/cartridge.o -I$(IDIR) -Wall


lnesrenderer:  ./LNES_EMU_HW/Include/lnesrenderer.hh
	$(CC) -c ./LNES_EMU_HW/lnes_renderer.cpp -o ./$(ODIR)/lnesrenderer.o -I$(IDIR) -Wall


.PHONY: clean
clean: 
	rm ./build/*
	rm program.a