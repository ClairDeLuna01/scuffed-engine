#include "font.hpp"

FT_Library getFTLibrary()
{
    static FT_Library ft;
    // cursed
    [[maybe_unused]]
    static bool once = []() {
        if (FT_Init_FreeType(&ft))
        {
            throw std::runtime_error("Failed to initialize FreeType");
        }
        return true;
    }();
    return ft;
}