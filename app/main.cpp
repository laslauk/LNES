/*
    Lasse NES emulator LNES 2022
    remember to clone submodules

*/



/*

    platform:

    APP:-----------

    [CPU] [MEMORY]
    ^           ^
    |           |
    |           |
    v---[BUS]---v

    ---------------

*/

#include <fstream>
#include "memory.hh"
#include "cpu_6502.hh"
#include "bus.hh"
#include <assert.h>
#include <iostream>
#include <bitset>
#include "PPU.hh"
#include "cartridge.hh"
#include "LNES_utilities.hh"
#include "Frame.hh"
#include "lnesrenderer.hh"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"


const std::string path1 = "./sample_640×426.bmp";//"./sample_640×426.bmp";
const std::string path2 = "./cat.png";
const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 240;




/*
 std::vector< std::vector<std::vector<Color>> > getAllTiles( const std::vector<uint8_t> &CHR_ROM) {



 std::vector< std::vector<std::vector<Color>> > tiles;
    int i = 0;
    for(int tileN = 0 ; tileN < 512; tileN++) {
         std::vector<std::vector<Color>> _tile;

        for(int y=0; y<8; y++){ //Y
            uint8_t pixelRow = CHR_ROM.at(y+(16*tileN));
            uint8_t pixelRow2 = CHR_ROM.at((y+8)+(16*tileN));

               std::vector<Color> rowColors;

            for(int pixelCol = 0;pixelCol < 8;pixelCol++){ //X
                
                bool bitLow = pixelRow & (1<< 7 - pixelCol);
                bool bitHigh = pixelRow2 &(1<< 7- pixelCol );
                int index = bitHigh << 1 | bitLow & 0x01;
                Color color = getRGBValue(index);
                rowColors.push_back(color);

                
            }
            _tile.push_back(rowColors);
        }
    tiles.push_back(_tile);
          
    }
return tiles;
   

}
*/




void lineariseTiles( std::vector<std::vector<std::vector<Color>>> tiles, uint8_t *buffer){

    /* 8x8 tilejä */
    /* width = 256, height 240 */
    int i = 0;
    int row = 0;
    int col = 0;
    int cnt = 0;
    for(auto tile: tiles)
    {
            //32 yhelle
            buffer[i] = 0;
            buffer[i+1] = 0;
            buffer[i+2] = 0;

        if(i % 7 == 0){
            cnt++;
        }
           
            row++;
                if(cnt % 32 == 0) {
                    row = 0;
                }
                col = cnt/32;
                i++;
    }

 }

/*
void drawTileToFrame( Frame& frame, int tileNum, const std::vector<uint8_t> &CHR_ROM, int bank) {
//bank left or right, +512kib address for right

    //draw tile to frame

    uint8_t tileSDL[16*3];
    std::vector<std::vector<Color>> tileV;


    //chrom rom 0x2000 
    int tileBaseAddr = tileNum * 16;

        //TODO : Redudant calculation to do twice, for claritys sake double for now.
  std::vector<Color> rowColors;

    std::cout << " Start rowcolors push " << std::endl;
        for(int y=0; y<8; y++){ //Y
            uint8_t pixelRow = CHR_ROM.at(y+(tileBaseAddr));
            uint8_t pixelRow2 = CHR_ROM.at((y+8)+(tileBaseAddr));

             

            for(int pixelCol = 0;pixelCol < 8;pixelCol++){ //X
                
                bool bitLow = pixelRow & (1<< 7 - pixelCol);
                bool bitHigh = pixelRow2 &(1<< 7- pixelCol );
                int index = bitHigh << 1 | bitLow & 0x01;

                std::cout << (int)index << std::endl;

                Color color = getRGBValue(index);
                 rowColors.push_back(color);
            }
           // tileV.push_back(rowColors);
        }

    std::cout << " Start tileSDL  push " << std::endl;
    int i = 0;/*
        for(auto color : rowColors) {
                tileSDL[i] = color.R;
                tileSDL[i+1] = color.G;
                tileSDL[i+2] = color.B;
                i++;
        } */
