#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "8080.h"
#include <time.h>

global_variable State8080 GlobalCPU;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

u8 *VideoPixels = NULL;
const int WindowWidth = 290;
const int WindowHeight = 360;

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

    VideoPixels = (u8 *) calloc(1, 256*224);
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
		}
    }
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;
}

void GenerateInterupt(State8080 *cpu, int interrupt_num)
{
	// "PUSH PC"
	// ??? review
	// Push(state, (state->pc & 0xFF00) >> 8, (state->pc & 0xff));
	cpu->pc = 8 * interrupt_num;
}

static time_t lastInterrupt;

u8 MachineIN(State8080 *cpu, u8 port)
{
	u8 a;
	switch(port)
	{
		case 3:
		{
			u16 v = (shift1<<8) | shift0;
			a = ((v >> (8-shift_offset)) & 0xFF);
		} break;
	}
	return a;
}

void MachineOUT(u8 port, u8 value)
{
	switch(port)
	{
		case 2:
		{
			shift_offset = value & 0x7;
		} break;
		case 4:
		{
			shift0 = shift1;
			shift1 = value;
		} break;
	}
}

void MachineKeyUp(u8 dir)
{
	// @TODO:
}
void MachineKeyDown(u8 dir)
{
	// @TODO:
	switch(dir)
	{
		case LEFT:
		{
			port[1] = 0x20; // set bit 5 of port 1
		} break;
		case RIGHT:
		{
			port[1] = 0x40; // set bit 6 of port 1
		} break;
	}
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    // SDL_Delay(0.001);
    // GlobalCPU.DebugPrint = 1;
	u8 opcode = state->memory[state->[pc];
	if (*opcode == 0xDB) // IN
	{
		u8 port = opcode[1];
		GlobalCPU.a = MachineIN(&GlobalCPU, port);
		GlobalCPU.pc++;
	}
	else if (*opcode == 0xD3) // OUT
	{
		u8 port = opcode[1];
		MachineOUT(&GlobalCPU, port);
		GlobalCPU.pc++;
	}
	else 
	{
		Emulate8080Op(&GlobalCPU);
	}

	if (time(NULL) - lastInterrupt > 0) // 1/60 seconds has elapsed
	{
		if (GlobalCPU.interrupt_enabled)
		{
			GenerateInterupt(&GlobalCPU, 2); // Interrupt 2
			lastInterrupt = time(NULL);
		}
	}


	// @TODO: Timing, Proper video refresh rate and pixel refreshing
	// @see: https://web.archive.org/web/20241011062657/http://www.emulator101.com/displays-and-refresh.html
    if((GlobalCPU.Steps%10000) == 0)
    {
        // @see: https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html
        // NOTE: Video RAM is 2400-3FFF
        // The screens pixels are on/off (1 bit each). 256*224/8 = 7168 (7K) bytes.
        u8 *VideoRAM = GlobalCPU.memory + 0x2400;

        // @TODO: don't use u8 for a 1 bit value.
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

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}

