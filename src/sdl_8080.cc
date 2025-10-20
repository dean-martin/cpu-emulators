#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
// @TODO: replace with header
#include "8080.cc"
#include <time.h>

// @TODO: "space-invaders.h" or "sdl_8080.h"
#define SCREEN_WIDTH 290
#define SCREEN_HEIGHT 360

typedef struct {
	State8080 cpu;

	u8 *video_pixels;

	u16 shift_register;
	u8 shift_offset;

	SDL_Window *window;
	SDL_Renderer *renderer;
} App;

App app;

// @see: https://www.computerarcheology.com/Arcade/SpaceInvaders/Hardware.html#inputs
static u8 InputPorts[8+1] = {
// Port 0
//  bit 0 DIP4 (Seems to be self-test-request read at power up)
//  bit 1 Always 1
//  bit 2 Always 1
//  bit 3 Always 1
//  bit 4 Fire
//  bit 5 Left
//  bit 6 Right
//  bit 7 ? tied to demux port 7 ?
	0b00001110, // Port 0

// Port 1
//  bit 0 = CREDIT (1 if deposit)
//  bit 1 = 2P start (1 if pressed)
//  bit 2 = 1P start (1 if pressed)
//  bit 3 = Always 1
//  bit 4 = 1P shot (1 if pressed)
//  bit 5 = 1P left (1 if pressed)
//  bit 6 = 1P right (1 if pressed)
//  bit 7 = Not connected
	0b00001000, // Port 1

// Port 2
//  bit 0 = DIP3 00 = 3 ships  10 = 5 ships
//  bit 1 = DIP5 01 = 4 ships  11 = 6 ships
//  bit 2 = Tilt
//  bit 3 = DIP6 0 = extra ship at 1500, 1 = extra ship at 1000
//  bit 4 = P2 shot (1 if pressed)
//  bit 5 = P2 left (1 if pressed)
//  bit 6 = P2 right (1 if pressed)
//  bit 7 = DIP7 Coin info displayed in demo screen 0=ON
	0b10000000, // Port 2

// Port 3
//   bit 0-7 Shift register data
	0x0,
	// Outputs Ports? what.
};

static u8 OutputPorts[8+1] = {
// Port 2:
//  bit 0,1,2 Shift amount

// Port 3: (discrete sounds)
//  bit 0=UFO (repeats)        SX0 0.raw
//  bit 1=Shot                 SX1 1.raw
//  bit 2=Flash (player die)   SX2 2.raw
//  bit 3=Invader die          SX3 3.raw
//  bit 4=Extended play        SX4
//  bit 5= AMP enable          SX5
//  bit 6= NC (not wired)
//  bit 7= NC (not wired)
//  Port 4: (discrete sounds)
//  bit 0-7 shift data (LSB on 1st write, MSB on 2nd)

// Port 5:
//  bit 0=Fleet movement 1     SX6 4.raw
//  bit 1=Fleet movement 2     SX7 5.raw
//  bit 2=Fleet movement 3     SX8 6.raw
//  bit 3=Fleet movement 4     SX9 7.raw
//  bit 4=UFO Hit              SX10 8.raw
//  bit 5= NC (Cocktail mode control ... to flip screen)
//  bit 6= NC (not wired)
//  bit 7= NC (not wired)

// Port 6:
//  Watchdog ... read or write to reset
};

void RenderScreen();

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    /* Create the window */
	if (!SDL_CreateWindowAndRenderer("Hello World", SCREEN_WIDTH, SCREEN_HEIGHT, NULL, &app.window, &app.renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

	if (!InitCPU(&app.cpu)) 
	{
		SDL_Log("Failed to init CPU, not enough memory?");
		exit(1);
	}
    if(LoadROMFile(&app.cpu, "rom/invaders") == 0)
    {
		SDL_Log("Failed to load invaders rom file, make sure it's placed as ./rom/invaders");
        exit(1);
    }

	// @TODO: review sizing later
    app.video_pixels = (u8 *) calloc(4, 256*224);
    if (!app.video_pixels)
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
		if (e->down)
		{
			if (e->key == SDLK_C) 
			{
				// @TODO: Pretend a Coin was inserted, start game!
				// PrintBinary(InputPorts[1]);
				InputPorts[1] |= 1; // deposit CREDIT
				// PrintBinary(InputPorts[1]);
				SDL_Log("Coin deposited!!");
			}
			if (e->key == SDLK_S) 
			{
				SDL_Log("Start please!!");
				InputPorts[1] |= 0b00000100;
			}
		}

		if (e->key == SDLK_D)
		{
			app.cpu.DebugPrint = app.cpu.DebugPrint ? 0 : 1;
			SDL_Log("debug print: %d", app.cpu.DebugPrint);
		}

		// key released
		if (!e->down)
		{
			if (e->key == SDLK_C) 
			{
				// @TODO: Pretend a Coin was inserted, start game!
				SDL_Log("Coin bit cleared.");
				// InputPorts[1] &= ~0x1; // deposit CREDIT
			}
		}
    }
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;
}

