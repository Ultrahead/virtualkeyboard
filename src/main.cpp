/**
 * @file  main.cpp
 * @brief Implementation file for the static funtions for the main class.
 */

#ifdef _WIN64

#include <io.h>
#define access _access

#else

#include <unistd.h>

#endif // _WIN64

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "def.h"
#include "screen.h"
#include "sdlUtils.h"
#include "resourceManager.h"
#include "keyboard.h"
#include "main.h"

int main(int argc, char** argv)
{
	// 1. Initialize SDL with the corresponding flags and early exit on fail (return 1).
	// 2. Initialize the joystick, if found.
	// 3. Initialize the screen and early exit on fail (return 1).
	// 4. Initialize resources and early exit on fail (return 1).
	// 5. Initialize and open the virtual keyboard.
	// 6. Clean up SDL resources.
	// 7. Return the result of the keyboard execution (where 0 is Ok).

	if (initSDL() == false) return 1;

	initJoystick();

	if (initScreen() == false) return 1;

	if (initResources(argc, argv) == false) return 1;

	const int result = initKeyboard(argc, argv);

	SDL_Utils::cleanupAndQuit();

	return result;
}

const bool initSDL(void)
{
	// 1. Disable the screen cursor.
	// 2. Set environment flag to disable mouse, so as to avoid crashes if absent.
	// 3. Initialize SDL with the corresponding flags and early exit on fail (return FALSE).
	// 4. Clear any SDL-error message and return TRUE (success).

	SDL_Log("Initializing SDL ...");
	SDL_ShowCursor(SDL_DISABLE);

#ifdef _WIN64
	if (_putenv("SDL_NOMOUSE=1") == 0)
#else
	if(setenv("SDL_NOMOUSE", "1", true) == 0)
#endif // _WIN64
	{
		SDL_Log("SDL_NOMOUSE set successfully.");
	}
	else
	{
		SDL_LogError(0, "Could not set SDL_NOMOUSE: %s", SDL_GetError());
	}

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) == 0)
	{
		SDL_Log("SDL initialized successfully.");
	}
	else
	{
		SDL_LogError(0, "SDL initialization failed: %s", SDL_GetError());
		return false;
	}

	SDL_ClearError();
	return true;
}

void initJoystick(void)
{
	// 1. Call SDL_NumJoysticks() to get the number of joysticks connected to the system.
	// 2. If no joysticks are found, log a warning and return early.
	// 3. Attempt to open the first joystick (index = 0).
	// 4. If successful, log the joystick's name and its key characteristics. And return.
	// 5. If it fails, log a warning indicating that the joystick could not be opened.

	SDL_Log("Initializing joysticks ...");

	if (SDL_NumJoysticks() == 0)
	{
		SDL_LogWarn(0, "No joystick found!");
		return;
	}

	if (SDL_Joystick* l_joystick = SDL_JoystickOpen(0))
	{
		SDL_Log("Opened Joystick 0 ...");
		SDL_Log("  Name: %s", SDL_JoystickNameForIndex(0));
		SDL_Log("  Number of Axes: %d", SDL_JoystickNumAxes(l_joystick));
		SDL_Log("  Number of Buttons: %d", SDL_JoystickNumButtons(l_joystick));
		SDL_Log("  Number of Balls: %d", SDL_JoystickNumBalls(l_joystick));
		return;
	}

	SDL_LogWarn(0, "Could NOT open Joystick 0!\n");
}

