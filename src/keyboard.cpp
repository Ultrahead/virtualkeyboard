/**
 * @file  keyboard.cpp
 * @brief Implementation file for the CKeyboard class.
 */

#include <iostream>
#include "keyboard.h"
#include "screen.h"
#include "sdlUtils.h"
#include "resourceManager.h"
#include "def.h"

 /*
  * @brief Constant expressions used to properly render the virtual keeyboard on screen.
  */
constexpr auto KEYROWS = 4;
constexpr auto KEYCOLUMNS = 13;
constexpr auto TOTALKEYS = KEYROWS * KEYCOLUMNS;

/*
 * @brief Macros used to properly render the virtual keeyboard on screen.
 */
#define KB_WIDTH	   static_cast<Sint16>(265 * Globals::g_Screen.getAdjustedPpuX())
#define KB_HEIGHT	   static_cast<Sint16>(104 * Globals::g_Screen.getAdjustedPpuY())
#define KB_X           static_cast<Sint16>((Globals::g_Screen.m_logicalWidth - KB_WIDTH) >> 1)
#define KB_Y           static_cast<Sint16>((Globals::g_Screen.m_logicalHeight - KB_HEIGHT - (10 + FOOTER_HEIGHT) * Globals::g_Screen.getAdjustedPpuY()))
#define FIELD_WIDTH    static_cast<Sint16>(KB_WIDTH - 8 * Globals::g_Screen.getAdjustedPpuX())
#define FIELD_Y        static_cast<Sint16>(KB_Y - 25 * Globals::g_Screen.getAdjustedPpuY())

namespace
{
    /*
     * @brief Indicates the last key that was selected on the first row of keys. Used for vertical looping.
     */
	int m_lastKeySelectedFirstRow = 0;

    /*
     * @brief Indicates the last key that was selected on the last row of keys. Used for vertical looping.
     */
    int m_lastKeySelectedLastRow = TOTALKEYS - KEYCOLUMNS;

#if CARETTICKS == true

    /*
     * @brief Timer used to make the caret blink. It is overriden when the caret must be shown.
     */
	SDL_TimerID m_timerId = 0;

#endif
}

#if CARETTICKS == true

static Uint32 changeCaretVisibility(Uint32 interval, void* param)
{
    // 1. Try to cast the param as a keyboard.
    // 2. If it fails, abort and wait for the next tick.
    // 3. If it succeeds, assess whether the caret must be rendered or not in the frame.
    // 4. Keep the timer running by returning the tick time.

    CKeyboard* l_thisKeyboard = static_cast<CKeyboard*>(param);

    if (l_thisKeyboard == nullptr)
    {
        return CARETTICKTIME;
    }

    l_thisKeyboard->m_showCaret = l_thisKeyboard->m_mustShowCaret ? true : !(l_thisKeyboard->m_showCaret);

    /*
     * Thise code does not run in the main thread so it fails on the TSP (that is why we need a show caret variable)
     * 
    CWindow* l_thisWindow = static_cast<CWindow*>(param);

    if (l_thisWindow == nullptr)
    {
        return CARETTICK;
    }

    l_thisWindow->render(true);
    SDL_UpdateWindowSurface(Globals::g_sdlwindow);
    SDL_Log("Timer Ok: %d", l_thisKeyboard->m_showCaret);
    */

    return CARETTICKTIME;
}

#endif

