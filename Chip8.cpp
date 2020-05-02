#include "Chip8.h"
#include <fstream>
#include <random>
#include <iostream>
#include <iomanip>

std::random_device d;
std::mt19937 rand_eng(d());
std::uniform_int_distribution<int> uniform_dist(0,255);

Chip8::Chip8():
    m_PC(0x200),
    m_SP(0),
    m_DT(0),
    m_ST(0),
    m_WaitkeyReg(0),
    m_I(0),
    m_KeyWait(false)
{
    //load font into memory
	for(int i=0;i<16*5;++i) m_Memory[i] = m_Font[i];
}


void Chip8::LoadProgram(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary);

    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();

    if(file.is_open())
    {
            //all programs start at memory location 0x200
            char *tmp = (char *) &m_Memory[0x200];
            file.seekg(0, std::ios::beg);
            file.read(tmp,file_size);
            file.close();
    }
    else
    {
            std::cout<<"Error loading program"<< std::endl;
    }	

    file.close();
}

void Chip8::Execute()
{
	u16 opcode = (m_Memory[m_PC] << 8) | m_Memory[m_PC+1]; 
	u16 addr = (opcode & 0x0FFF);
	u16 tmp = 0x0000;
	u8 nibble = (opcode & 0xF);
	u8 x = ((opcode & 0x0F00) >> 8);
	u8 y = ((opcode & 0x00F0) >> 4);
	u8 kk = (opcode & 0x00FF);

	switch(opcode & 0xF000)
	{
		case 0x0000:
			switch(kk)
			{
				//Clear screen CLS
				case 0x00E0:
                    for(int i=0; i<WIDTH*HEIGHT;++i) m_Display[i] = 0;
					m_PC+=2;
					break;
				//Return from subroutine RET
				case 0x00EE:
                    m_SP--;
					m_PC=m_Stack[m_SP];
                    m_PC+=2;
					break;
				default:
					std::cout<<"Unknown instruction"<<std::endl;
                    exit(1);
					break;
			}
			break;

		//Jump to address JP addr
		case 0x1000:
			m_PC = addr;
			break;
		//call subroutine at nnn CALL addr
		case 0x2000:
			m_Stack[m_SP++] = m_PC;
			m_PC= addr;
			break;
		//skip next instruction if Vx = kk SE Vx, byte
		case 0x3000:
			if(m_V[x] == kk) m_PC+=4;
			else m_PC+=2;
			break;
		//skip next instruction if Vx != kk SNE Vx, byte
		case 0x4000:
			if(m_V[x] != kk) m_PC+=4;
			else m_PC+=2;
			break;
		//skip next instruction if Vx = Vy SE Vx, Vy
		case 0x5000:
			if(m_V[x] == m_V[y]) m_PC+=4;
			else m_PC+=2;
			break;
		//set Vx = kk LD Vx, byte
		case 0x6000:
			m_V[x] = kk;
			m_PC+=2;
			break;
		//set Vx = Vx + kk ADD Vx, byte
		case 0x7000:
			m_V[x] += kk;
			m_PC+=2;
			break;	
		case 0x8000:
			switch(nibble)
			{
				//set Vx = Vy LD Vx, Vy
				case 0x0000:
					m_V[x] = m_V[y];
					m_PC+=2;
					break;
				//set Vx = Vx OR Vy
				case 0x0001:
					m_V[x] |= m_V[y];
					m_PC+=2;
					break;
				//set Vx = Vx AND Vy
				case 0x0002:
					m_V[x] &= m_V[y];
					m_PC+=2;
					break;
				//set Vx = Vx XOR Vy
				case 0x0003:
					m_V[x] ^= m_V[y];
					m_PC+=2;
					break;
				//set Vx = Vx + Vy, set VF = carry
				case 0x0004:
					if(m_V[x] > (0xFF-m_V[y])) m_V[15] = 1;
					else m_V[15] = 0;

					m_V[x] += m_V[y];
					m_PC+=2;
					break;
				//set Vx = Vx - Vy, set VF = NOT borrow
				case 0x0005:
					if(m_V[y] > m_V[x]) m_V[15] = 0;
					else m_V[15] = 1;
					m_V[x] -= m_V[y];

					m_PC+=2;
					break;
				//set Vx = Vx SHR 1
				case 0x0006:
					m_V[15] = (m_V[x] & 1);
					m_V[x] = (m_V[x] >> 1); 
					m_PC+=2;
					break;
				//set Vx = Vy - Vx, set VF = NOT Borrow SUBN Vx, Vy
				case 0x0007:
					if(m_V[x] > m_V[y]) m_V[15] = 0;
					else m_V[15] = 1;

					m_V[x] = m_V[y] - m_V[x];
					m_PC+=2;
					break;
				//Set Vx = Vx SHL 1
				case 0x000E:
					m_V[15] = ((m_V[x] >> 7) & 1);
					m_V[x] = (m_V[x] << 1);
					m_PC+=2;
					break;
			}
            break;

			//skip next instruction if Vx != Vy
			case 0x9000:
				if(m_V[x] != m_V[y]) m_PC+=4;	
				else m_PC+=2;
				break;

			//set i = nnn
			case 0xA000:
				m_I = addr;
				m_PC+=2;
				break;

			//jump to location nnn + V0
			case 0xB000:
				m_PC = (addr + m_V[0]);
				break;

			//Set Vx = random byte AND kk
			case 0xC000:
			{
				int rand = uniform_dist(rand_eng); 
				m_V[x] = (rand & kk);
				m_PC+=2;
				break;
			}
			//Display n-byte sprite starting at memory location i at (Vx, Vy), set VF = collision
			case 0xD000:
			{
				int x_pos = m_V[x];
				int y_pos = m_V[y];

				m_V[15] = 0;

                if(x_pos > WIDTH) x_pos=0;
                if(y_pos > HEIGHT) y_pos=0; 
                if(x_pos < 0) x_pos=WIDTH;
                if(y_pos < 0) y_pos=HEIGHT;
                
                for(int i=0;i<nibble;++i)
                {
                    for(int k=0;k<8;++k)
                    {
                        if(((m_Memory[m_I+i] << k) & 0x80) == 0x80)
                        {
                            if(m_Display[(x_pos+k)+((y_pos+i)*WIDTH)] == 1)
                            {
                                m_V[15] = 1;
                            }
                            m_Display[(x_pos+k)+((y_pos+i)*WIDTH)] ^= 1;
                        }
                    }
                }

				m_PC+=2;
				break;
			}
			case 0xE000:
				switch(kk)
				{
					//Skip next instruction if key with the value of Vx is pressed
					case 0x009E:
						if(m_Keypad == m_V[x]) m_PC+=4;
						else m_PC+=2;
						break;
					//Skip next instruction if key with the value of Vx is not pressed
					case 0x00A1:
						if(m_Keypad != m_V[x]) m_PC+=4;
						else m_PC+=2;
						break;
					default:
						std::cout<<"Unknown instruction"<<std::endl;
                        exit(1);
						break;
				}
				break;
			case 0xF000:
				switch(kk)
				{
					//Set Vx = delay timer value
					case 0x0007:
						m_V[x] = m_DT;
						m_PC+=2;
						break;
					// Wait for a key press, store the value of the key in Vx
					case 0x000A:
						m_WaitkeyReg = x;
						m_KeyWait = true;
						m_PC+=2;
						break;
					//Set delay timer = Vx
					case 0x0015:
						m_DT = m_V[x];
						m_PC+=2;
						break;
					//Set sound timer = Vx
					case 0x0018:
						m_ST = m_V[x];
						m_PC+=2;
						break;
					//Set I = I + Vx
					case 0x001E:
						m_I+= m_V[x];
						m_PC+=2;
						break;
					//Set I = location of sprite for digit Vx
					case 0x0029:
						m_I = m_V[x]*5;
						m_PC+=2;
						break;
					//Store BCD representation of Vx in memory locations I, I+1, and I+2
					case 0x0033:
						m_Memory[m_I] = ((m_V[x])/100)%10;
						m_Memory[(m_I+1)] = ((m_V[x])/10)%10;
						m_Memory[(m_I+2)] = (m_V[x])%10;
						m_PC+=2;
						break;
					//Store registers V0 through Vx in memory starting at location i
					case 0x0055:
						for(int i=0;i<=x;++i) m_Memory[m_I+i] = m_V[i];
						m_PC+=2;
						break;
					//Read registers V0 through Vx from memory starting at location i
					case 0x0065:
						for(int i=0;i<=x;i++) m_V[i] = m_Memory[m_I+i];
						m_PC+=2;
						break;
				}
				break;
		default:
            exit(1);
			std::cout<<"Unknown instruction"<<std::endl;
			break;
	}
}

