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
        }}
};

#pragma GCC diagnostic pop

// http://stackoverflow.com/questions/470690/how-to-automatically-generate-n-distinct-colors (search for kellysMaxContrastSet)
const size_t sDistinctColors[] = {
    0xA6BDD7, //Very Light Blue
    0xC10020, //Vivid Red
    0xFFB300, //Vivid Yellow
    0x803E75, //Strong Purple
    0xFF6800, //Vivid Orange
    0xCEA262, //Grayish Yellow
      //0x817066, //Medium Gray

    //The following will not be good for people with defective color vision
    0x007D34, //Vivid Green
    0xF6768E, //Strong Purplish Pink
    0x00538A, //Strong Blue
    0xFF7A5C, //Strong Yellowish Pink
    0x53377A, //Strong Violet
    0xFF8E00, //Vivid Orange Yellow
    0xB32851, //Strong Purplish Red
    0xF4C800, //Vivid Greenish Yellow
    0x7F180D, //Strong Reddish Brown
    0x93AA00, //Vivid Yellowish Green
    0x593315, //Deep Yellowish Brown
    0xF13A13, //Vivid Reddish Orange
    0x232C16, //Dark Olive Green
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
