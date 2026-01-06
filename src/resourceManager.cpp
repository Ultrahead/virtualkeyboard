/**
 * @file  resourceManager.cpp
 * @brief Implementation file for the CResourceManager class.
 */

#ifndef __MINGW32__
#define zoomSurface GFX_zoomSurface
#define shrinkSurface GFX_shrinkSurface
#endif // __MINGW32__

#include <cstdint>
#include <iostream>
#include <SDL_image.h>
#include <SDL2_rotozoom.h>
#include "resourceManager.h"
#include "def.h"
#include "screen.h"
#include "sdlUtils.h"

namespace
{
    /**
     * @brief      Loads an icon from the specified path.
     * @param path The path to the icon file.
     * @return     Pointer to the loaded SDL Surface, or a nullptr if loading fails.
     */
    SDL_Surface* LoadIcon(const char* path) 
    {
        // 1. Load the image with the provided path.
        // 2. Check whether and image was properly loaded.
        // 3. Return the pointer (a valid one or a null pointer).

        SDL_Surface* l_image = IMG_Load(path);

        if(l_image == nullptr)
        {
            SDL_LogError(0, "LoadIcon: %s", IMG_GetError());
        }

        return l_image;
    }
} // namespace

CResourceManager& CResourceManager::instance(void)
{
	// 1. Create the static instance of the resource manager.
    // 2. Return the singleton.

    static CResourceManager l_singleton;
    return l_singleton;
}

CResourceManager::CResourceManager(void) :
    m_font(nullptr), m_surfaces() 
{ 
    // Nothing to do here. Let us the object be properly instantiated.
	// And create all resources when needed, elsewhere.
}

const bool CResourceManager::init(const int argc, char** const argv)
{
	// 1. Try to get the path to the "res" folder (from environment variable, executable path or current path).
	// 2. Try to get the background image path from the command line arguments.
	// 3. If not provided, use the default one.
	// 4. Load the background image and assign it to the proper slot in the array.
	// 5. Load the font and assign it to the corresponding member.
	// 6. If any of the loading operation fails, return FALSE. Otherwise, return TRUE.

    fs::path l_resPath;
	uint8_t l_pathSourceId = 0;

#ifndef __MINGW32__
    char* l_env = nullptr;
    size_t len = 0;

    if (_dupenv_s(&l_env, &len, "VK_RES_PATH") == 0 && l_env != nullptr)
#else
    if (const char* l_env = getenv("VK_RES_PATH"))
#endif // __MINGW32__
    {
		l_resPath = fs::path(l_env);      
		l_pathSourceId = isPathValid(l_resPath) ? 1 : 0; // From the environment variable
    }

    if(l_pathSourceId < 1)
    {
        fs::path l_executablePath(argv[0]);
        const fs::path l_parentPath(l_executablePath.parent_path().append(RES_DIR_NAME));

        try 
        {
            l_resPath = l_parentPath.is_absolute() ? l_parentPath : fs::absolute(l_parentPath);
            l_pathSourceId = isPathValid(l_resPath) ? 2 : 0; // From the executable path
        }
        catch (const fs::filesystem_error& e) 
        {
            std::cerr << "Error determining absolute path: " << e.what() << std::endl;
        }
    }

    if (l_pathSourceId < 1)
    {
		l_resPath = fs::current_path().append(RES_DIR_NAME);
        l_pathSourceId = isPathValid(l_resPath) ? 3 : 0; // From the working path (with the res folder)
    }

    if (l_pathSourceId < 1)
    {
        l_resPath = fs::current_path();
		l_pathSourceId = 4; // From the working path (without the res folder), if everything else fails.
    }

    SDL_Log("Using resource path: %s (from source: %i)", l_resPath.string().c_str(), l_pathSourceId);

    const char* l_backgroundName = (argc > 1) ? argv[1] : "background_default.png";
    fs::path l_tempPath(l_backgroundName);

    const fs::path l_backgroundPath = l_tempPath.is_absolute() ? l_tempPath : l_resPath / l_backgroundName;
    
    if (SDL_Surface* l_surface = LoadIcon(l_backgroundPath.string().c_str()))
	{
		m_surfaces[T_SURFACE_BACKGROUND] = l_surface;
        SDL_Log("Using background: %s", l_backgroundName);
	}
	else
    {
	    SDL_LogError(0, "Could not load keyboard's background image: %s", IMG_GetError());
    }

    const char* l_fontName = (argc > 2) ? (*argv[2] != '\0' ? argv[2] : "FieryTurk.ttf") : "FieryTurk.ttf";
    l_tempPath = fs::path(l_fontName);
    const fs::path l_fontPath = l_tempPath.is_absolute() ? l_tempPath : l_resPath / l_fontName;
    m_font = SDL_Utils::loadFont(l_fontPath.string().c_str(), static_cast<int>(FONT_SIZE * Globals::g_Screen.getAdjustedPpuY()));

    if(m_font == nullptr)
    {
        fs::path l_executablePath(argv[0]);
        const fs::path l_fontPathDefault(l_executablePath.parent_path().append(RES_DIR_NAME) / "FieryTurk.ttf");
        m_font = SDL_Utils::loadFont(l_fontPathDefault.string().c_str(), static_cast<int>(FONT_SIZE * Globals::g_Screen.getAdjustedPpuY()));

        if (m_font == nullptr)
        {
            SDL_LogError(0, "Could not load keyboard's font: %s", TTF_GetError());
            return false;
        }
        else
		{
			SDL_Log("Using default font: FieryTurk.ttf");
		}
    }
    else
    {
		SDL_Log("Using font: %s", l_fontName);
    }

    return true;
}

void CResourceManager::sdlCleanup(void)
{
	// 1. Free all surfaces in the array.
	// 2. Free the TTF font.
	// 3. Set all pointers to nullptr.

    INHIBIT(SDL_Log("Cleaning up resources ...");)
    
    // Free surfaces
    for (int l_i = 0; l_i < NB_SURFACES; ++l_i)
    {
        if (m_surfaces[l_i] != nullptr)
        {
            SDL_FreeSurface(m_surfaces[l_i]);
            m_surfaces[l_i] = nullptr;
        }
    }

    // Free font
    if (m_font != nullptr)
    {
        TTF_CloseFont(m_font);
        m_font = nullptr;
    }
}

SDL_Surface* CResourceManager::getSurface(const T_SURFACE p_surface) const  
{  
   // 1. Check whether the surface type is valid.  
   // 2. If not, log an error and return a nullptr.  
   // 3. Otherwise, return the pointer to the surface.  

   if (p_surface < 0 || p_surface >= NB_SURFACES)  
   {  
       SDL_LogError(0, "Invalid surface type passed to load.");  
       return nullptr;  
   }  

   return m_surfaces[p_surface];  
}
