#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "8080.h"
#include <time.h>

// @TODO: scaling up the screen. maybe there's a SDL stretch blit?
// @TODO: sound

// good reference: https://www.youtube.com/watch?v=uGjgxwiemms
// timing is way too slow

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

// @TODO: disassembler is inconsistant with literal hex, address hex, and memory hex
// i.e., #01, $006c, ($006c)
// review.

// An advanced debugging framework would be amazing here, saving and reloading state,
// and a UI to inspect memory and what not. Naming functions at addresses, etc.
// A real debugger for 8080 cpu emulation.

// PC at 0x0020 is IN#0x01
// RRC checks start at 0x0022
// Port 1 is 0b00001111
// JC $0067
// MVI A, #$01
// STA ($20ea) (this is what gets checked later on?)
// 0x006c JMP $003f
// 0x003f STA($20ea) (why are we storing this twice? what)
// 0x0042 LDA $20e9
// ANA A -- C=1
// 0x0046 JZ #$0082
// 0x0049 LDA $20ef
// ANA A
// JNZ $006f
// 0x0050 LDA $20eb
//
// This routine loads a byte into memory, then RRC offs to check the first 3 bits.
// Where is the $20ca write then?
// CALL $0abf -- This starts a common routine?
// LDA $20ca
// RRC
// JC $0abb
// RRC
// JC $1868
// RRC
// JC $0aab
// RET
// 0x005a JMP $0082 -- restarting the loop?
// yup, popping registers into the stack to then Enable Input (EI)
// 0x000c JMP $008c -- this does some XRA A and wierdness, to clear the CY bit?
// XRA A
// STA ($2072)
// 0x0090 LDA $20e9
// ANA A
// JZ #$0082
// 0x0097(cont) LDA $20ef
// ANA A
// 0x009b JNZ $00a5
// 0x009e LDA $20c1
// RRC
// JNC #$0082
// POP H
// POP D
// POP B
// POP PSW
// EI
// ...
// -- this is the TILT check, neat. does early return if fine.
// 0x001d CALL $17cd
// 0x17cd IN #0x02 (port "2")
// ANI #$04
// RZ (Z=1)
// 0x0020 IN #0x01 (port "1") (00001111) I think this was input?
// 0x0022 RRC
// 0x0023 JC $0067 
// 0x0067 MVI A, #$01
// 0x0069 STA ($20ea)
// 0x006c JMP $003f
// 0x003f STA ($20ea)

// don't understand, it's checking $20xx area, (considered RAM in SpaceInvaders iirc)
// but storing input in places it doesn't check later on? Is there a bug in the emulation?
// Storing to: $2072, $20ea
// Loading from: $20e9, $20ef,$20ca, $20c1

// I might have it stuck in a early boot loop with an input flag that's suppose
// to clear later on. Yeah, I shifted up the ports (n+1 = n), and that was definitely wrong.
// Port[2] is definitely the TILT check using 0x04
// okay, retrying with what I think are "

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
	0b00001111, // Port 0


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
		SDL_KeyboardEvent e = event->key;
		if (e.key == SDLK_ESCAPE || e.key == SDLK_Q)
			return SDL_APP_SUCCESS;
		if (e.down)
		{
			if (e.key == SDLK_C) {
				InputPorts[1] |= 1; // deposit CREDIT
			}

			// player 1
			if (e.key == SDLK_S) {
				InputPorts[1] |= 0b00000100;
			}
			if (e.key == SDLK_LEFT) {
				InputPorts[1] |= 0b00100000;
			}
			if (e.key == SDLK_RIGHT) {
				InputPorts[1] |= 0b01000000;
			}
			if (e.key == SDLK_UP || e.key == SDLK_SPACE) {
				InputPorts[1] |= 0b00010000;
			}

			// player 2
			// @TODO: pick a decent layout? or just use the same...
			if (e.key == SDLK_E) {
				InputPorts[1] |= 0b00000010;
			}
			if (e.key == SDLK_A) {
				InputPorts[2] |= 0b00100000;
			}
			if (e.key == SDLK_D) {
				InputPorts[2] |= 0b01000000;
			}
			if (e.key == SDLK_W) {
				InputPorts[2] |= 0b00010000;
			}
		} 

		if (e.key == SDLK_D)
		{
			// app.cpu.debug = app.cpu.debug ? 0 : 1;
			// SDL_Log("debug print: %d", app.cpu.debug);
		}

    }

	if (event->type == SDL_EVENT_KEY_UP) {
		SDL_KeyboardEvent e = event->key;
		if (e.key == SDLK_C) {
			InputPorts[1] &= ~1;
		}

		// player 1
		if (e.key == SDLK_S) {
			InputPorts[1] &= ~0b00000100;
		}
		if (e.key == SDLK_LEFT) {
			InputPorts[1] &= ~0b00100000;
		}
		if (e.key == SDLK_RIGHT) {
			InputPorts[1] &= ~0b01000000;
		}
		if (e.key == SDLK_UP || e.key == SDLK_SPACE) {
			InputPorts[1] &= ~0b00010000;
		}

		// player 2
		if (e.key == SDLK_S) {
			InputPorts[1] &= ~0b00000010;
		}
		if (e.key == SDLK_A) {
			InputPorts[2] &= ~0b00100000;
		}
		if (e.key == SDLK_D) {
			InputPorts[2] &= ~0b01000000;
		}
		if (e.key == SDLK_W) {
			InputPorts[2] &= ~0b00010000;
		}
	}

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }

    return SDL_APP_CONTINUE;
}

