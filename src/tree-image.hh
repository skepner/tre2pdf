#pragma once

#include <string>
#include <stdexcept>

#include "cairo.hh"
#include "date.hh"
#include "color.hh"
#include "json.hh"

// ----------------------------------------------------------------------

class Tree;
class Node;
class TreeImage;
class Surface;

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wweak-vtables"
#endif
class TreeImageError : public std::runtime_error
{
 public: using std::runtime_error::runtime_error;
};
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

class Location
{
 public:
    inline Location() : x(0), y(0) {}
    inline Location(double aX, double aY) : x(aX), y(aY) {}
    double x, y;

}; // class Location

inline std::ostream& operator << (std::ostream& out, const Location& aLocation)
{
    return out << aLocation.x << ' ' << aLocation.y;
};

// ----------------------------------------------------------------------

class Size
{
 public:
    inline Size() : width(0), height(0) {}
    inline Size(double aWidth, double aHeight) : width(aWidth), height(aHeight) {}
    double width, height;

}; // class Size

inline std::ostream& operator << (std::ostream& out, const Size& aSize)
{
    return out << aSize.width << ' ' << aSize.height;
}

inline Location operator + (const Location& a, const Size& s)
{
    return {a.x + s.width, a.y + s.height};
}

// inline Location operator - (const Location& a, const Size& s)
// {
//     return {a.x - s.width, a.y - s.height};
// }

inline Size operator - (const Location& a, const Location& b)
{
    return {a.x - b.x, a.y - b.y};
}

inline Size operator - (const Size& a, const Location& b)
{
    return {a.width - b.x, a.height - b.y};
}

inline Size operator * (const Size& a, double v)
{
    return {a.width * v, a.height * v};
}

// ----------------------------------------------------------------------

class Viewport
{
 public:
    inline Viewport() {}
    inline Viewport(const Location& a, const Size& s) : origin(a), size(s) {}
    inline Viewport(const Location& a, const Location& b) : origin(a), size(b - a) {}
    inline Location opposite() const { return origin + size; }
    Location origin;
    Size size;

}; // class Viewport

inline std::ostream& operator << (std::ostream& out, const Viewport& aViewport)
{
    return out << '[' << aViewport.origin << ' ' << aViewport.opposite() << ']';
};

// ----------------------------------------------------------------------

typedef Color (*Coloring)(const Node&);

// ----------------------------------------------------------------------

class TimeSeries
{
 public:
    inline TimeSeries() : mShow(true), mMonthWidth(10.0), mDashWidth(0.5), mDashLineWidth(1.0), mMonthLabelScale(0.9), mMaxNumberOfMonths(20), mMonthSeparatorColor(0), mMonthSeparatorWidth(0.1) {}

    inline double width() const { return mShow ? mNumberOfMonths * mMonthWidth : 0.0; }
    inline bool show() const { return mShow; }

    void setup(TreeImage& aMain, const Tree& aTre);
    void draw(TreeImage& aMain, const Tree& aTre, Coloring aColoring);

    inline const Location& origin() const { return mOrigin; }
    inline Location& origin() { return mOrigin; }
    inline void origin(const Location& aOrigin) { mOrigin = aOrigin; }

    json dump_to_json() const;
    void load_from_json(const json& j);

 private:
    bool mShow;
    Date mBegin, mEnd;
    double mMonthWidth;
    double mDashWidth;           // relative to the mMonthWidth
    double mDashLineWidth;       // in points
    double mMonthLabelScale;
    size_t mMaxNumberOfMonths;
    Color mMonthSeparatorColor;
    double mMonthSeparatorWidth;

    size_t mNumberOfMonths;
    Location mOrigin;

    void draw_labels(TreeImage& aMain);
    void draw_labels_at_side(Surface& surface, const Location& a, double label_font_size, double month_max_width);
    void draw_month_separators(TreeImage& aMain);
    void draw_dashes(TreeImage& aMain, const Tree& aTre, Coloring aColoring);
};