/*
    std::cout << " Start SEt pixel push " << std::endl;
    std::cout << "rowcolors size: " << (int)rowColors.size() << std::endl;
     i = 0;
    for(int y=0; y<8; y++) {
        for(int x = 0; x<8; x++) {
            frame.setPixel(rowColors.at(i), x,y);
            std::cout << "i: " << i << std::endl;
            i++;
        }
    }
    std::cout << "END SET PIXEL PUSH" << std::endl;
}
*/
ROMData parseROM(  std::vector<uint8_t> romFileBytes ) {
    
    std::cout << "parsing rom ..." << std::endl;
    bool iNESFormat = false;
    bool NES20Format = false;
    bool trainerPresent = false;
    ROMData romdata;
    uint8_t mapper;
    Mirroring mirroringMode;
    std::vector<uint8_t> header; //16bytes
    std::vector<uint8_t> prg_rom;
    std::vector<uint8_t> chr_rom;   
    std::vector<uint8_t> programBytes;

 

    //Get 16 byte header
    for(int i = 0; i < 16; i++) {
        header.push_back(romFileBytes[i]);
    }    
    std::cout << "header read!" << std::endl;
    
    //Get Rest of the rom
    for(auto i = romFileBytes.begin()+16; i != romFileBytes.end(); i++){
        programBytes.push_back(*i);
    }
        std::cout << "rest of file read!" << std::endl;

    /* PARSE HEADER */
    uint8_t byte7 = header[7];
    //CHECK that is iNES file format
    if(header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1a) {
    romdata.iNESFormat = true;
 
    //checkataan bitwise maskilla onko 10xx eli 0x0C on 1000
        
       if (iNESFormat == true && (header[7]&0x0C)==0x08){
                std::cout << "Nes2.0 format (more rare)" << std::endl;
                NES20Format = true; 
            }
            else {
                std::cout << "nes 1.0 format " << std::endl;
            }
    }

    //stop reading
    if(!romdata.iNESFormat) {return ROMData();}

    uint8_t sizeOfPRG_ROM_in16kb_units = 0;
    uint8_t size_of_CHR_ROM_in8kb_units = 0;
    uint8_t flags6 = header[6];
    uint8_t flags7 = header[7];
    uint8_t flags8 = header[8];
    uint8_t mapper_low_half_byte = (flags6 & 0xF0) >> 4;
    uint8_t mapper_hi_half_byte = (flags6 & 0xF0 );

   uint8_t mapper_type = (mapper_hi_half_byte | (mapper_low_half_byte & 0x0F));

    bool VSunistystem = flags7 & (1 <<0);
    bool flags8_15_ines2 = (flags7 & (0x0C) == 2);
  
    mirroringMode  = (flags6 & (1 << 0)) ? Mirroring::HORIZONTAL: Mirroring::VERTICAL;
    trainerPresent = (flags6 & (1 << 2));
    uint8_t skipTrainerBytes = trainerPresent ? 512 : 0;
    
    if(flags6 & (1<<3)) mirroringMode = Mirroring::FOUR_SCREEN;
    

    int prg_rom_size  = 0;
    int chr_rom_size = 0;

    //if flags 8 in ines 1.0 format
    if(!flags8_15_ines2) {
        sizeOfPRG_ROM_in16kb_units = header[4];
        size_of_CHR_ROM_in8kb_units = header[5];
        std::cout << "PRG ROM size 16kib units: " << (unsigned)sizeOfPRG_ROM_in16kb_units <<std::endl;
        std::cout << "CHR ROM size 8kib units: " << (unsigned)size_of_CHR_ROM_in8kb_units <<std::endl;

        prg_rom_size = 16384*sizeOfPRG_ROM_in16kb_units;
        chr_rom_size = 8192*size_of_CHR_ROM_in8kb_units;

            std::cout << "PRG ROM size bytes : " << std::hex << (unsigned)prg_rom_size <<std::endl;
        std::cout << "CHR ROM size bytes units: " << std::hex<< (unsigned)chr_rom_size <<std::endl;

    } else {

        //In ines 2.0 format


    }

    //read program bytes
    for(auto it = (0 + skipTrainerBytes); it < prg_rom_size; ++it){
        romdata.prg_rom.push_back(programBytes.at(it));
    }

    for(auto it = (0 + skipTrainerBytes + prg_rom_size); it < prg_rom_size+chr_rom_size; ++it) {
        romdata.chr_rom.push_back(programBytes.at(it));
    }






    
    std::cout << "ekkat 8x8" << std::endl;
    for(auto i= 0; i <8; i++){
       
           std::cout << std::hex << (unsigned)romdata.chr_rom.at(i) << std::endl;
        
    }

    std::cout << "prg and chr loaded" << std::endl;
    std::cout << "CHR size: "<< std::hex <<(unsigned)romdata.chr_rom.size() << std::endl;
    std::cout << "PRG size: "<<std::hex <<(unsigned)romdata.prg_rom.size() << std::endl;
 


return romdata;

}


