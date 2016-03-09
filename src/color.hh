#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include "cairo.hh"
#include "json.hh"

// ----------------------------------------------------------------------

class Color
{
 public:
    inline Color(size_t aColor=0xFF00FF)
        : mColor(static_cast<uint32_t>(aColor)) {}
    inline Color(const Color&) = default;
    // inline Color(const json& j)
    //     : mColor(0) { *this = j; }
    inline Color& operator=(const Color& aSrc) = default;
    inline Color& operator=(size_t aColor)
        {
            mColor = static_cast<uint32_t>(aColor);
            return *this;
        }

    inline Color& operator=(const json& j)
        {
            const std::string jj = j;
            try {
                if (! ((jj.size() == 7 || jj.size() == 10) && jj[0] == '#'))
                    throw std::exception();
                const char* const s = jj.c_str();
                char* end;
                mColor = static_cast<uint32_t>(std::strtoul(s + 1, &end, 16));
                if ((end - s) != 7)
                    throw std::exception();
                if (jj.size() == 10) {
                    if (jj[7] != ':')
                        throw std::exception();
                    alphaI(static_cast<uint32_t>(std::strtoul(s + 8, &end, 16)));
                    if ((end - s) != 10)
                        throw std::exception();
                }
            }
            catch (std::exception&) {
                throw std::runtime_error(std::string("cannot parse Color from (json): ") + jj);
            }
            return *this;
        }

    inline bool operator == (const Color& aColor) const { return mColor == aColor.mColor; }
    inline bool operator != (const Color& aColor) const { return ! operator==(aColor); }

    inline double alpha() const { return double(0xFF - ((mColor >> 24) & 0xFF)) / 255.0; }
    inline double red() const { return double((mColor >> 16) & 0xFF) / 255.0; }
    inline double green() const { return double((mColor >> 8) & 0xFF) / 255.0; }
    inline double blue() const { return double(mColor & 0xFF) / 255.0; }

    inline size_t alphaI() const { return static_cast<size_t>((mColor >> 24) & 0xFF); }
    inline void alphaI(uint32_t v) { mColor = (mColor & 0xFFFFFF) | ((v & 0xFF) << 24); }
    inline size_t rgbI() const { return static_cast<size_t>(mColor & 0xFFFFFF); }

    inline void set_source_rgba(cairo_t* context) const
        {
            cairo_set_source_rgba(context, red(), green(), blue(), alpha());
        }

    // inline void set_transparency(double aTransparency) { mColor = (mColor & 0x00FFFFFF) | ((int(aTransparency * 255.0) & 0xFF) << 24); }

      //void load_from_json(const json& j) const;

    operator json() const;

 private:
    uint32_t mColor; // 4 bytes, most->least significant: transparency-red-green-blue, 0x00FF0000 - opaque red, 0xFF000000 - fully transparent

}; // class Color

// ----------------------------------------------------------------------

inline std::ostream& operator << (std::ostream& out, const Color& aColor)
{
    out << '#' << std::hex << std::setw(6) << std::setfill('0') << aColor.rgbI();
    if (aColor.alphaI())
        out << ':' << std::setw(2) << aColor.alphaI();
    out << std::dec;
    return out;
}

inline Color::operator json() const
{
    std::stringstream s;
    s << *this;
    return s.str();
}

// ----------------------------------------------------------------------

class Colors
{
 public:
    Colors();

    Color continent(std::string aContinent) const;
    Color aa_at(json aa_at, std::string pos) const;

 private:
    json mDatabase;
    Colors(const Colors&) = default;

}; // class Colors

Colors& colors();

// ----------------------------------------------------------------------
