#include "win32_8080.h"

// @TODO: run the emulator, copy pixels from RAM (to our buffer) and blit to screen.

global_variable bool GlobalRunning;
global_variable bool GlobalPause;
global_variable win32_buffer GlobalBuffer;
global_variable State8080 GlobalCPU;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

internal void
Win32ResizeDIBSection(win32_buffer *Buffer, int Width, int Height)
{
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;

    int BytesPerPixel = 4;
    Buffer->BytesPerPixel = BytesPerPixel;

    // NOTE(casey): When the biHeight field is negative, this is the clue to
    // Windows to treat this bitmap as top-down, not bottom-up, meaning that
    // the first three bytes of the image are the color for the top left pixel
    // in the bitmap, not the bottom left!
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
    Buffer->MemorySize = BitmapMemorySize;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Width*BytesPerPixel;
}

internal void
Win32DisplayBufferInWindow(win32_buffer *Buffer,
                           HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    StretchDIBits(DeviceContext,
                  /*
                  X, Y, Width, Height,
                  X, Y, Width, Height,
                  */
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, Buffer->Width, Buffer->Height,
                  Buffer->Memory,
                  &Buffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

internal void
Win32ProcessPendingMessages()
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                uint32 VKCode = (uint32)Message.wParam;
                bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                bool32 IsDown = ((Message.lParam & (1 << 31)) == 0);
                if(WasDown != IsDown)
                {
                    if(VKCode == 'W')
                    {
			// @TODO: Implement!
                    }
                    else if(VKCode == 'A')
                    {
			// @TODO: Implement!
                    }
                    else if(VKCode == 'S')
                    {
			// @TODO: Implement!
                    }
                    else if(VKCode == 'D')
                    {
			// @TODO: Implement!
                    }
                    else if(VKCode == VK_UP)
                    {
			// @TODO: Implement!
                    }
                    else if(VKCode == VK_LEFT)
                    {
			// @TODO: Implement!
                    }
                    else if(VKCode == VK_DOWN)
                    {
			// @TODO: Implement!
                    }
                    else if(VKCode == VK_RIGHT)
                    {
			// @TODO: Implement!
                    }
                    else if(VKCode == 'Q')
                    {
			GlobalRunning = false;
			printf("bye bye");
                    }
                    else if(VKCode == VK_ESCAPE)
                    {
			GlobalRunning = false;
			printf("bye bye");
                    }
                    else if(VKCode == VK_SPACE)
                    {
			// @TODO: Implement!
                    }
#if HANDMADE_INTERNAL
                    else if(VKCode == 'P')
                    {
                        if(IsDown)
                        {
                            GlobalPause = !GlobalPause;
                        }
                    }
#endif
                }

                bool32 AltKeyWasDown = (Message.lParam & (1 << 29));
                if((VKCode == VK_F4) && AltKeyWasDown)
                {
                    GlobalRunning = false;
                }
            } break;

            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}

const int Width = 256; // 256
const int Height = 256; // 224