// ----------------------------------------------------------------------

class Clades
{
 public:
    inline Clades() : mShow(false), mSlotWidth(5.0), mLineWidth(1.0), mArrowWidth(3), mArrowColor(0), mArrowExtra(0.5),
                      mLabelColor(0), mLabelFontSize(10.0), mSeparatorColor(0x808080),
                      mSeparatorWidth(0.2), mOrigin(-1, -1) {}

    inline double width() const { return mShow ? mWidth : 0.0; }
    inline bool show() const { return mShow; }
    inline void show(bool aShow) { mShow = aShow; }

    void setup(TreeImage& aMain, const Tree& aTre);
    void draw(TreeImage& aMain, const Tree& aTre);

    inline const Location& origin() const { return mOrigin; }
    inline Location& origin() { return mOrigin; }
    inline void origin(const Location& aOrigin) { mOrigin = aOrigin; }

    json dump_to_json() const;
    void load_from_json(const json& j);

 private:
    bool mShow;
    double mSlotWidth;
    double mLineWidth;
    double mArrowWidth;
    Color mArrowColor;
    double mArrowExtra;          // fraction of vertical_step
    Color mLabelColor;
    double mLabelFontSize;
    Color mSeparatorColor;
    double mSeparatorWidth;

    Location mOrigin;
    double mWidth;

    struct CladeArrow
    {
        int begin;
        int end;
        std::string label;
        std::string id;
        int slot;
        std::string label_position; // "middle", "top", "bottom"
        double label_position_offset;
        double label_rotation;
        double label_offset;

        inline CladeArrow() = default;
        inline CladeArrow(int aBegin, int aEnd, std::string aLabel, std::string aId)
            : begin(aBegin), end(aEnd), label(aLabel), id(aId), slot(-1), label_position("middle"),
              label_position_offset(0.0), label_rotation(0.0), label_offset(3.0) {}

        inline CladeArrow(const json& j)
            {
                from_json(j, "begin", begin);
                from_json(j, "end", end);
                from_json(j, "_id", id);
                from_json(j, "label", label);
                from_json(j, "label_position", label_position);
                from_json(j, "label_position", label_position);
                from_json(j, "label_position_offset", label_position_offset, 0.0);
                from_json(j, "label_rotation", label_rotation, 0.0);
                from_json(j, "label_offset", label_offset, 3.0);
                from_json(j, "slot", slot);
            }

        inline operator json() const
            {
                return json {
                    {"_id", id},
                    {"begin", begin},
                    {"end", end},
                    {"label", label},
                    {"label_position", label_position},
                    {"label_position_offset", label_position_offset},
                    {"label_rotation", label_rotation},
                    {"label_offset", label_offset},
                    {"slot", slot}
                };
            }
    };

    std::vector<CladeArrow> mClades;
    std::map<std::string, CladeArrow> mPerClade;

    inline void add_clade(int aBegin, int aEnd, std::string aLabel, std::string aId)
        {
            CladeArrow c{aBegin, aEnd, aLabel, aId};
            auto const per = mPerClade.find(aId);
            if (per != mPerClade.end()) {
                const CladeArrow& ca = per->second;
                c.label = ca.label;
                if (ca.begin >= 0)
                    c.begin = ca.begin;
                if (ca.end >= 0)
                    c.end = ca.end;
                if (ca.slot >= 0)
                    c.slot = ca.slot;
                if (!ca.label_position.empty())
                    c.label_position = ca.label_position;
                c.label_position_offset = ca.label_position_offset;
                c.label_rotation = ca.label_rotation;
                c.label_offset = ca.label_offset;
            }
            mClades.push_back(c);
        }

    void assign_slots(TreeImage& aMain);
    void draw_clade(TreeImage& aMain, const CladeArrow& aClade);

}; // class Clades

// ----------------------------------------------------------------------

class TreePart
{
 public:
    inline TreePart() : mHorizontalStep(5.0), mLineWidth(0.2), mLabelScale(1.0), mLineColor(0), mNameOffset(0.2),
                        mRootEdge(0.0), mOrigin(-1, -1) {}

