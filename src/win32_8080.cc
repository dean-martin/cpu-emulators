#include "win32_8080.h"

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

const int WindowWidth = 290;
const int WindowHeight = 360;

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
        (GetSystemMetrics(SM_CXSCREEN)/2)-WindowWidth, (GetSystemMetrics(SM_CYSCREEN)/2)-WindowHeight,
	 WindowWidth, WindowHeight,

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
    Win32ResizeDIBSection(&GlobalBuffer, WindowWidth, WindowHeight);

    InitCPU(&GlobalCPU);
    if(LoadROMFile(&GlobalCPU, "W:\\chip-8\\rom\\invaders") == 0)
    {

		exit(1);
    }

    // @TODO: 60Hz limiting?

    u8 *Row = (u8 *) GlobalBuffer.Memory;
    for(int Y=0;
	    Y < WindowHeight;
	    ++Y)
    {
		u32 *Pixel = (u32 *)Row;
		for(int X=0;
			X < WindowWidth;
			++X)
		{
			*Pixel++ = 0x00000000;
		}
		Row += GlobalBuffer.Pitch;
    }

	u8 *VideoPixels = (u8 *)calloc(1, 256*224*8);
	if (!VideoPixels)
	{
		printf("[ERROR] failed to alloc VideoPixels\n");
		exit(1);
	}

    GlobalRunning = 1;
    // GlobalCPU.DebugPrint = 1;
	// 16bit
	// @see: https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html#dedicated-shift-hardware
	local_persist u16 x16;
	local_persist u16 y16;
	local_persist u8 ShiftOffset;
    while(GlobalRunning)
    {
		Win32ProcessPendingMessages();

		u8 *opcode = &GlobalCPU.memory[GlobalCPU.pc];
		if (*opcode == 0xDB) // IN
		{
			u8 port = opcode[1];
			switch(port)
			{
				case 1:
				{
					GlobalCPU.a = 0b00001101;
					GlobalCPU.a = 1;
				} break;
				case 2:
				{
					GlobalCPU.a = 0;
				} break;
				case 3:
				{
					u16 value = (x16 << 8) | y16;
					GlobalCPU.a = ((value >> (8-ShiftOffset) & 0xFF));
				} break;
			}
			GlobalCPU.pc += 2;
		}
		else if (*opcode == 0xD3) // OUT
		{

			u8 port = opcode[1];
			switch(port)
			{
				// Writing to port 2 (bits 0,1,2) sets the offset for the 8 bit result, eg.
				case 2:
				{
					ShiftOffset = ((x16 << 8) | y16) & 0x7;
				} break;
				// Writing to port 4 shifts x into y, and the new value into x, eg.
				case 4:
				{
					y16 = x16;
					x16 = *opcode;
				} break;
			}
			// @TODO:
			GlobalCPU.pc += 2;
		}
		else
			Emulate8080Op(&GlobalCPU);


		// @TODO: Sleep(MS) based on cycles elapsed.

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

		u8 *Row = (u8 *)GlobalBuffer.Memory;
		for(int X=256-1;
				X>=0;
				--X)
		{
			u32 *Pixel = (u32 *)Row;
			for(int Y=0;
					Y<224;
					++Y)
			{
				u32 *GamePixel = (u32 *)(VideoPixels + (X*4) + (Y*256*4));
				*Pixel++ = *GamePixel;
			}
			Row += GlobalBuffer.Pitch;
		}

	    Win32DisplayBufferInWindow(&GlobalBuffer, DeviceContext, WindowWidth, WindowHeight);
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