CKeyboard::CKeyboard(const std::string &p_inputText):
    CWindow(),
    m_imageKeyboard(nullptr),
    m_textField(nullptr),
    m_inputText(p_inputText),
    m_selected(0),
    m_footer(nullptr),
    m_keySet(0),
	m_showCaret(true),
    m_mustShowCaret(false),
    m_caretPosition(p_inputText.length()),
    m_font(CResourceManager::instance().getFont())
{
    // Steps:
    // 1. Define key sets for the keyboard (lowercase and uppercase with special characters).
    // 2. Retrieve screen-scaling factors (adjusted PPU values) and keyboard dimensions.
    // 3. Create the keyboard background image:
    //    a. Attempt to load a predefined background image.
    //    b. If unavailable, create an alternative background with a solid color.
    // 4. Create the caret image for text input.
    // 5. Create the keyboard surface and fill it with a border and background color.
    // 6. Render individual keys on the keyboard by looping through rows and columns to position and style each key.
    // 7. Create the "Cancel" button background and style it.
    // 8. Create the "OK" button background and style it.
    // 9. Create the text-field image for displaying input text.
    // 10. Create the footer image and add instructional text.
    // 11. If caret blinking is enabled, initialize a timer for caret visibility toggling.

    // Key sets
    m_keySets[0] = "1234567890-=«qwertyuiop[]`asdfghjkl;'\\©zxcvbnm,./£ñ ";
    m_keySets[1] = "!@#$%^&*()_+«QWERTYUIOP{}~ASDFGHJKL:\"|®ZXCVBNM<>?¿Ñ ";

    const float l_adjustedPpuX = Globals::g_Screen.getAdjustedPpuX();
    const float l_adjustedPpuY = Globals::g_Screen.getAdjustedPpuY();
    const int l_keyboardWidth = KB_WIDTH;
    const int l_keyboardHeight = KB_HEIGHT;

    // Create keyboard image
    {
        SDL_Rect l_rect{};
        l_rect.w = Globals::g_Screen.m_logicalWidth;
        l_rect.h = Globals::g_Screen.m_logicalHeight;
        l_rect.x = 0;
        l_rect.y = 0;
        SDL_Surface* l_imageBakground = CResourceManager::instance().getSurface(CResourceManager::T_SURFACE_BACKGROUND);

        if (l_imageBakground != nullptr)
        {
            SDL_BlitScaled(l_imageBakground, nullptr, Globals::g_screen, &l_rect);
        }
        else
        {
            SDL_Surface* l_imageAlternativeBackground = SDL_Utils::createImage(l_rect.w, l_rect.h, SDL_MapRGB(Globals::g_screen->format, COLOR_BG_3));
            SDL_Utils::applySurface(0, 0, l_imageAlternativeBackground, Globals::g_screen);
            SDL_FreeSurface(l_imageAlternativeBackground);
            l_imageAlternativeBackground = nullptr;
        }

        m_caret = SDL_Utils::createImage(static_cast<Sint16>(1 * l_adjustedPpuX), static_cast<int>((3 + FONT_SIZE) * l_adjustedPpuY), SDL_MapRGB(Globals::g_screen->format, COLOR_BG_3));

        m_imageKeyboard = SDL_Utils::createImage(l_keyboardWidth, l_keyboardHeight, SDL_MapRGB(Globals::g_screen->format, COLOR_BORDER));
        l_rect.w = l_keyboardWidth - static_cast<int>(4 * l_adjustedPpuX);
        l_rect.h = static_cast<int>(100 * l_adjustedPpuY);
        l_rect.x = static_cast<int>(2 * l_adjustedPpuX);
        l_rect.y = static_cast<int>(2 * l_adjustedPpuY);

        SDL_FillRect(m_imageKeyboard, &l_rect, SDL_MapRGB(m_imageKeyboard->format, COLOR_BG_2));
        
        // Create Keys
        for (unsigned int l_y = 0; l_y < KEYROWS; ++l_y)
        {
            for (unsigned int l_x = 0; l_x < KEYCOLUMNS; ++l_x)
            {
                l_rect.x = (3 + 20 * l_x) * static_cast<int>(l_adjustedPpuX);
                l_rect.y = (3 + 20 * l_y) * static_cast<int>(l_adjustedPpuY);
                l_rect.w = static_cast<int>(19 * l_adjustedPpuX);
                l_rect.h = static_cast<int>(18 * l_adjustedPpuY);
                SDL_FillRect(m_imageKeyboard, &l_rect, SDL_MapRGB(m_imageKeyboard->format, COLOR_BORDER));

                l_rect.x += static_cast<int>(1 * l_adjustedPpuX);
                l_rect.y += static_cast<int>(1 * l_adjustedPpuY);
                l_rect.w -= static_cast<int>(2 * l_adjustedPpuX);
                l_rect.h -= static_cast<int>(2 * l_adjustedPpuY);
                SDL_FillRect(m_imageKeyboard, &l_rect, SDL_MapRGB(m_imageKeyboard->format, COLOR_BG_1));
            }
        }

        // Create button "Cancel" (background only)
        l_rect.x = static_cast<int>(3 * l_adjustedPpuX);
        l_rect.y = static_cast<int>(83 * l_adjustedPpuY);
        l_rect.w = static_cast<int>(0.5f * l_keyboardWidth - 3.5f * l_adjustedPpuX);
        l_rect.h = static_cast<int>(18 * l_adjustedPpuY);
        SDL_FillRect(m_imageKeyboard, &l_rect, SDL_MapRGB(m_imageKeyboard->format, COLOR_BORDER));

        l_rect.x += static_cast<int>(1 * l_adjustedPpuX);
        l_rect.y += static_cast<int>(1 * l_adjustedPpuY);
        l_rect.w -= static_cast<int>(2 * l_adjustedPpuX);
        l_rect.h -= static_cast<int>(2 * l_adjustedPpuY);
        SDL_FillRect(m_imageKeyboard, &l_rect, SDL_MapRGB(m_imageKeyboard->format, COLOR_BG_1));

        // Create button "OK" (background only)
        l_rect.w = static_cast<int>(0.5f * l_keyboardWidth - 3.5f * l_adjustedPpuX);
        l_rect.h = static_cast<int>(18 * l_adjustedPpuY);
        l_rect.x = static_cast<int>(l_rect.w + 4 * l_adjustedPpuX);
        l_rect.y = static_cast<int>(83 * l_adjustedPpuY);
        SDL_FillRect(m_imageKeyboard, &l_rect, SDL_MapRGB(m_imageKeyboard->format, COLOR_BORDER));

        l_rect.x += static_cast<int>(1 * l_adjustedPpuX);
        l_rect.y += static_cast<int>(1 * l_adjustedPpuY);
        l_rect.w -= static_cast<int>(2 * l_adjustedPpuX);
        l_rect.h -= static_cast<int>(2 * l_adjustedPpuY);
        SDL_FillRect(m_imageKeyboard, &l_rect, SDL_MapRGB(m_imageKeyboard->format, COLOR_BG_1));

        // Create the text field image
        m_textField = SDL_Utils::createImage(l_keyboardWidth, static_cast<int>(static_cast<int>(19 * Globals::g_Screen.getAdjustedPpuY())), SDL_MapRGB(Globals::g_screen->format, COLOR_BORDER));
        l_rect.x = static_cast<int>(2 * l_adjustedPpuX);
        l_rect.y = static_cast<int>(2 * l_adjustedPpuY);
        l_rect.w = static_cast<int>(l_keyboardWidth - 4 * l_adjustedPpuX);
        l_rect.h = static_cast<int>(15 * l_adjustedPpuY);
        SDL_FillRect(m_textField, &l_rect, SDL_MapRGB(m_imageKeyboard->format, COLOR_BG_1));
    }

    // Create the footer with instructions
    m_footer = SDL_Utils::createImage(Globals::g_Screen.m_logicalWidth, static_cast<int>(FOOTER_HEIGHT * l_adjustedPpuY), SDL_MapRGB(Globals::g_screen->format, COLOR_BORDER));
    SDL_Utils::applyText(Globals::g_Screen.m_logicalWidth >> 1, 1, m_footer, m_font, "A-Press  B-Keyset  Select-OK  Menu-Cancel  L/R-Caret  L2/R2-Edges  Y-Backspace  X-Space", Globals::g_colorTextTitle, {COLOR_TITLE_BG}, SDL_Utils::ETextAlign::CENTER);

    #if CARETTICKS == true

    // If the caret is set for ticking, add a timer.
    m_timerId = SDL_AddTimer(CARETTICKTIME, changeCaretVisibility, this);

    #endif
}

