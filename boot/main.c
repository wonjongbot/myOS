#include "graphics.h"

int start(){
    //DrawChar(getCodePage437Character, font_codepage437_width, font_codepage437_height, 'A', 8, 0, 255,255,255);
    char str[] = "Poopy Stinky";
    DrawStr(getCodePage437Character, font_codepage437_width, font_codepage437_height, str, 0, 0, 255,255,255);
    while(1){
        ClearScreen(19, 41, 75);
        DrawRect(359,162,307,118, 255, 95, 5);
        DrawRect(429,280,169,208, 255, 95, 5);
        DrawRect(359,488,307, 118, 255, 95, 5);
        DrawStr(getCodePage437Character, font_codepage437_width, font_codepage437_height, str, 0, 0, 255,255,255);

        Flush();
    }
}