int getFileLength(std::ifstream& file) {
        file.seekg (0, file.end);
        int length = file.tellg();
        std::cout << "length: " << length<<std::endl;
        return length;
}

std::vector<uint8_t> loadROMfile(std::string romPath) {

    // Open the file stream
    std::ifstream romFile(romPath, std::ifstream::binary | std::ios_base::in);
    std::vector<uint8_t> romFileBytes;

    
    if(romFile) {

        romFile.seekg (0, romFile.end);
        int length = romFile.tellg();
        romFile.seekg(0, romFile.beg);
        romFileBytes.resize(length);
        std::cout << "Reaidng file, lengthg in bytes: " << length << std::endl;

    /*
        cast is used to convert a pointer of some data type into a pointer of another data type,
        even if the the data types before and after conversion are different.
        It does not check if the pointer type and data pointed by the pointer is same or not.
    */
   
    if (romFile.is_open()) {
              //  romFile.read(reinterpret_cast<char*>(&romFileBytes[0]), length);
                romFile.read(reinterpret_cast<char*>(&romFileBytes[0]), length);
                std::cout << "all characters read successfully.";
         } else{
             std::cout << "error: only " << romFile.gcount() << " could be read";
         }
         
    romFile.close();

    }
 
 else {
        std::cout << "Error opening file!" << std::endl;
    }

    std::cout << "first bytes:" << std::endl;
    std::cout << std::hex << (unsigned)romFileBytes[0] << " - "<< (unsigned)romFileBytes[1] << " - "<< (unsigned)romFileBytes[2] << std::endl;
        
    return romFileBytes;
   
          
}





bool init();
bool loadMedia();
void close();
SDL_Texture* loadTexture(std::string path);
SDL_Window* window = nullptr;
SDL_Renderer* renderer= nullptr;
SDL_Texture * texture = nullptr;

bool init() {

    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    window = SDL_CreateWindow("SDL tut", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH*4, 4*SCREEN_HEIGHT,SDL_WINDOW_SHOWN);


    //create renderer for window
    renderer = SDL_CreateRenderer(window,-1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer,0xff,0xff,0xff,0xff);
    //init png load
    int imgFlags = IMG_INIT_PNG;
    IMG_Init(imgFlags);
    return true;
}

bool loadMedia() {
    texture = loadTexture("./cat.png");
    return true;
}

void close() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

SDL_Texture* loadTexture(std::string path) {
    SDL_Texture* newText = nullptr;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    newText = SDL_CreateTextureFromSurface(renderer,loadedSurface);
    SDL_FreeSurface(loadedSurface);
    return newText;
}



void prepareScene(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer,96,128,255,255);
    SDL_RenderClear(renderer);
}

void presentScene(SDL_Renderer* renderer) {
    SDL_RenderPresent(renderer);
}



void drawTileAtCoord(int x, int y, int tileNum) {


}