CKeyboard::~CKeyboard(void)
{
    // 1. Remove the timer.
    // 2. Free all SDL resources.

	SDL_RemoveTimer(m_timerId);

    if (m_imageKeyboard != nullptr)
    {
        SDL_FreeSurface(m_imageKeyboard);
        m_imageKeyboard = nullptr;
    }

    if (m_textField != nullptr)
    {
        SDL_FreeSurface(m_textField);
        m_textField = nullptr;
    }

    if (m_textField != nullptr)
    {
        SDL_FreeSurface(m_caret);
        m_caret = nullptr;
    }

    if (m_footer != nullptr)
    {
        SDL_FreeSurface(m_footer);
        m_footer = nullptr;
    }
}

void CKeyboard::render(const bool p_focus) const
{
    // 1. Draw the input text field on the screen.
    // 2. Render the input text, ensuring it fits within the text field.
    //    a. If the text is too long, clip it to fit the visible area.
    //    b. Calculate the caret position based on the visible text.
    // 3. If the caret is visible, draw it at the calculated position.
    // 4. Draw the keyboard background image.
    // 5. Highlight the currently selected key or button.
    //    a. Determine the row and column of the selected key.
    //    b. If either of the buttons 'Cancel' or 'OK' is selected, highlight it instead.
    // 6. Render the text for each key on the keyboard.
    //    a. Handle UTF-8 characters that may require multiple bytes.
    //    b. Highlight the text of the selected key.
    // 7. Render the text for the 'Cancel' and 'OK' buttons.
    // 8. Draw the footer with the instructions to use the keyboard.

    INHIBIT(SDL_Log("CKeyboard::render  fullscreen: %s  focus: %s", isFullScreen(), p_focus);)

    const static int l_keyboardX = KB_X;
    const static int l_keyboardY = KB_Y;
    const static int l_keyboardWidth = KB_WIDTH;
    const static int l_keyboardHeight = KB_HEIGHT;
    const static int l_fieldY = FIELD_Y;
    const static int l_fieldWidth = FIELD_WIDTH;
    const static float l_adjustedPpuX = Globals::g_Screen.getAdjustedPpuX();
    const static float l_adjustedPpuY = Globals::g_Screen.getAdjustedPpuY();

    // 1. Draw input text field
    SDL_Utils::applySurface(l_keyboardX, l_fieldY, m_textField, Globals::g_screen);

    const float l_textAreaLenght = l_fieldWidth - 3 * l_adjustedPpuX;
    int l_caretPositionTmp = (int)m_caretPosition;
    SDL_Rect l_rect{};
    l_rect.y = 0;
    l_rect.w = l_fieldWidth;

    // 2. Draw main parts of the keyboard
    {
        // 2a. Render input text
        if (m_inputText.empty() == false)
        {
            SDL_Surface* l_surfaceTmp = SDL_Utils::renderText(m_font, m_inputText, Globals::g_colorTextNormal, { COLOR_BG_1 });

            const std::string l_subString = m_inputText.substr(0, l_caretPositionTmp);

            if (TTF_SizeUTF8(m_font, l_subString.c_str(), &l_caretPositionTmp, nullptr) != 0)
            {
                SDL_LogWarn(0, "Could not measure UTF8 string: %s", TTF_GetError());
            }

            l_rect.w = l_fieldWidth;
            l_rect.h = l_surfaceTmp->h;

            if (l_caretPositionTmp > l_textAreaLenght)
            {
                // 2b. Clip text if too long
                const int l_areaDiffOffset = static_cast<int>(l_textAreaLenght - l_caretPositionTmp);
                l_caretPositionTmp = std::min(static_cast<int>(l_textAreaLenght), l_caretPositionTmp);
                l_rect.x = -l_areaDiffOffset;
                SDL_Utils::applySurface(l_keyboardX + static_cast<Sint16>(5 * l_adjustedPpuX), l_fieldY + static_cast<Sint16>(4 * l_adjustedPpuY), l_surfaceTmp, Globals::g_screen, &l_rect);
            }
            else
            {
                l_rect.x = 0;
                l_caretPositionTmp = std::min(l_surfaceTmp->w, l_caretPositionTmp);
                SDL_Utils::applySurface(l_keyboardX + static_cast<Sint16>(5 * l_adjustedPpuX), l_fieldY + static_cast<Sint16>(4 * l_adjustedPpuY), l_surfaceTmp, Globals::g_screen, &l_rect);
            }
        }

        // 3. Draw caret if visible
        if (m_showCaret)
        {
            l_rect.x = 0;
            l_rect.y = 0;
            l_rect.h = m_caret->h;
            SDL_Utils::applySurface(l_caretPositionTmp + l_keyboardX + static_cast<Sint16>(5 * l_adjustedPpuX), l_fieldY + static_cast<Sint16>(4 * l_adjustedPpuY), m_caret, Globals::g_screen, &l_rect);
        }

        // 4. Draw keyboard background
        SDL_Utils::applySurface(l_keyboardX, l_keyboardY, m_imageKeyboard, Globals::g_screen);
    }

    unsigned int selected_letter_x = -1;
    unsigned int selected_letter_y = -1;

    // 5. Highlight selected key or button
    {
        SDL_Rect l_rect{};

        if (m_selected < TOTALKEYS)
        {
            // 5a. Determine selected key's row and column
            if (m_selected >= 3 * KEYCOLUMNS)
            {
                selected_letter_x = m_selected - (3 * KEYCOLUMNS);
                selected_letter_y = 3;
            }
            else if (m_selected >= 2 * KEYCOLUMNS)
            {
                selected_letter_x = m_selected - (2 * KEYCOLUMNS);
                selected_letter_y = 2;
            }
            else if (m_selected >= KEYCOLUMNS)
            {
                selected_letter_x = m_selected - KEYCOLUMNS;
                selected_letter_y = 1;
            }
            else
            {
                selected_letter_x = m_selected;
                selected_letter_y = 0;
            }

            l_rect.x = l_keyboardX + static_cast<int>((4 + selected_letter_x * 20) * l_adjustedPpuX);
            l_rect.y = l_keyboardY + static_cast<int>((4 + selected_letter_y * 20) * l_adjustedPpuY);
            l_rect.w = static_cast<int>(17 * l_adjustedPpuX);
            l_rect.h = static_cast<int>(16 * l_adjustedPpuY);
        }
        else
        {
            // 5b. Highlight Cancel or OK button
            l_rect.x = l_keyboardX + static_cast<int>((4 * l_adjustedPpuX + (m_selected == 1 + TOTALKEYS) * (0.5f * l_keyboardWidth - 2.5f * l_adjustedPpuX)));
            l_rect.y = l_keyboardY + static_cast<int>(84 * l_adjustedPpuY);
            l_rect.w = static_cast<int>(0.5f * l_keyboardWidth - 5.5f * l_adjustedPpuX);
            l_rect.h = static_cast<int>(16 * l_adjustedPpuY);
        }

        SDL_FillRect(Globals::g_screen, &l_rect, SDL_MapRGB(Globals::g_screen->format, COLOR_CURSOR));
    }

    // 6. Render keys' text
    {
        unsigned int l_i(0);
        std::string l_text("");

        for (unsigned int l_y = 0; l_y < KEYROWS; ++l_y)
        {
            for (unsigned int l_x = 0; l_x < KEYCOLUMNS; ++l_x)
            {
                if (checkUtf8Code(m_keySets[m_keySet].at(l_i)))
                {
                    l_text = m_keySets[m_keySet].substr(l_i, 2);
                    l_i += 2;
                }
                else
                {
                    l_text = m_keySets[m_keySet].substr(l_i, 1);
                    l_i += 1;
                }

                const auto p_x = l_keyboardX + static_cast<int>((13 + 20 * l_x) * l_adjustedPpuX);
                const auto p_y = l_keyboardY + static_cast<int>((7 + 20 * l_y) * l_adjustedPpuY);

                SDL_Utils::applyText(p_x, p_y, Globals::g_screen, m_font, l_text, Globals::g_colorTextNormal,
                    selected_letter_x == l_x && selected_letter_y == l_y ? SDL_Color{ COLOR_CURSOR } : SDL_Color{ COLOR_BG_1 }, SDL_Utils::ETextAlign::CENTER);
            }
        }
    }

    // 7. Render 'Cancel' and 'OK' buttons' text
    const Sint16 p_yb = l_keyboardY + static_cast<Sint16>(87 * l_adjustedPpuY);
    SDL_Utils::applyText(l_keyboardX + static_cast<Sint16>(0.25f * l_keyboardWidth + 3 * l_adjustedPpuX), p_yb, Globals::g_screen, m_font, "Cancel", Globals::g_colorTextNormal,
        m_selected == TOTALKEYS ? SDL_Color{ COLOR_CURSOR } : SDL_Color{ COLOR_BG_1 }, SDL_Utils::ETextAlign::CENTER);
    SDL_Utils::applyText(l_keyboardX + static_cast<Sint16>(0.75f * l_keyboardWidth - 3 * l_adjustedPpuX), p_yb, Globals::g_screen, m_font, "OK", Globals::g_colorTextNormal,
        m_selected == 1 + TOTALKEYS ? SDL_Color{ COLOR_CURSOR } : SDL_Color{ COLOR_BG_1 }, SDL_Utils::ETextAlign::CENTER);

    // 8. Draw the footer
    SDL_Utils::applySurface(0, (Globals::g_Screen.m_logicalHeight - m_footer->h), m_footer, Globals::g_screen);
}

