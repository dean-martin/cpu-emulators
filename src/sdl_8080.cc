#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "8080.h"

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

    VideoPixels = (u8 *)calloc(1, 256*224*8);
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
    if (event->type == SDL_EVENT_KEY_DOWN ||
        event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    // SDL_Delay(0.001);
    // GlobalCPU.DebugPrint = 1;
    Emulate8080Op(&GlobalCPU);

    if((GlobalCPU.Steps%10000) == 0)
    {
        // @see: https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html
        // NOTE: Video RAM is 2400-3FFF
        // The screens pixels are on/off (1 bit each). 256*224/8 = 7168 (7K) bytes.
        u8 *VideoRAM = GlobalCPU.memory + 0x2400;

        u32 *Pixel = (u32 *)VideoPixels;
        for(int Y=0;
                Y < 224;
                ++Y)
        {
            for(int X=0;
                    X < 256/8;
                    ++X, ++VideoRAM)
            {
                if((VideoRAM - GlobalCPU.memory) > 0x3FFF)
                {
                        printf("[ERROR] VideoRAM overflowed: 0x%x", (VideoRAM - GlobalCPU.memory));
                        exit(1);
                }

                for(int I=0;
                        I<8;
                        ++I)
                {
                        if((*VideoRAM >> I) & 1)
                                *Pixel++ = 0x0000FF00;
                        else
                                *Pixel++ = 0x00000000;
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
        for(int X=256-1;
                X>=0;
                --X, ++Y2)
        {
            for(int Y=0;
                    Y<224;
                    ++Y, X2++)
            {
                u32 *GamePixel = (u32 *)(VideoPixels + (X*4) + (Y*256*4));
                if (*GamePixel)
                {
                    SDL_RenderPoint(renderer, X2, Y2);
                }
            }
            X2 = 0;
        }

        SDL_RenderPresent(renderer);
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}