    inline const Location& origin() const { return mOrigin; }
    inline double width() const { return mWidth; }
    inline double name_offset() const { return mNameOffset; }
    inline double vertical_step() const { return mVerticalStep; }
    inline size_t number_of_lines() const { return mNumberOfLines; }

    void setup(TreeImage& aMain, const Tree& aTre);
    void adjust_label_scale(TreeImage& aMain, const Tree& aTre, double tree_right_margin);
    void adjust_horizontal_step(TreeImage& aMain, const Tree& aTre, double tree_right_margin);
    void draw(TreeImage& aMain, const Tree& aTre, Coloring aColoring);

    json dump_to_json() const;
    void load_from_json(const json& j);

 private:
    double mHorizontalStep;
    double mLineWidth;          // width of the drawn tree line
    double mLabelScale;
    Color mLineColor;
    double mNameOffset;
    double mRootEdge;

    double mWidth;
    size_t mNumberOfLines;
    double mVerticalStep;       // vertical step between name nodes
    Location mOrigin;

    void draw_node(TreeImage& aMain, const Node& aNode, double aLeft, Coloring aColoring, double aEdgeLength = -1.0);
    double tree_width(TreeImage& aMain, const Node& aNode, double aEdgeLength = -1.0) const;

}; // class TreePart

// ----------------------------------------------------------------------

class Surface
{
 public:
    inline Surface() : mContext(nullptr) {}

    inline ~Surface()
        {
            if (mContext != nullptr)
                cairo_destroy(mContext);
        }

    void setup(std::string aFilename, const Size& aCanvasSize);

    const Size& canvas_size() const { return mCanvasSize; }

    void line(const Location& a, const Location& b, const Color& aColor, double aWidth, cairo_line_cap_t aLineCap = CAIRO_LINE_CAP_BUTT);
    void double_arrow(const Location& a, const Location& b, const Color& aColor, double aLineWidth, double aArrowWidth);
    void text(const Location& a, std::string aText, const Color& aColor, double aSize, double aRotation = 0);

    Size text_size(std::string aText, double aSize, double* x_bearing = nullptr);

    void test();

 private:
    cairo_t* mContext;
    Size mCanvasSize;

    Location arrow_head(const Location& a, double angle, double sign, const Color& aColor, double aArrowWidth);

}; // class Surface

// ----------------------------------------------------------------------

class TreeImage
{
 public:
    inline TreeImage()
        : mBorder(0.1), mSpaceTreeTs(5.0), mSpaceTsClades(5.0)
        {
        }

    void make_pdf(std::string aFilename, const Tree& aTre, Coloring aColoring = nullptr, const Size& aCanvasSize = {72 * 8.5, 72 * 11.0});

    inline TimeSeries& time_series() { return mTimeSeries; }
    inline const TimeSeries& time_series() const { return mTimeSeries; }
    inline Clades& clades() { return mClades; }
    inline const Clades& clades() const { return mClades; }
    inline Surface& surface() { return mSurface; }
    inline const Surface& surface() const { return mSurface; }
    inline TreePart& tree() { return mTree; }
    inline const TreePart& tree() const { return mTree; }

    inline const Viewport& viewport() const { return mViewport; }
    inline double space_tree_ts() const { return mSpaceTreeTs; }
    inline double space_ts_clades() const { return mSpaceTsClades; }

      // To be passed to make_pdf
    static Color coloring_by_continent(const Node& aNone);

    json dump_to_json() const;
    void load_from_json(const json& j);

 private:
    double mBorder;             // around all parts, relative to the canvas size

    Viewport mViewport;
    double mSpaceTreeTs;
    double mSpaceTsClades;

    Surface mSurface;
    TreePart mTree;
    TimeSeries mTimeSeries;
    Clades mClades;

    void setup(std::string aFilename, const Tree& aTre, const Size& aCanvasSize);
};

// ----------------------------------------------------------------------
