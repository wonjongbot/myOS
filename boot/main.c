char* vram;

int start(){
    vram = (char*) 0xb8000;

    for(int i = 0; i < 2 * 25 * 80; i +=2){
        *(vram + i) = 0;
        *(vram + i + 1)  = 0x1c;
    }
    return 0;
}