u8 MachineIN(State8080 *cpu, u8 port)
{
	// // This enables attract mode?
	// if (port == 1)
	// 	return 1;
	// if (port == 2)
	// 	return InputPorts[2];

	if (port == 3)
	{
		return ((app.shift_register >> (8-app.shift_offset)) & 0xFF);
	}

	return InputPorts[port];
}

void MachineOUT(State8080 *cpu, u8 port)
{
	if (port != 6) // Watch dog is noisy.
		SDL_Log("OUT: Port: %d", port);
	if (port == 2)
	{
		app.shift_offset = cpu->a & 0x7;
	}
	if (port == 4)
	{
		app.shift_register = (cpu->a << 8) | (app.shift_register >> 8);
	}
	OutputPorts[port] = cpu->a;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
	time_t Now = time(NULL);
	local_persist time_t LastInterruptTime;
	local_persist time_t LastRenderTime;
	local_persist bool AlternateInterrupt;
	local_persist time_t PreviousNow;

	if (app.cpu.interrupt_enabled && (Now - LastInterruptTime) > 1.0/60.0)
	{
		if (AlternateInterrupt)
		{
			GenerateInterrupt(&app.cpu, 2); // Interrupt "0x10"
		}
		else
		{
			GenerateInterrupt(&app.cpu, 1); // Interrupt "0x8"
		}
		AlternateInterrupt = !AlternateInterrupt;
		LastInterruptTime = time(NULL);
		SDL_Log("interrupted");
	}

	if((Now - LastRenderTime) > 1.0/60.0)
    {
		RenderScreen();
		LastRenderTime = time(NULL);
		// SDL_Log("rendered");
    }
	
	// @TODO: fix
	double MillisecondDifference = (Now - PreviousNow)/1000;
	int cycles_to_catch_up = 2 * MillisecondDifference;
	cycles_to_catch_up+=10; //lol
	int cycles = 0;
	if (MillisecondDifference)
		printf("md:%f\n",MillisecondDifference);

	while(cycles_to_catch_up > cycles)
	{
		u8 *opcode = &app.cpu.memory[app.cpu.pc];
		if (*opcode == 0xDB) // IN
		{
			u8 port = opcode[1];
			app.cpu.a = MachineIN(&app.cpu, port);

			char *bin = (char *)malloc(sizeof(char[9]));
			ByteToBinary(app.cpu.a, bin);
			SDL_Log("IN: port:%d bin: %s\n", port, bin);
			free(bin);

			app.cpu.pc += 2;
			cycles += 3;
		}
		else if (*opcode == 0xD3) // OUT
		{
			u8 port = opcode[1];
			MachineOUT(&app.cpu, port);

			SDL_Log("OUT happened: a: %d\n", app.cpu.a);

			app.cpu.pc += 2;
			cycles += 3;
		}
		else
		{
			cycles += Emulate8080Op(&app.cpu);
		}
	}

	PreviousNow = Now;

    return SDL_APP_CONTINUE;
}

void RenderScreen()
{
	// @see: https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html
	// NOTE: Video RAM is 2400-3FFF
	// The screens pixels are on/off (1 bit each). 256*224/8 = 7168 (7K) bytes.
	u8 *VideoRAM = app.cpu.memory + 0x2400;

	// @TODO: Don't write into separate buffer.
	u8 *Pixel = (u8 *)app.video_pixels;
	for(int Y=0; Y < 224; ++Y)
	{
		for(int X=0; X < 256; (X+=8, ++VideoRAM))
		{
			if((VideoRAM - app.cpu.memory) > 0x3FFF)
			{
				printf("[ERROR] VideoRAM overflowed: 0x%x", (VideoRAM - app.cpu.memory));
				exit(1);
			}

			for(int I = 0; I < 8; ++I)
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

	// Clear to black
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
	SDL_RenderClear(app.renderer);

	// render color green
	SDL_SetRenderDrawColor(app.renderer, 0, 255, 0, 255);
	int X2 = 0;
	int Y2 = 0;
	for(int X=256-1; X>=0; (--X, ++Y2, X2=0))
	{
		for(int Y=0; Y<224; (++Y, ++X2))
		{
			u8 *GamePixel = (u8 *)(app.video_pixels + (X) + (Y*256));
			if (*GamePixel)
			{
				SDL_RenderPoint(app.renderer, X2, Y2);
			}
		}
	}

	SDL_RenderPresent(app.renderer);
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}

