#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "8080.h"
#include <time.h>

global_variable State8080 GlobalCPU;
global_variable u16 GlobalShiftRegister;
global_variable u8 GlobalShiftOffset;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

u8 *VideoPixels = NULL;
const int WindowWidth = 290;
const int WindowHeight = 360;

// @see: https://www.computerarcheology.com/Arcade/SpaceInvaders/Hardware.html#inputs
static u8 ports[8+1] = {
	0b00001110, // Port 0
	0b00001000, // Port 1
	0b10000000, // Port 2
};

void RenderScreen();

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    /* Create the window */
	if (!SDL_CreateWindowAndRenderer("Hello World", WindowWidth, WindowHeight, NULL, &window, &renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if((!InitCPU(&GlobalCPU)) || (LoadROMFile(&GlobalCPU, "rom/invaders") == 0))
    {
        exit(1);
    }

	// @TODO: review sizing later
    VideoPixels = (u8 *) calloc(4, 256*224);
    if (!VideoPixels)
    {
        printf("[ERROR] failed to alloc VideoPixels\n");
        exit(1);
    }

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if(event->type == SDL_EVENT_KEY_DOWN)
    {
		SDL_KeyboardEvent *e = (SDL_KeyboardEvent *) event;
		if (e->key == SDLK_ESCAPE || e->key == SDLK_Q)
			return SDL_APP_SUCCESS;
		if (e->key == SDLK_C) {
			// @TODO: Pretend a Coin was inserted, start game!
			SDL_Log("Coin deposited!!");
			ports[1] |= 1; // deposit CREDIT
			// ports[1] = 0xFF;
		}
		if (e->key == SDLK_S) {
			SDL_Log("Start please!!");
			ports[1] |= 0b00000100;
		}
    }
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;
}

void GenerateInterrupt(State8080 *cpu, int interrupt_num)
{
	PUSH_PC(&GlobalCPU);
	cpu->pc = 8 * interrupt_num;
	// simulate DI
	GlobalCPU.interrupt_enabled = 0;
}

u8 MachineIN(State8080 *cpu, u8 port)
{
	SDL_Log("IN: Port: %d", port);
	// This enables attract mode
	if (port == 0)
		return 1;
	if (port == 1)
		return 0;

	if (port == 3)
	{
		return ((GlobalShiftRegister >> (8-GlobalShiftOffset)) & 0xFF);
	}
	return ports[port];
}

void MachineOUT(State8080 *cpu, u8 port)
{
	if (port != 6) // Watch dog is noisy.
		SDL_Log("OUT: Port: %d", port);
	if (port == 2)
	{
		GlobalShiftOffset = cpu->a & 0x7;
	}
	if (port == 4)
	{
		GlobalShiftRegister = (cpu->a << 8) | (GlobalShiftRegister >> 8);
	}
	ports[port] = cpu->a;
}


/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
	local_persist time_t LastRenderTime;
	local_persist time_t LastInterruptTime;
	local_persist bool AlternateInterrupt;

    // SDL_Delay(0.1);
    // GlobalCPU.DebugPrint = 1;
	u8 *opcode = &GlobalCPU.memory[GlobalCPU.pc];
	if (*opcode == 0xDB) // IN
	{
		u8 port = opcode[1];
		GlobalCPU.a = MachineIN(&GlobalCPU, port);
		GlobalCPU.pc+=2;
	}
	else if (*opcode == 0xD3) // OUT
	{
		u8 port = opcode[1];
		MachineOUT(&GlobalCPU, port);
		GlobalCPU.pc+=2;
	}
	else 
		Emulate8080Op(&GlobalCPU);

	time_t now = time(NULL);

	if ((now - LastInterruptTime > 1.0/60.0) && GlobalCPU.interrupt_enabled)
	{
		if (AlternateInterrupt)
			GenerateInterrupt(&GlobalCPU, 2); // Interrupt "0x10"
		else
			GenerateInterrupt(&GlobalCPU, 1); // Interrupt "0x8"
		AlternateInterrupt = !AlternateInterrupt;
		LastInterruptTime = time(NULL);
	}

	if(now - LastRenderTime > 1.0/60.0)
    {
		RenderScreen();
		LastRenderTime = time(NULL);
    }

    return SDL_APP_CONTINUE;
}

void RenderScreen()
{
	// @see: https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html
	// NOTE: Video RAM is 2400-3FFF
	// The screens pixels are on/off (1 bit each). 256*224/8 = 7168 (7K) bytes.
	u8 *VideoRAM = GlobalCPU.memory + 0x2400;

	// @TODO: Don't write into separate buffer.
	u8 *Pixel = (u8 *)VideoPixels;
	for(int Y=0; Y < 224; ++Y)
	{
		for(int X=0; X < 256/8; (++X, ++VideoRAM))
		{
			if((VideoRAM - GlobalCPU.memory) > 0x3FFF)
			{
				printf("[ERROR] VideoRAM overflowed: 0x%x", (VideoRAM - GlobalCPU.memory));
				exit(1);
			}

			for(int I=0; I<8; ++I)
			{
				if((*VideoRAM >> I) & 1)
					*Pixel++ = 1;
				else
					*Pixel++ = 0;
			}

#if 0
			if (*VideoRAM != 0)
			{
				printf("0x%x ", *VideoRAM);
				PrintBinary(*VideoRAM);
			}
#endif
		}
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	int X2 = 0;
	int Y2 = 0;
	for(int X=256-1; X>=0; (--X, ++Y2, X2=0))
	{
		for(int Y=0; Y<224; (++Y, ++X2))
		{
			u8 *GamePixel = (u8 *)(VideoPixels + (X) + (Y*256));
			if (*GamePixel)
			{
				SDL_RenderPoint(renderer, X2, Y2);
			}
		}
	}

	SDL_RenderPresent(renderer);
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}