const bool CKeyboard::keyPress(const SDL_Event& p_event)
{
    // 1. Call the base class' method to handle any generic key press logic.
    // 2. Initialize a return value to FALSE to indicate no key press was handled yet.
    // 3. Handle different supported key-press events based on the key symbol of the passed event.
    // 4. Update the last selected key in the first or last row for vertical looping, if applicable.
    // 5. Return the result indicating whether the key press was handled (TRUE) or not (FALSE).

    CWindow::keyPress(p_event);
    bool l_returnValue(false);

    static constexpr int s_keyColumnsMinusOneRow = KEYCOLUMNS * (KEYROWS - 1);
    static constexpr int s_keyColumnsMinusOne = KEYCOLUMNS - 1;
    static constexpr int s_totalKeysPlusOne = 1 + TOTALKEYS;

    switch (p_event.key.keysym.sym)
    {
    case MYKEY_UP:
        l_returnValue = moveCursorUp(LOOP_ONKEYPRESS);
        break;
    case MYKEY_DOWN:
        l_returnValue = moveCursorDown(LOOP_ONKEYPRESS);
        break;
    case MYKEY_LEFT:
        l_returnValue = moveCursorLeft(LOOP_ONKEYPRESS);
        break;
    case MYKEY_RIGHT:
        l_returnValue = moveCursorRight(LOOP_ONKEYPRESS);
        break;
    case MYKEY_SYSTEM:
        // Y => Backspace
        l_returnValue = pressBackspace();
        break;
    case MYKEY_OPERATION:
        // X => Space
        l_returnValue = typeChar(true);
        break;
    case MYKEY_OPEN:
        // A => Button pressed
        if (m_selected == s_keyColumnsMinusOne)
        {
            l_returnValue = pressBackspace(); // Backspace letter selected
        }
        else if (m_selected == TOTALKEYS)
        {
            // Button Cancel
            m_returnValue = -1;
            l_returnValue = true;
        }
        else if (m_selected == s_totalKeysPlusOne)
        {
            // Button OK
            m_returnValue = 1;
            l_returnValue = true;
        }
        else
        {
            // A letter button
            l_returnValue = typeChar();
        }
        break;
    case MYKEY_CARETLEFT:
        // L => Moves the caret to the left
        l_returnValue = moveCaret(true);
        break;
    case MYKEY_CARETRIGHT:
        // R => Moves the caret to the right
        l_returnValue = moveCaret(false);
        break;
    case MYKEY_PAGEDOWN:
        // L2 => Change keys to the top-most left
        if (m_selected == s_totalKeysPlusOne)
        {
            --m_selected;
        }
        else if (m_selected != TOTALKEYS)
        {
            int row = static_cast<int>(floor(m_selected / (float)KEYCOLUMNS));
            m_selected = row * KEYCOLUMNS;
        }
        l_returnValue = true;
        break;
    case MYKEY_PAGEUP:
        // R2 => Change keys to the top-most right
        if (m_selected == TOTALKEYS)
        {
            ++m_selected;
        }
        else if (m_selected != s_totalKeysPlusOne)
        {
            int row = static_cast<int>(floor(m_selected / (float)KEYCOLUMNS));
            m_selected = (KEYCOLUMNS - 1) + row * KEYCOLUMNS;
        }
        l_returnValue = true;
        break;
    case MYKEY_TRANSFER:
        // SELECT => Button OK
        m_returnValue = 1;
        l_returnValue = true;
        break;
    case MYKEY_SELECT:
        // B => Change keyset
        m_keySet = (m_keySet + 1) % NB_KEY_SETS;
        l_returnValue = true;
        break;
    case MYKEY_PARENT:
        // MENU => Button Cancel
        m_returnValue = -1;
        l_returnValue = true;
        break;
    default:
        break;
    }

    if (m_selected >= 0 && m_selected < KEYCOLUMNS)
    {
        m_lastKeySelectedFirstRow = m_selected;
    }
    else if (m_selected >= s_keyColumnsMinusOneRow && m_selected < TOTALKEYS)
    {
        m_lastKeySelectedLastRow = m_selected;
    }

    return l_returnValue;
}