void Chip8::Init()
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0 )
    {
            std::cout<<"SDL_Init error: " << SDL_GetError() << std::endl;
            return;
    }

    m_Window = SDL_CreateWindow("Chip8", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WIDTH*PX_SCALE,HEIGHT*PX_SCALE, SDL_WINDOW_SHOWN);
    m_Renderer = SDL_CreateRenderer(m_Window, -1, 0);

    Mix_Init(MIX_INIT_OGG);
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cout<<"Could not initialize SDL_mixer"<<std::endl;
        return;
    }
    
    m_BeepSound = Mix_LoadWAV("beep.ogg");
    if(m_BeepSound == nullptr)
    {
        std::cout<<"Could not load beep sound"<<Mix_GetError()<<std::endl;
        return;
    }
}

void Chip8::Run()
{
    Init();

    float startTime = SDL_GetTicks();
    bool running=true;

    for(;;)
    {
        if(m_PC >= 0x1000) break;

        if(!GetInput()) break;
        
        float currTime = SDL_GetTicks();
        float deltaTime = currTime - startTime;

        if(deltaTime >= (1000.0f/FPS))
        {
            UpdateTimers();
            if(!m_KeyWait) Execute();
            DrawFrame();
            startTime = currTime;
        } 
    }
    Mix_FreeChunk(m_BeepSound);
    Mix_CloseAudio();
    SDL_DestroyRenderer(m_Renderer);
    SDL_DestroyWindow(m_Window);
    Mix_Quit();
    SDL_Quit();
}

