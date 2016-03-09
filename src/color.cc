#include "color.hh"

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

const json sDefaultDatabase = {
    {"continents", {
            {"EUROPE",            0x00FF00},
            {"CENTRAL-AMERICA",   0xAAF9FF},
            {"MIDDLE-EAST",       0x8000FF},
            {"NORTH-AMERICA",     0x00008B},
            {"AFRICA",            0xFF8000},
            {"ASIA",              0xFF0000},
            {"RUSSIA",            0xB03060},
            {"AUSTRALIA-OCEANIA", 0xFF69B4},
            {"SOUTH-AMERICA",     0x40E0D0},
            {"ANTARCTICA",        0x808080},
            {"CHINA-SOUTH",       0xFF0000},
            {"CHINA-NORTH",       0x6495ED},
            {"CHINA-UNKNOWN",     0x808080},
            {"UNKNOWN",           0x808080},
        }},
    {"aa_at", {
            {"T", 0x0000FF},
            {"I", 0x00FF00},
            {"N", 0xFF0000},
            {"Q", 0xFFA500},
            {"",  0x000000},
        }}
};

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

static Colors* sColors = nullptr;

Colors& colors()
{
    if (sColors == nullptr)
        sColors = new Colors();
    return *sColors;

} // colors

// ----------------------------------------------------------------------

Colors::Colors()
    : mDatabase(sDefaultDatabase)
{
}

// ----------------------------------------------------------------------

Color Colors::continent(std::string aContinent) const
{
    auto const c = mDatabase["continents"].count(aContinent) ? mDatabase["continents"][aContinent] : mDatabase["continents"]["UNKNOWN"];
    return Color(c.get<size_t>());

} // Colors::continent

// ----------------------------------------------------------------------

Color Colors::aa_at(json aa_at) const
{
    const std::string aa = aa_at["199"].is_null() ? "" : aa_at["199"];
    auto const c = mDatabase["aa_at"].count(aa) ? mDatabase["aa_at"][aa] : mDatabase["aa_at"][""];
    return Color(c.get<size_t>());

} // Colors::continent

// ----------------------------------------------------------------------