const bool initScreen(void)
{
	// 1. Get the number of video displays.
	// 2. Iterate over each display to find the best resolution and refresh rate.
	// 3. Log the best resolution found and also the current resolutions chosen.
	// 4. Calculate the adjusted pixels-per-unit (PPU) if auto-scaling is enabled.
	// 5. Log the adjusted PPU and whether auto-scaling is on or off.
	// 6. Create an SDL window with the actual specified width and height.
	// 15. Check if the window surface was created successfully and early exit if not (return FALSE).
	// 16. Try to set the window opacity and log a warning if it fails (Note: this is not portable).
	// 17. Return TRUE indicating that the screen initialization was successful.

	SDL_Rect l_best{ 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	const int l_displayCount = SDL_GetNumVideoDisplays();

	SDL_Log("Number of displays: %i", l_displayCount);

	for (int l_displayIndex(0); l_displayIndex < l_displayCount; ++l_displayIndex)
	{
		SDL_Log("  Display %i:", l_displayIndex);

		int l_modeCount(SDL_GetNumDisplayModes(l_displayIndex));
		int l_bestRefreshRate(0);

		for (int l_modeIndex(0); l_modeIndex < l_modeCount; ++l_modeIndex)
		{
			SDL_DisplayMode l_mode{ SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };

			if (SDL_GetDisplayMode(l_displayIndex, l_modeIndex, &l_mode) == 0)
			{
				SDL_Log("    %i bpp\t%i x %i @ %iHz", SDL_BITSPERPIXEL(l_mode.format), l_mode.w, l_mode.h, l_mode.refresh_rate);

				if (l_displayIndex == 0 && (l_best.w < l_mode.w || (l_best.w == l_mode.w && l_bestRefreshRate < l_mode.refresh_rate)))
				{
					l_best.w = l_mode.w;
					l_best.h = l_mode.h;
					l_bestRefreshRate = l_mode.refresh_rate;
				}
			}
		}
	}

	SDL_Log("Best resolution: %i x %i", l_best.w, l_best.h);
	SDL_Log("Current resolution: %i x %i", Globals::g_Screen.m_logicalWidth, Globals::g_Screen.m_logicalHeight);

#if AUTOSCALE

	const float l_adjustedPpu = std::min
	(
		Globals::g_Screen.m_logicalWidth / static_cast<float>(SCREEN_WIDTH_REFERENCE),
		Globals::g_Screen.m_logicalHeight / static_cast<float>(SCREEN_HEIGHT_REFERENCE)
	);

	Globals::g_Screen.m_ppuX = Globals::g_Screen.m_ppuY = l_adjustedPpu;

	const char* l_autoScaleText = "On";

#else

	const char* l_autoScaleText = "Off";

#endif

	SDL_Log("Adjusted ppu with auto-scaling %s: %f x %f", l_autoScaleText, Globals::g_Screen.m_ppuX, Globals::g_Screen.m_ppuY);

	Globals::g_sdlwindow = SDL_CreateWindow("Virtual Keyboard",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		Globals::g_Screen.m_actualScreenWidth, Globals::g_Screen.m_actualScreenHeight,
		SDL_WINDOW_OPENGL);
	Globals::g_screen = SDL_GetWindowSurface(Globals::g_sdlwindow);

	if (Globals::g_screen == nullptr)
	{
		SDL_LogError(0, "Could not create screen surface: %s", SDL_GetError());
		return false;
	}

	if (SDL_SetWindowOpacity(Globals::g_sdlwindow, SCREEN_OPACITY) > 0)
	{
		SDL_LogWarn(0, "Could not adjust window opacity %s", SDL_GetError());
	}

	return true;
}

const bool initResources(const int argc, char** const argv)
{
	// 1. Initialize the TTF-based resource library.
	// 2. If initialization fails, log an error message and early exit (return FALSE).
	// 3. Initialize the resource manager, passing argument count and values.
	// 4. If initialization fails, log an error message and early exit (return FALSE).
	// 5. Return TRUE, indicating successful initialization of resources.

	if (TTF_Init() == -1)
	{
		SDL_LogError(0, "Initialization of TTF failed: %s", SDL_GetError());
		return false;
	}

	if (CResourceManager::instance().init(argc, argv) == false)
	{
		SDL_LogError(0, "Resource initialization failed: %s", SDL_GetError());
		return false;
	}

	return true;
}

const int initKeyboard(const int argc, char** const argv)
{
	// 1. Create an instance of the virtual keyboard, passing the third commandline argument if available (otherwise, an empty string).
	// 2. Execute the keyboard and check if execution succeeds (result equals 1).
	// 3. If successful, clear the console screen (using system-specific commands).
	// 4. Retrieve the input text from the keyboard and check if it is empty.
	// 5. Format the output string by wrapping the input text with "[VKStart]" and "[VKEnd]".
	// 6. Print the whole formatted output string to the console and return a value indicating success (0).
	// 7. If the keyboard execution was not successful, log a warning message and return 1.

	CKeyboard l_keyboard(argc < 4 ? "" : argv[3]);

	if (l_keyboard.execute() == 1)
	{
#ifdef _WIN64

		std::system("cls");

#else

		std::system("clear");

#endif

		std::string text{ l_keyboard.getInputText() };

		if (text.empty())
		{
			text = "";
		}

		const std::string output{ "[VKStart]" + l_keyboard.getInputText() + "[VKEnd]" };

		std::cout << output << std::endl; // To avoid that other processes or threads to chime in, always output a whole string
		return 0;
	}

	SDL_LogWarn(0, "Input operation aborted!");

	return 1;
}