const bool CKeyboard::keyHold(void)
{
    // 1. By default, set the result to return as FALSE (the cursor was not moved).
    // 2. Handle the last pressed key mapping it to supported cases (MYKEY_xx).
    // 3. For each supported case:
    //    a. Check if the key is held down using 'tick' function and the SDL keyboard state.
    //    b. If the key is held, call the corresponding movement or action function.
    //    c. Update the return value based on the result of the function call.
    //    d. Set whether to show the caret either to FALSE or the result of the function call, depending on the case.
    // 4. For unsupported keys (default case), indicate that the caret does not need to be visible in the frame.
    // 6. Return the result indicating whether holding a key was handled (TRUE) or not (FALSE).

    bool l_returnValue(false);
    const Uint8 l_isJoyButtonDown(static_cast<const Uint8>(m_isJoyButtonDown));

    switch(m_lastPressed)
    {
        case MYKEY_UP:
            if (tick(l_isJoyButtonDown | SDL_GetKeyboardState(nullptr)[SDL_GetScancodeFromKey(MYKEY_UP)]))
            {
                l_returnValue = moveCursorUp(LOOP_ONJOYDOWN);
                m_mustShowCaret = false;
            }
            break;
        case MYKEY_DOWN:
            if (tick(l_isJoyButtonDown | SDL_GetKeyboardState(nullptr)[SDL_GetScancodeFromKey(MYKEY_DOWN)]))
            {
                l_returnValue = moveCursorDown(LOOP_ONJOYDOWN);
                m_mustShowCaret = false;
            }
            break;
        case MYKEY_LEFT:
            if (tick(l_isJoyButtonDown | SDL_GetKeyboardState(nullptr)[SDL_GetScancodeFromKey(MYKEY_LEFT)]))
            {
                l_returnValue = moveCursorLeft(LOOP_ONJOYDOWN);
                m_mustShowCaret = false;
            }
            break;
        case MYKEY_RIGHT:
            if (tick(l_isJoyButtonDown | SDL_GetKeyboardState(nullptr)[SDL_GetScancodeFromKey(MYKEY_RIGHT)]))
            {
                l_returnValue = moveCursorRight(LOOP_ONJOYDOWN);
                m_mustShowCaret = false;
            }
            break;
        case MYKEY_SYSTEM:
            // Y => Backspace
            if (tick(l_isJoyButtonDown | SDL_GetKeyboardState(nullptr)[SDL_GetScancodeFromKey(MYKEY_SYSTEM)]))
            {
                l_returnValue = pressBackspace();
                m_mustShowCaret = l_returnValue;
            }
            break;
        case MYKEY_OPERATION:
            // X => Space
            if (tick(l_isJoyButtonDown | SDL_GetKeyboardState(nullptr)[SDL_GetScancodeFromKey(MYKEY_OPERATION)]))
            {
                l_returnValue = typeChar(true);
                m_mustShowCaret = l_returnValue;
            }
            break;
        case MYKEY_OPEN:
            // A => Add letter
            if (tick(l_isJoyButtonDown | SDL_GetKeyboardState(nullptr)[SDL_GetScancodeFromKey(MYKEY_OPEN)]))
            {
                if (m_selected == KEYCOLUMNS - 1)
                {
                    l_returnValue = pressBackspace(); // Backspace letter selected
                }
                else
                {
                    l_returnValue = typeChar();
                }

                m_mustShowCaret = l_returnValue;
            }
            break;
        case MYKEY_CARETLEFT:
            // L => Moves the caret to the left
            if (tick(l_isJoyButtonDown | SDL_GetKeyboardState(nullptr)[SDL_GetScancodeFromKey(MYKEY_CARETLEFT)]))
            {
                l_returnValue = moveCaret(true);
                m_mustShowCaret = l_returnValue;
            }
            break;
        case MYKEY_CARETRIGHT:
            // R => Moves the caret to the right
            if (tick(l_isJoyButtonDown | SDL_GetKeyboardState(nullptr)[SDL_GetScancodeFromKey(MYKEY_CARETRIGHT)]))
            {
                l_returnValue = moveCaret(false);
                m_mustShowCaret = l_returnValue;
            }
            break;
        default:
            m_mustShowCaret = false;
            break;
    }

    return l_returnValue;
}

