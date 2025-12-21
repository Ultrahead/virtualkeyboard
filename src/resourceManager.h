/**
 * @file  resourceManager.h
 */
#ifndef _RESOURCEMANAGER_H_
#define _RESOURCEMANAGER_H_

#include <SDL.h>
#include <SDL_ttf.h>
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief Macro that indicates the number of surface resources to load.
 *
 * @param X Specifies the number of surfaces to load.
 */
#define NB_SURFACES 1

  /**
   * @brief Macro that indicates the size of the font to load.
   *
   * @param X Specifies the size, in pixels, of the font to load.
   */
#define FONT_SIZE 8

/**
 * @class Singleton used to manage resources.
 * @brief Manages resources such as surfaces and fonts.
 */
class CResourceManager
{
    public:

    /**
     * @enum  T_SURFACE
     * @brief Enumeration of surface types.
     */
    typedef enum
    {
        T_SURFACE_BACKGROUND = 0, /**< Background surface */
        T_SURFACE_UNKNOWN         /**< Unknown surface */
    }
    T_SURFACE;

    /**
     * @brief  Gets the singleton instance of the resource manager.
     * @return Reference to the unique resource-manager instance.
     */
    static CResourceManager& instance(void);

    /**
     * @brief      Initializes the resource manager.
     * @param argc The amount of external arguments passed when executed the program.
     * @param argv The array of passed arguments.
     * @return     TRUE if initialization was successful; otherwise, FALSE.
     */
    const bool init(const int p_argumentCount, char** const p_argumentValues);

    /**
     * @brief Cleans up all resources.
     */
    void sdlCleanup(void);

    /**
     * @brief           Gets a specific surface type.
     * @param p_surface The surface type to load.
     * @return          Pointer to the SDL surface.
     */
    SDL_Surface* getSurface(const T_SURFACE p_surface) const;

    /**
     * @brief  Gets the loaded TTF font.
     * @return Pointer to the TTF font.
     */
    inline TTF_Font* getFont(void) const { return m_font; }

    private:

    /**
     * @brief Constructor for the resource manager.
     */
    CResourceManager(void);

    /**
     * @brief          Copy constructor for the resource manager (forbidden).
     * @param p_source The source resource manager to copy.
     */
    CResourceManager(const CResourceManager& p_source) = delete;

    /**
     * @brief          Move constructor for the resource manager (forbidden).
     * @param p_source The source resource manager to move resources from.
     */
    CResourceManager(const CResourceManager&& p_source) = delete;

    /**
     * @brief               Checks whether the path to a folder is valid or not.
     * @param p_pathToCheck The path to check.
     * @return              TRUE if the path is a valid directory; otherwise, FALSE.
     */
    inline bool isPathValid(const fs::path& p_pathToCheck)
    {
        return fs::exists(p_pathToCheck) && fs::is_directory(p_pathToCheck);
    }

    /**
     * @brief Array of surfaces to load.
     */
    SDL_Surface* m_surfaces[NB_SURFACES];

    /**
     * @brief Pointer to the TTF font.
     */
    TTF_Font* m_font;
};

#endif // _RESOURCEMANAGER_H_
