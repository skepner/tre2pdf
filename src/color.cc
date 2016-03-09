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
    {"dis", {
            {"T", 0x0000FF},
            {"I", 0x00FF00},
            {"N", 0xFF0000},
            {"Q", 0xFFA500},
            {"",  0x000000},
        }}
};

#pragma GCC diagnostic pop

const size_t sDistinctColors[] = {
    0x0000FF,
    0x00FF00,
    0xFF0000,
    0xFF8000,
};

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

Color Colors::distinct_by_index(size_t aIndex) const
{
    return aIndex < (sizeof(sDistinctColors) / sizeof(sDistinctColors[0])) ? sDistinctColors[aIndex] : 0xFFC0CB;

} // Colors::distinct_by_index

// ----------------------------------------------------------------------