const bool CKeyboard::moveCursorUp(const bool p_loop)
{
    // 1. By default, set the result to return as FALSE (the cursor was not moved).
    // 2. Check if the currently selected key is beyond the total number of keys:
    //    a. Move the selection to the last key in the last row.
    //    b. Set the return value to TRUE to indicate movement.
    // 3. Check if the currently selected key is in the second or subsequent rows:
    //    a. Move the selection up by one row.
    //    b. Set the return value to TRUE.
    // 4. Check if the currently selected key is in the lower half of the first row:
    //    a. If looping is enabled, move the selection to the "OK" button.
    //    b. Update the last selected key in the last row.
    //    c. Set the return value to TRUE.
    // 5. For all other cases (upper half of the first row):
    //    a. If looping is enabled, move the selection to the "Cancel" button.
    //    b. Update the last selected key in the last row.
    //    c. Set the return value to TRUE.
    // 6. Return the result indicating whether the cursor was moved (TRUE) or not (FALSE).

    bool l_returnValue(false);

    static constexpr int s_totalKeysPlusOne = 1 + TOTALKEYS;
    static constexpr int s_keyColumnsMinusOneRow = KEYCOLUMNS * (KEYROWS - 1);
    static int s_halfSetDown = (int)std::floor(0.5f * KEYCOLUMNS);

    if (m_selected >= TOTALKEYS)
    {
        m_selected = m_lastKeySelectedLastRow;
        l_returnValue = true;
    }
    else if (m_selected >= KEYCOLUMNS)
    {
        m_selected -= KEYCOLUMNS;
        l_returnValue = true;
    }
    else if (m_selected >= s_halfSetDown)
    {
        if (p_loop)
        {
            m_lastKeySelectedLastRow = m_selected + s_keyColumnsMinusOneRow;
            m_selected = s_totalKeysPlusOne;
            l_returnValue = true;
        }
    }
    else
    {
        if (p_loop)
        {
            m_lastKeySelectedLastRow = m_selected + s_keyColumnsMinusOneRow;
            m_selected = TOTALKEYS;
            l_returnValue = true;
        }
    }

    return l_returnValue;
}

