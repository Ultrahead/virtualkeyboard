/**
 * @file  def.h
 * @brief This file contains macro definitions and constants used throughout the VirtualKeyboard project.
 *
 * @note This file should be compiled using C++11.
 */
#ifndef _DEF_H_
#define _DEF_H_

/**
 * @brief Macro used to inhibit or suppress pecific code.
 *
 * @param X Specifies the code to inhibit. For example, INHIBIT(SDL_Delay(100);).
 */
#define INHIBIT(X)

/**
 * @brief Macros used to specify the pixels per unit in both, x and y axes.
 */
#define PPU_MULTIPLIER_X 3
#define PPU_MULTIPLIER_Y 3

/**
 * @brief Macro used to indicate the relative path to the folder with the resources used by this program.
 *
 * @param X Specifies the relative path to the resources.
 */
#define RES_DIR_NAME "res/"

/**
 * @brief Macro used to indicate whether the caret must blink or always stay visible.
 *
 * @param X If TRUE, the caret will blink. Otherwise, the caret will always stay visible.
 */
#define CARETTICKS true

/**
 * @brief Macro that indicates the caret blinking time when the latter is activated.
 *
 * @param X Specifies the amount of milliseconds between visibilty changes.
 */
#define CARETTICKTIME 500

/**
 * @brief Macro that indicates whether the keyboard must autoscale when the initial resolution differs from the default one.
 *   
 * @param X If TRUE, keyboard rendering size will autoscale.
 */
#define AUTOSCALE true

/**
 * @brief Macros that indicate the screen's width and height to take as a reference by default.
 *
 * @param X The reference size, respectively, in pixels. By default, it is set to 1280x720 pixels to match the TSP's screen.
 */
#define SCREEN_WIDTH_REFERENCE 1280
#define SCREEN_HEIGHT_REFERENCE 720

/**
 * @brief Macros that indicate the screen size used to render the keyboard, by default.
 *
 * @param X The reference width and height, respectively, in pixels. By default, it is set to 1280x720 pixels to match the TSP's screen.
 *			 But it can be changed to 1024x768 pixels to match the Trimui-Brick's screen.
 */
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

/**
 * @brief Macros that indicates the screen's opacity.Note: there is no guarantee for opacity to work on all platforms (in the TSP it does not).
 *
 * @param X The reference size, respectively, in pixels. By default, it is set to 1280x720 pixels to match the TSP's screen.
 */
#define SCREEN_OPACITY 1.00f

/**
 * @brief Macro that indicates the desired amount of milliseconds per frame.
 *
 * @param X The amount of milliseconds per frame.
 */
#define MS_PER_FRAME 33

/**
 * @brief Macros that indicate the height of header and footer, respectively.
 *
 * @param X The size, in pixels of header/footer, respectively.
 */
#define HEADER_HEIGHT 17
#define FOOTER_HEIGHT 13

/**
 * @brief Macros that indicate the size of border and margin, respectively.
 *
 * @param X The size, in pixels of header/footer, respectively.
 */
#define DIALOG_BORDER 2
#define DIALOG_MARGIN 8

/**
 * @brief Macros that set colors to used in different parts of the keyboard.
 *
 * @param X The color in RGB format.
 */
#define COLOR_TITLE_BG 102, 85, 74
#define COLOR_TEXT_NORMAL 70, 27, 10
#define COLOR_TEXT_TITLE 233, 229, 227
#define COLOR_TEXT_DIR 75, 70, 164
#define COLOR_TEXT_SELECTED 255, 0, 0
#define COLOR_CURSOR 232, 152, 80
#define COLOR_BG_1 255, 255, 255
#define COLOR_BG_2 232, 228, 224
#define COLOR_BG_3 52, 52, 52
#define COLOR_BORDER 102, 85, 74

/**
 * @brief Macros that map specific SDL input codes with the ones used by this program.
 *
 * @param X The input mapping.
 */
#define MYKEY_UP SDL_KeyCode::SDLK_UP
#define MYKEY_RIGHT SDL_KeyCode::SDLK_RIGHT
#define MYKEY_DOWN SDL_KeyCode::SDLK_DOWN
#define MYKEY_LEFT SDL_KeyCode::SDLK_LEFT
#define MYKEY_SYSTEM SDL_KeyCode::SDLK_q
#define MYKEY_PAGEUP SDL_KeyCode::SDLK_PAGEUP
#define MYKEY_PAGEDOWN SDL_KeyCode::SDLK_PAGEDOWN
#define MYKEY_OPEN SDL_KeyCode::SDLK_RETURN
#define MYKEY_PARENT SDL_KeyCode::SDLK_BACKSPACE
#define MYKEY_OPERATION SDL_KeyCode::SDLK_a
#define MYKEY_SELECT SDL_KeyCode::SDLK_INSERT
#define MYKEY_TRANSFER SDL_KeyCode::SDLK_w
#define MYKEY_CARETLEFT SDL_KeyCode::SDLK_HOME
#define MYKEY_CARETRIGHT SDL_KeyCode::SDLK_END

#endif // _DEF_H_