bool Chip8::GetInput()
{
    SDL_Event e;
    while(SDL_PollEvent(&e))
    {
        switch(e.type)
        {
            case SDL_QUIT:
                return false; 
            case SDL_KEYUP:
                m_Keypad = 0xF;			
                break;
            case SDL_KEYDOWN:
                if(e.key.keysym.sym == SDLK_ESCAPE) return false;

                auto i = m_Keys.find(e.key.keysym.sym);
                if(i != m_Keys.end()) m_Keypad = i->second;
                if(i != m_Keys.end() && m_KeyWait)
                {
                    m_V[m_WaitkeyReg] = m_Keypad;
                    m_KeyWait = false;
                }
                break;
        }
    }

    return true;
}

void Chip8::UpdateTimers()
{
    if(m_DT > 0) --m_DT;
    if(m_ST > 0) 
    {
        if(m_ST == 1) Mix_PlayChannel(-1,m_BeepSound,0); 
        --m_ST;
    }
}

void Chip8::DrawFrame()
{
    for(int i=0; i<WIDTH*HEIGHT; ++i)
    {
        if(m_Display[i] == 1)
            SDL_SetRenderDrawColor(m_Renderer, 255,255,255,255);	
        else
            SDL_SetRenderDrawColor(m_Renderer, 0,0,0,255);	

        SDL_Rect r;
        r.x = (i%(WIDTH))*PX_SCALE;
        r.y = (i/(WIDTH))*PX_SCALE;
        r.w = PX_SCALE;
        r.h = PX_SCALE;
        SDL_RenderFillRect(m_Renderer, &r);
    }
    SDL_RenderPresent(m_Renderer);
}

void Chip8::MemoryDump(u16 start, u16 end)
{
    std::cout<<std::hex<<(int)0x0<<"\t";
    for(int i=start;i<end;++i)
    {
        if(i%0x10 == 0)
        {
                std::cout<<"\n";
                std::cout<<std::hex<<(int)i<<"\t";
        }
        std::cout<<std::hex<<std::setfill('0')<<std::setw(2)<<(int)(m_Memory[i])<<" ";
    }
}
