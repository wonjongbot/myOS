#include "graphics.h"

int rgb(int r, int g, int b){
    r = (int)(r>>3); //5
    g = (int)(g>>2); //6
    b = (int)(b>>3); //5

    return r<<11 | g<<5 | b;
}

void Draw(int x, int y, int r, int g, int b){
    VBEInfoBlock* VBE = (VBEInfoBlock*) VBEInfoAddress;
    uint16_t* buffer = (uint16_t*) FramebufferAddr; 

    int index = y * VBE->width + x;
    *(buffer + index) = rgb(r,g,b);
    //*((unsigned short*)VBE->framebuffer+index) = rgb(r,g,b);
}
void ClearScreen(int r, int g, int b){
    VBEInfoBlock* VBE = (VBEInfoBlock*) VBEInfoAddress;
    for(int y = 0; y < VBE->height; y++){
        for(int x = 0; x < VBE->width; x++){
            Draw(x,y,r,g,b);
        }
    }
}
void DrawRect(int x, int y, int width, int height, int r, int g, int b){
    for (int j = y; j < (y+height); j++){
        for(int i = x; i < (x+ width); i++){
            Draw(i, j, r, g, b);
        }
    }
}

void DrawChar(int (*f)(int, int), int font_width, int font_height, char glyph,  int x, int y, int r, int g, int b){
    for (int j = 0; j < font_height; j++){
        unsigned int row = (*f)((int)(glyph), j);
        int shift = (font_width) - 1;
        int bit = 0;
        for(int i = 0; i < font_width; i++){
            bit = (row >> (shift)) & (unsigned int)0x1;
            if (bit == 1)
                Draw(i+x, j+y , r, g, b);
            shift -= 1;
        }
    }
}

void DrawStr(int (*f)(int, int), int font_width, int font_height, char* str, int x, int y, int r, int g, int b){
    VBEInfoBlock* VBE = (VBEInfoBlock*) VBEInfoAddress;
    int i = 0, j = 0, k = 0;
    for(int k = 0; *(str + k) != 0; k++){
        if(*(str + k) != '\n'){
            DrawChar(f, font_width, font_height, *(str+k), x + i*font_width, y + j*font_height, r, g, b);
            i += 1;
        }
        else{
            i = 0;
            j += 1;
        }
    }
}

void Flush(){
    VBEInfoBlock* VBE = (VBEInfoBlock*) VBEInfoAddress;
    uint16_t* buffer = (uint16_t*) FramebufferAddr; 
    int index;

    for (int y = 0; y < VBE->height; y++){
        for (int x = 0; x < VBE->width; x++){
            index = y * VBE->width + x;
            *((uint16_t*)VBE->framebuffer + index) = *(buffer + index);
        }
    }
}