u8 MachineIN(State8080 *cpu, u8 port)
{
	if (port == 3) {
		return ((app.shift_register >> (8-app.shift_offset)) & 0xFF);
	}

	return InputPorts[port];
}

void MachineOUT(State8080 *cpu, u8 port)
{
	// if (port != 6) // Watch dog is noisy.
	// 	SDL_Log("OUT: Port: %d", port);

	if (port == 2) {
		app.shift_offset = cpu->a & 0x7;
		return;
	}
	if (port == 4) {
		app.shift_register = (cpu->a << 8) | (app.shift_register >> 8);
		return;
	}
	OutputPorts[port] = cpu->a;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
	register clock_t Now = clock();
	local_persist clock_t LastInterruptTime;
	local_persist clock_t LastRenderTime;
	local_persist bool AlternateInterrupt;
	local_persist clock_t PreviousNow;

	// hmmm wallclock is definitely better here. time(NUlL) calls were too expensive.
	if (app.cpu.interrupt_enabled && (Now - LastInterruptTime) > 9000) {
		if (AlternateInterrupt) {
			GenerateInterrupt(&app.cpu, 2); // Interrupt "0x10"
			// SDL_Log("Interrupt 0x10");
		}
		else {
			GenerateInterrupt(&app.cpu, 1); // Interrupt "0x8"
			// SDL_Log("Interrupt 0x08");
		}
		AlternateInterrupt = !AlternateInterrupt;
		LastInterruptTime = clock();
	}
	
	// @TODO: fix
	double MillisecondDifference = Now - PreviousNow;
	int cycles_to_catch_up = 2 * MillisecondDifference;
	int cycles = 0;

	while(cycles_to_catch_up > cycles)
	{
		u8 *opcode = &app.cpu.memory[app.cpu.pc];
		if (*opcode == 0xDB) // IN
		{
			if (app.cpu.debug)
				Debug(&app.cpu);
			u8 port = opcode[1];
			app.cpu.a = MachineIN(&app.cpu, port);

			if (app.cpu.debug) {
				char *bin = (char *)malloc(sizeof(char[9]));
				ByteToBinary(app.cpu.a, bin);
				SDL_Log("IN: port:%d bin: %s\n", port, bin);
				free(bin);
			}

			app.cpu.pc += 2;
			cycles += 3;
		}
		else if (*opcode == 0xD3) // OUT
		{
			if (app.cpu.debug)
				Debug(&app.cpu);
			u8 port = opcode[1];
			MachineOUT(&app.cpu, port);

			// This is really noisy because it's using the emulated shift
			// hardware.
			// SDL_Log("OUT happened: a: %d\n", app.cpu.a);

			app.cpu.pc += 2;
			cycles += 3;
		}
		else {
			cycles += Emulate8080Op(&app.cpu);
		}
	}

	PreviousNow = Now;

	if((Now - LastRenderTime) > 9000) {
		RenderScreen();
		LastRenderTime = clock();
		// SDL_Log("rendered");
    }

    return SDL_APP_CONTINUE;
}

void RenderScreen()
{
	// @see: https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html
	// @note: Video RAM is 2400-3FFF
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

