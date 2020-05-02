#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

typedef uint8_t u8;
typedef uint16_t u16;

const int WIDTH = 64;
const int HEIGHT = 32;
const int PX_SCALE = 10;
const int FPS = 300;

class Chip8
{
    public:
        Chip8();
        void Init();
        void LoadProgram(const std::string &filename);
        void Execute();
        void Run();
        bool GetInput();
        void UpdateTimers();
        void DrawFrame();
        void MemoryDump(u16 start, u16 end);

    private:
        u8      m_Memory[0x1000] = {0};
        u8      m_Display[WIDTH*HEIGHT] = {0};
        u8      m_V[16] = {0};      //8-bit registers 
        u16     m_Stack[16] = {0};  //stack with size 16
        u8      m_SP;               //stack pointer
        u8      m_Keypad;           //16 key keypad
        u8      m_DT;               //delay timer (decrements at a rate of 60Hz when non-zero)
        u8      m_ST;               //sound timer (decrements at a rate of 60Hz when-non-zero)
        u8      m_WaitkeyReg;
        u16     m_PC;               //program counter starts from address 0x200
        u16     m_I;                //register to store memory addresses 
        bool    m_KeyWait;

        const std::unordered_map<int, u8> m_Keys
        {
            {SDLK_1, 0x1},{SDLK_2, 0x2},{SDLK_3, 0x3},{SDLK_4,0xC},
            {SDLK_q, 0x4},{SDLK_w, 0x5},{SDLK_e, 0x6},{SDLK_r,0xD},
            {SDLK_a, 0x7},{SDLK_s, 0x8},{SDLK_d, 0x9},{SDLK_f,0xE},
            {SDLK_z, 0xA},{SDLK_x, 0x0},{SDLK_c, 0xB},{SDLK_v,0xF}
        };

        const u8 m_Font[16*5] = 
                {/*0*/0xF0,0x90,0x90,0x90,0xF0,
                 /*1*/0x20,0x60,0x20,0x20,0x70,
                 /*2*/0xF0,0x10,0xF0,0x80,0xF0,
                 /*3*/0xF0,0x10,0xF0,0x10,0xF0,
                 /*4*/0x90,0x90,0xF0,0x10,0x10,
                 /*5*/0xF0,0x80,0xF0,0x10,0xF0,
                 /*6*/0xF0,0x80,0xF0,0x90,0xF0,
                 /*7*/0xF0,0x10,0x20,0x40,0x40,
                 /*8*/0xF0,0x90,0xF0,0x90,0xF0,
                 /*9*/0xF0,0x90,0xF0,0x10,0xF0,
                 /*A*/0xF0,0x90,0xF0,0x90,0x90,
                 /*B*/0xE0,0x90,0xE0,0x90,0xE0,
                 /*C*/0xF0,0x80,0x80,0x80,0xF0,
                 /*D*/0xE0,0x90,0x90,0x90,0xE0,
                 /*E*/0xF0,0x80,0xF0,0x80,0xF0,
                 /*F*/0xF0,0x80,0xF0,0x80,0x80};

        SDL_Window*     m_Window;
        SDL_Renderer*   m_Renderer;
        Mix_Chunk*      m_BeepSound;
};

#endif