const bool CKeyboard::moveCursorDown(const bool p_loop)
{
    // 1. By default, set the result to return as FALSE (the cursor was not moved).
    // 2. Check if the currently selected key is beyond the total number of keys:
    //    a. If looping is enabled, move the selection to the first key of the first row.
    //    b. Set the return value to TRUE to indicate movement.
    // 3. Check if the currently selected key is in the upper half of the keyboard:
    //    a. Move the selection to the "OK" button.
    //    b. Update the last selected key in the first row.
    //    c. Set the return value to TRUE.
    // 4. Check if the currently selected key is in the last row:
    //    a. Move the selection to the "Cancel" button.
    //    b. Update the last selected key in the first row.
    //    c. Set the return value to TRUE.
    // 5. For all other cases, move the selection down by one row and set the return value to TRUE.
    // 6. Return the result indicating whether the cursor was moved (TRUE) or not (FALSE).

    bool l_returnValue(false);

    static constexpr int s_keyRowsMinusOne = KEYROWS - 1;
    static constexpr int s_keyColumnsMinusOneRow = KEYCOLUMNS * (KEYROWS - 1);
    static constexpr int s_keyInLastRow = TOTALKEYS - (KEYCOLUMNS - 1);
    static int s_halfSetUp = (int)(TOTALKEYS - std::ceil(0.5f * KEYCOLUMNS));

    if (m_selected >= TOTALKEYS)
    {
        if (p_loop)
        {
            m_selected = m_lastKeySelectedFirstRow;
            l_returnValue = true;
        }
    }
    else if (m_selected >= s_halfSetUp)
    {
        m_lastKeySelectedFirstRow = m_selected - s_keyColumnsMinusOneRow;
        m_selected = 1 + TOTALKEYS;
        l_returnValue = true;
    }
    else if (m_selected >= s_keyInLastRow)
    {
        m_lastKeySelectedFirstRow = m_selected - s_keyColumnsMinusOneRow;
        m_selected = TOTALKEYS;
        l_returnValue = true;
    }
    else
    {
        m_selected += KEYCOLUMNS;
        l_returnValue = true;
    }

    return l_returnValue;
}

const bool CKeyboard::moveCursorLeft(const bool p_loop)
{
    // 1. By default, set the result to return as FALSE (the cursor was not moved).
    // 2. Check if the selected key is at the left-most column of any row or the first key in the keyboard.
    // 3. If it is and looping is enabled:
    //    a. Move the selection to the right-most key of the same row.
    //    b. Set the return value to TRUE to indicate the cursor was moved.
    // 4. If the selected key is the "Cancel" button and looping is enabled:
    //    a. Move the selection to the "OK" button.
    //    b. Set the return value to TRUE.
    // 5. For all other cases:
    //    a. Decrement the selected key index to move the cursor to the left.
    //    b. Set the return value to TRUE.
    // 6. Return the result indicating whether the cursor was moved (TRUE) or not (FALSE).

    bool l_returnValue(false);

    static constexpr int s_keyColumnsMinusOne = KEYCOLUMNS - 1;
    static constexpr int s_secondRow = 2 * KEYCOLUMNS;
    static constexpr int s_thirdRow = 3 * KEYCOLUMNS;

    switch (m_selected)
    {
        case 0:
        case KEYCOLUMNS:
        case s_secondRow:
        case s_thirdRow:
            if (p_loop)
            {
                m_selected += s_keyColumnsMinusOne;
                l_returnValue = true;
            }
            break;
        case TOTALKEYS:
            if (p_loop)
            {
                m_selected += 1;
                l_returnValue = true;
            }
            break;
        default:
            --m_selected;
            l_returnValue = true;
            break;
    }

    return l_returnValue;
}

const bool CKeyboard::moveCursorRight(const bool p_loop)
{
    // 1. By default, set the result to return as FALSE (the cursor was not moved).
    // 2. Check if the selected key is at the right-most column of any row or the last key in the keyboard.
    // 3. If it is and looping is enabled:
    //    a. Move the selection to the left-most key of the same row.
    //    b. Set the return value to TRUE to indicate the cursor was moved.
    // 4. If the selected key is the "OK" button and looping is enabled:
    //    a. Move the selection to the "Cancel" button.
    //    b. Set the return value to TRUE.
    // 5. For all other cases:
    //    a. Increment the selected key index to move the cursor to the right.
    //    b. Set the return value to TRUE.
    // 6. Return the result indicating whether the cursor was moved (TRUE) or not (FALSE).

    bool l_returnValue(false);

    static constexpr int s_keyColumnsMinusOne = KEYCOLUMNS - 1;
    static constexpr int s_secondColumnMinusOne = 2 * KEYCOLUMNS - 1;
    static constexpr int s_thirdColumnMinusOne = 3 * KEYCOLUMNS - 1;

    switch (m_selected)
    {
        case s_keyColumnsMinusOne:
        case s_secondColumnMinusOne:
        case s_thirdColumnMinusOne:
        case TOTALKEYS - 1:
            if (p_loop)
            {
                m_selected -= s_keyColumnsMinusOne;
                l_returnValue = true;
            }
            break;
        case 1 + TOTALKEYS:
            if (p_loop)
            {
                m_selected -= 1;
                l_returnValue = true;
            }
            break;
        default:
            ++m_selected;
            l_returnValue = true;
            break;
    }

    return l_returnValue;
}