void RotatePixels(u32 *Pixels)
{
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"i got no class";

    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND Window = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Space Invaders Emulator",    // Window text
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, Width, Height,

        NULL,       // Parent window
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );

    if (Window == NULL)
    {
        return 0;
    }

    ShowWindow(Window, nCmdShow);

    HDC DeviceContext = GetDC(Window);

    // The raster resolution is 256x224 at 60Hz. The monitor is rotated in the cabinet 90 degrees counter-clockwise.
    Win32ResizeDIBSection(&GlobalBuffer, Width, Height);

    InitCPU(&GlobalCPU);
    if(LoadROMFile(&GlobalCPU, "W:\\chip-8\\rom\\invaders") == 0)
    {
	exit(1);
    }

    // @TODO: 60Hz limiting

    u8 *Row = (u8 *) GlobalBuffer.Memory;
    for(int Y=0;
	    Y < Height;
	    ++Y)
    {
	u32 *Pixel = (u32 *)Row;
	for(int X=0;
		X < Width;
		++X)
	{
	    *Pixel++ = 0x00000000;
	}
	Row += GlobalBuffer.Pitch;
    }

    GlobalRunning = 1;
    // GlobalCPU.DebugPrint = 1;
    while(GlobalRunning)
    {
	Win32ProcessPendingMessages();
	Emulate8080Op(&GlobalCPU);
	// @TODO: timing based on cycles?
	// if((GlobalCPU.Steps%1000) == 0)
	// {
	//     Sleep(1.0f);
	// }
	// @see: https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html
	// NOTE: Video RAM is 2400-3FFF
	// The screens pixels are on/off (1 bit each). 256*224/8 = 7168 (7K) bytes.
	u8 *VideoRAM = GlobalCPU.memory + 0x2400;

	// @TODO: Slowly redrawing the screen gives a cool effect and slows down the CPU.
	// Should Sleep(MS) instead but this is amusing.
	if((GlobalCPU.Steps%10000) ==0 || false)
	{
	u8 *Row = (u8 *) GlobalBuffer.Memory;
	for(int Y=0;
		Y < 224; // hardcoding for now
		++Y)
	{
	    u32 *Pixel = (u32 *)Row;
	    for(int X=0;
		    X < Width/8;
		    ++X)
	    {

		if((VideoRAM - GlobalCPU.memory) > 0x3FFF)
		{
		    printf("[ERROR] VideoRAM overflowed: 0x%x", (VideoRAM - GlobalCPU.memory));
		    exit(1);
		}

		// @TODO: How to rotate 90deg?
		// i learned the maffs for this, rotating a point around an origin.
		// it'd be swapping the X and Y, and changing signs.
		for(int I=0;
			I<8;
			++I)
		{
		    if((*VideoRAM >> I) & 1)
			*Pixel++ = 0xFF00FF00;
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

		VideoRAM += 1;
	    }
	    Row += GlobalBuffer.Pitch;
	}

#if 1
	{

	    // rotating this is gonna be weird because the col and row sizes are different...
	    // that must have been the issue, ahhh
	    // THAT WAS THE ISSUE, THE _WHOLE_ TIME.
	    // i knew the logic checked out.
	u8 *NewBuffer = (u8 *) calloc(1, GlobalBuffer.MemorySize);
	int RX=0;
	int RY=0;
	for(int X=Width-1;
		X>=0;
		--X)
	{
	    for(int Y=0;
		    Y<Height;
		    ++Y)
	    {
		u32 *Pixel = (u32 *)((u8*)GlobalBuffer.Memory + (X*GlobalBuffer.BytesPerPixel) + (Y*GlobalBuffer.Pitch));
		u32 *TransposedPixel = (u32 *)(NewBuffer + (RX++*GlobalBuffer.BytesPerPixel) + (RY*GlobalBuffer.Pitch));
		*TransposedPixel = *Pixel;
		if (*TransposedPixel > 0)
		    *TransposedPixel = 0xFFFF0000;
	    }
	    RY++;
	    RX = 0;
	}

	for(int I=0;
		I<GlobalBuffer.MemorySize;
		++I)
	{
	    u8 *Buffer = (u8 *)GlobalBuffer.Memory;
	    *(Buffer+I) = *(NewBuffer+I);
	}
	free(NewBuffer);
	}
#endif

	Win32DisplayBufferInWindow(&GlobalBuffer, DeviceContext, Width, Height);
	}

    }

    return 0;
}

const HBRUSH GRAY = CreateSolidBrush(RGB(128,128,128));

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);
}


LRESULT CALLBACK WindowProc(HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
	GlobalRunning = false;
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(Window, &ps);

            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
	    StretchDIBits(hdc,
			  /*
			  X, Y, Width, Height,
			  X, Y, Width, Height,
			  */
			  0, 0, Dimension.Width, Dimension.Height,
			  0, 0, GlobalBuffer.Width, GlobalBuffer.Height,
			  GlobalBuffer.Memory,
			  &GlobalBuffer.Info,
			  DIB_RGB_COLORS, SRCCOPY);

            EndPaint(Window, &ps);
        }
        return 0;

    }
    return DefWindowProc(Window, uMsg, wParam, lParam);
}