void setTextures(const std::vector<std::vector<std::vector<Color>>>& allTiless, std::vector<SDL_Texture*> &textures, SDL_Renderer* renderer){

    for(auto tile: allTiless) {
    uint8_t buffer[8*8*3];
    memset(buffer,0, 8*8*3);
    int i = 0;
    SDL_Texture* tex  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, 8,8);
    for(int y = 0; y < 8; y++) {
        for(int x = 0; x < 8; x++) {
            buffer[i] =   tile[y][x].R;
            buffer[i+1] = tile[y][x].G;
            buffer[i+2] = tile[y][x].B;
            i+=2;
        }
    }
    SDL_SetRenderTarget(renderer, tex);
    SDL_UpdateTexture(tex, NULL, buffer,8*3);
    textures.push_back(tex);
}


}

void tileToSDLArray(int gx, int gy, std::vector<std::vector<Color>> tile, uint8_t* pixelData) {

int _WIDTH = 256;
int _HEIGHT = 240;
int emptyOffset = 0;


 for(int y = 0; y <8; y++) {
 
    for(int x = 0; x <8; x++) {
     std::cout << "start set pixel" << std::endl;

          int baseIndex = (((_WIDTH * 3 * gy) + (gx * 3)) + emptyOffset)+(x*3) ;
    
        /* Frame */
        /* RGB = 3 bytes, pitch == pixel width in bytes, so width*3 */
                // X to index = + x*3 (3 bytes for each data entry)
                // one y adds WIDTH amount of index * 3,
      
            Color color = tile[y][x];
            if(baseIndex+2 < _WIDTH*_HEIGHT*3) {
                pixelData[baseIndex]   = color.R;
                pixelData[baseIndex+1] = color.G;
                pixelData[baseIndex+2] = color.B;
                std::cout << "pixel color RGB:" << color.R << ", " << color.G << ", " << color.B << std::endl;
                    std::cout << "ENd set pixel" << std::endl;
            }
    }
        emptyOffset += ((((256) - (gx))+gx)*3);


 }
      



}


uint8_t* createTilePixelsSDL(const std::vector<std::vector<std::vector<Color>>>& allTiless) {
 

 std::vector<std::vector<Color>> tile = allTiless.at(5);

uint8_t* pixelData = new uint8_t[8*8*3];
 int _WIDTH = 8;
 int _HEIGHT = 8;


 for(int y = 0; y <8; y++) {
    for(int x = 0; x <8; x++) {
     std::cout << "start set pixel" << std::endl;
          int baseIndex = (_WIDTH * 3 * y) + (x * 3);
        /* Frame */
        /* RGB = 3 bytes, pitch == pixel width in bytes, so width*3 */
                // X to index = + x*3 (3 bytes for each data entry)
                // one y adds WIDTH amount of index * 3,
      
            Color color = tile[y][x];
            if(baseIndex+2 < _WIDTH*_HEIGHT*3) {
                pixelData[baseIndex]   = color.R;
                pixelData[baseIndex+1] = color.G;
                pixelData[baseIndex+2] = color.B;
                    std::cout << "ENd set pixel" << std::endl;
                }
    }
 }
      return pixelData;
     
}