const bool CKeyboard::typeChar(const bool p_addSpace)
{
    // 1. Determine the current size of the input text and initialize the caret advance to 1.
    // 2. Check if a space needs to be added (parameter equals TRUE).
    //    a. If true, insert a space at the current caret position.
    //    b. If false, proceed to add the selected character (step 3).
    // 3. If adding a character:
    //    a. Verify that the selected key index lays within the valid range of keys.
    //    b. Calculate the real index of the selected key in the key set, accounting for UTF-8 characters.
    //    c. Extract the substring representing the selected key from the key set.
    //    d. Insert the extracted substring at the current caret position in the input text.
    //    e. Update the caret advance based on the size of the inserted substring.
    // 4. If the selected key index is invalid, log an error and return FALSE.
    // 5. Update the caret position by adding the caret advance.
    // 6. Return TRUE to indicate successful insertion of the corresponding char.

    const size_t l_currentSize = m_inputText.size();
    size_t l_caretAdvance = 1;

    if (p_addSpace == false)
    {
        if (m_selected < TOTALKEYS)
        {
            unsigned char l_index(0);

            for (unsigned char l_c = 0; l_c < m_selected; ++l_c)
            {
                l_index += 1 + checkUtf8Code(m_keySets[m_keySet].at(l_index));
            }

            size_t l_size = static_cast<size_t>(1 + checkUtf8Code(m_keySets[m_keySet].at(l_index)));
            const std::string subString = m_keySets[m_keySet].substr(l_index, l_size);
            m_inputText.insert(m_caretPosition, subString);
            l_caretAdvance = l_size;
        }
        else
        {
            SDL_LogError(0, "Unexpected keyboard value: %c", m_selected);
            return false;
        }
    }
    else
    {
        m_inputText.insert(m_caretPosition, " ");
    }

    m_caretPosition += l_caretAdvance;
    INHIBIT(SDL_Log("Caret Position after insertion: %d", m_caretPosition);)

    return true;
}

const bool CKeyboard::pressBackspace(void)
{
    // 1. Check whether there is any text input and the caret position is not at the begining.
    // 2. If ok, check whether one or two character must be removed from the text.
    // 3. Removed the corresponding amount of characters.
    // 4. Return the corresponding value.

    bool l_returnValue(false);

    if (m_inputText.empty() == false && m_caretPosition > 0)
    {
        const size_t l_currentSize(m_inputText.size());
		size_t l_caretAdvance(1);

        if (m_caretPosition > 1 && checkUtf8Code(m_inputText.at(m_caretPosition - 2)))
        {
            l_caretAdvance = 2;
        }
        
		m_inputText.erase(m_caretPosition - l_caretAdvance, l_caretAdvance);
		m_caretPosition -= l_caretAdvance;
        l_returnValue = true;

        INHIBIT(SDL_Log("Caret Position after deletion: %d", m_caretPosition);)
    }

    return l_returnValue;
}

const bool CKeyboard::moveCaret(const bool goLeft)
{
    // 1. Check whether there is any text input.
    // 2. If there is, check whether the user intends to move the caret to the left or right.
    // 3. If going left:
    //       a. Check whether the previous character needs 1 or 2 bytes in memory for proper visualization.
    //       b. Move the caret to the left by the calculated amount, without surpassing index 0.
    // 4. If going right:
    //       a. Check whether the next character needs 1 or 2 bytes in memory for proper visualization.
    //       b. Move the caret to the right by the calculated amount, without surpassin the end of the string.
    // 5. For both cases indicate success (TRUE), because the caret was moved.
    // 6. Return the corresponding value (TRUE if moved; otherwise, FALSE).

    bool l_returnValue(false);

    if (m_inputText.empty() == false)
    {
		const size_t l_currentSize(m_inputText.size());

        if (goLeft)
        {
            const size_t l_caretAdvance = static_cast<size_t>(1) + ((m_caretPosition < 2) ? 0 : checkUtf8Code(m_inputText.at(m_caretPosition - 2)));
            
            if (m_caretPosition > 0) m_caretPosition = std::max(size_t(0), m_caretPosition - l_caretAdvance);
        }
        else if(m_caretPosition < l_currentSize)
        {
            const size_t l_caretAdvance = static_cast<size_t>(1) + ((m_caretPosition > l_currentSize - 2) ? 0 : checkUtf8Code(m_inputText.at(m_caretPosition)));

            m_caretPosition = std::min(l_currentSize, m_caretPosition + l_caretAdvance);
        }

        l_returnValue = true;
    }

    INHIBIT(SDL_Log("Caret Position after movement: %d", m_caretPosition);)

    return l_returnValue;
}

void CKeyboard::handleUnsupportedEvent(void)
{
	m_mustShowCaret = m_showCaret = false; // Always set it to false.
}

const bool CKeyboard::checkUtf8Code(const unsigned char p_char) const
{
    // Check whether the passed character in UTF8 coding requires 1 or 2 bytes
    // in memory to be visually represented.

    return (p_char >= 194 && p_char <= 198) || p_char == 208 || p_char == 209;
}