//draw a 256*240*3 SDL framebuffer
void draw32x30(PPU* ppu, uint8_t* pixelDataBuffer) {
    //get tiles
    int _WIDTH =256;
    int _HEIGHT = 240;
    int gx = 0;
    int gy = 0;
    int emptyOffset = 0;
    
    uint16_t  nameTableAddr =  ppu->getBGNametableAddr();
    std::cout << "nameTableAddrCOMPLETE: " << std::endl;
    std::vector<std::vector<uint8_t >> BGnametableBytes =  ppu->readBackGroundNametableBytes(0,nameTableAddr );
   std::cout << "BGnametableBytes:" << std::dec << BGnametableBytes.size() << std::endl;
    std::vector< std::vector<std::vector<std::vector<Color>>>> tilesAtVRAM = ppu->mapBGNametableBytesToPatterntableTiles( BGnametableBytes );
    std::cout << "Tiles at VRAM COMPLETE" << std::endl;
    //Tiles at vram 
            // 0   1*8*3  2*3*8
    /* tileRow [ [8x8] [8x8] [8x8]  ]
    /* 
        
    */

    int rowNum = 0;
    int baseIndex = 0;
    for (auto tileRow : tilesAtVRAM)
    {

        int tileXoffset = 0;
        for (auto tile : tileRow)
        {
            int gx = 0; //+8x3 tilestä
            int gy = 0;
            int tileBaseIndex = 0;
            int emptyOffset = 0;
            for (int y = 0; y < 8; y++)
            {
                //jokanen y lisää 3*

                for (int x = 0; x < 8; x++)
                {
                    //(((_WIDTH * 3 * rowNum) + (gx * 3)) + Tilexoffset)+(x*3);

                    baseIndex = (_WIDTH * gy * 3) + (gx*3) + (x*3) + emptyOffset ;
                    Color color = tile[y][x];
                    if (baseIndex + 2 < _WIDTH * _HEIGHT * 3)
                    {
                        std::cout << "Baseindex: " << (unsigned)baseIndex << std::endl;
                        pixelDataBuffer[baseIndex] = color.R;
                        pixelDataBuffer[baseIndex + 1] = color.G;
                        pixelDataBuffer[baseIndex + 2] = color.B;
                        std::cout << "pixel color RGB:" << color.R << ", " << color.G << ", " << color.B << std::endl;
                        std::cout << "ENd set pixel" << std::endl;
                    }
                    gx +=1;
                }
              
                //jokasen y:n lopussa 256-(gx+8*3)+gx
            }
              gy += 8;
               
                emptyOffset += ((((256) - (gx))+gx)*3);
            //jokasen tilen lopussa lisää 8x3 alkuun
           std::cout << "GX, GY: " << std::dec << (unsigned)gx <<", " << (unsigned)gy << std::endl;
        }
        /* Frame */
        /* RGB = 3 bytes, pitch == pixel width in bytes, so width*3 */
        // X to index = + x*3 (3 bytes for each data entry)
        // one y adds WIDTH amount of index * 3,
    }

        //transform tiles to SDL

    //draw each 8x8 tile
    //return SDL format 3x
}





int main() {
 
  std::cout << "Moro" << std::endl;

 //   Memory<uint8_t, uint16_t> mem;
     
        std::vector<uint8_t> rom_raw = loadROMfile("./LNES_Tests/rom_tests/donkey_kong.nes");
        ROMData rom = parseROM(rom_raw);

        LNES_Renderer lnes_renderer;


        Memory WRAM; //CPU
        Memory VRAM; //APU
        WRAM.init(0x100000);
        VRAM.init(2048);

        Bus bus;
        CPU_6502 CPU;
        Cartridge cartridge("Donkey Kongeli");
        CPU.renderer = &lnes_renderer;
         lnes_renderer.framebuffer = new uint8_t[256*240*3];
        
        CPU.setBus(&bus);
    
        WRAM.init_bus(&bus);
         cartridge.load(rom);
        PPU ppu; 
 
          ppu.setBus(&bus);
          ppu.setVRAM(&VRAM);

         bus.init_bus(&WRAM, &CPU, &ppu);
         bus.loadCartridge(&cartridge);
         ppu.setCartridge(&cartridge);
        lnes_renderer.ppu = &ppu;

      


         std::cout << ppu.getCartridge()->_name << ", size: " << cartridge._PRG_ROM->getMemoryLenghtBytes() << std::endl;

 std::cout << "aa" << std::endl;
         CPU.run();




/* TÄSTÄ CHR ROM VIEW  END*/
/*
std::cout << "start text shit " << std::endl;
SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, 28,28);
//Bufferissa RGBA, 4 byteä peräkkäin

uint8_t *pixels = new uint8_t[28*28*3];
memset(pixels, 254, 28*28*3);
pixels[0] = 155; pixels[1] = 20; pixels[2] = 75;
     std::cout << "update " << std::endl;
 SDL_Rect rect2 = {0,0,28,28}; //x,y,w,h
//vika parametri on bytejen määrä rivissä, width on 28 pixel ja yksi pixel on 4 tavua
SDL_UpdateTexture(tex, NULL, pixels,28*3); //copy pixels to the window
*/






        



    return 0;
}   