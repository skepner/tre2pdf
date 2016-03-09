#pragma once

#include <string>
#include <stdexcept>
#include <functional>

#include "cairo.hh"
#include "date.hh"
#include "color.hh"
#include "json.hh"

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wweak-vtables"
#endif

// ----------------------------------------------------------------------

class Tree;
class Node;
class TreeImage;
class Surface;

// ----------------------------------------------------------------------

class TreeImageError : public std::runtime_error
{
 public: using std::runtime_error::runtime_error;
};

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

class Coloring
{
 public:
    Coloring() = default;
    Coloring(const Coloring&) = default;
    virtual ~Coloring() = default;
    virtual Color operator()(const Node&) const = 0;
};

class ColoringBlack : public Coloring
{
 public:
    virtual inline Color operator()(const Node&) const { return 0; }
};

// ----------------------------------------------------------------------

class TimeSeries
{
 public:
    inline TimeSeries() : mShow(true), mMonthWidth(10.0), mDashWidth(0.5), mDashLineWidth(1.0), mMonthLabelScale(0.9), mMaxNumberOfMonths(20), mMonthSeparatorColor(0), mMonthSeparatorWidth(0.1) {}

    inline double width() const { return mShow ? mNumberOfMonths * mMonthWidth : 0.0; }
    inline bool show() const { return mShow; }

    void setup(TreeImage& aMain, const Tree& aTre);
    void draw(TreeImage& aMain, const Tree& aTre, const Coloring& aColoring, bool aShowSubtreesTopBottom);

    inline const Location& origin() const { return mOrigin; }
    inline Location& origin() { return mOrigin; }
    inline void origin(const Location& aOrigin) { mOrigin = aOrigin; }

    json dump_to_json() const;
    void load_from_json(const json& j);

 private:
    struct SubtreeTopBottom
    {
        bool show;
        std::string branch_id;
        Color line_color;
        double line_width;
        bool draw_top;
        bool draw_bottom;

        inline SubtreeTopBottom() = default;

        inline SubtreeTopBottom(const json& j)
            {
                from_json(j, "show", show, true);
                from_json(j, "branch_id", branch_id);
                from_json(j, "line_color", line_color, Color(0x808080));
                from_json(j, "line_width", line_width, 1.0);
                from_json(j, "draw_top", draw_top, true);
                from_json(j, "draw_bottom", draw_bottom, true);
            }

        inline operator json() const
            {
                return json {
                    {"branch_id", branch_id},
                    {"show", show},
                    {"line_color", line_color},
                    {"line_width", line_width},
                    {"draw_top", draw_top},
                    {"draw_bottom", draw_bottom},
                };
            }
    };

    bool mShow;
    Date mBegin, mEnd;
    double mMonthWidth;
    double mDashWidth;           // relative to the mMonthWidth
    double mDashLineWidth;       // in points
    double mMonthLabelScale;
    size_t mMaxNumberOfMonths;
    Color mMonthSeparatorColor;
    double mMonthSeparatorWidth;
    std::vector<SubtreeTopBottom> mSubtreeTopBottom;

    size_t mNumberOfMonths;
    Location mOrigin;

    void draw_labels(TreeImage& aMain);
    void draw_labels_at_side(Surface& surface, const Location& a, double label_font_size, double month_max_width);
    void draw_month_separators(TreeImage& aMain);
    void draw_dashes(TreeImage& aMain, const Tree& aTre, const Coloring& aColoring);
    void draw_subtree_top_bottom(TreeImage& aMain, const Tree& aTre);
};

// ----------------------------------------------------------------------

class Clades
{
 public:
    inline Clades() : mShow(false), mSlotWidth(5.0), mLineWidth(1.0), mArrowWidth(3), mArrowColor(0), mArrowExtra(0.5),
                      mLabelColor(0), mLabelFontSize(10.0), mSeparatorColor(0x808080),
                      mSeparatorWidth(0.2), mSeparatorJustInTree(false), mSeparatorJustInTimeSeries(true), mOrigin(-1, -1) {}

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
    bool mSeparatorJustInTree;  // draw clade separator just to cover the tree area and not extend to the time series area
    bool mSeparatorJustInTimeSeries; // draw clade separator just to cover the time series area and not extend to the tree area

    Location mOrigin;
    double mWidth;

    struct CladeArrow
    {
        bool show;
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
            : show(true), begin(aBegin), end(aEnd), label(aLabel), id(aId), slot(-1), label_position("middle"),
              label_position_offset(0.0), label_rotation(0.0), label_offset(3.0) {}

        inline CladeArrow(const json& j)
            {
                from_json(j, "show", show, true);
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
                    {"show", show},
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
                c.show = ca.show;
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
    void draw(TreeImage& aMain, const Tree& aTre, const Coloring& aColoring, int aNumberStrainsThreshold, bool aShowBranchIds);

    json dump_to_json() const;
    void load_from_json(const json& j);

 private:
    struct BranchAnnotation
    {
        bool show;
        std::string id;         // branch id
        std::string label;
        Color color;
        double font_size;
        double label_offset_x;
        double label_offset_y;
        double label_interleave;
        bool show_line;
        Color line_color;
        double line_width;
        double line_x;
        double line_y;
        Color branch_id_color;
        double branch_id_font_size;
        double branch_id_offset_x;
        double branch_id_offset_y;

        inline BranchAnnotation(std::string aId = "") : id(aId) { defaults(); }

        inline BranchAnnotation(const json& j)
            {
                defaults();
                from_json(j, "show", show);
                from_json(j, "branch_id", id);
                from_json(j, "label", label);
                from_json(j, "color", color);
                from_json(j, "font_size", font_size);
                from_json(j, "label_offset_x", label_offset_x);
                from_json(j, "label_offset_y", label_offset_y);
                from_json(j, "label_interleave", label_interleave);
                from_json(j, "show_line", show_line);
                from_json(j, "line_color", line_color);
                from_json(j, "line_width", line_width);
                from_json(j, "line_x", line_x);
                from_json(j, "line_y", line_y);
                from_json(j, "branch_id_color", branch_id_color);
                from_json(j, "branch_id_font_size", branch_id_font_size);
                from_json(j, "branch_id_offset_x", branch_id_offset_x);
                from_json(j, "branch_id_offset_y", branch_id_offset_y);
            }

        inline operator json() const
            {
                return json {
                    {"branch_id", id},
                    {"show", show},
                    {"label", label},
                    {"color", color},
                    {"font_size", font_size},
                    {"label_offset_x", label_offset_x},
                    {"label_offset_y", label_offset_y},
                    {"label_interleave", label_interleave},
                    {"show_line", show_line},
                    {"line_color", line_color},
                    {"line_width", line_width},
                    {"line_x", line_x},
                    {"line_y", line_y},
                    {"branch_id_color", branch_id_color},
                    {"branch_id_font_size", branch_id_font_size},
                    {"branch_id_offset_x", branch_id_offset_x},
                    {"branch_id_offset_y", branch_id_offset_y}
                };
            }

        inline json make_json_for_branch_annotations_all() const
            {
                json j = *this;
                j.erase("_id");
                j.erase("label");
                return j;
            }

        inline void defaults()
            {
                show = true;
                color = Color(0);
                font_size = -1.0;
                label_offset_x = 0.0;
                label_offset_y = 0.0;
                label_interleave = 1.5;
                show_line = false;
                line_color = Color(0);
                line_width = 1.0;
                line_x = -10.0;
                line_y = 5.0;
                branch_id_color = Color(0xFFA000);
                branch_id_font_size = -1.0;
                branch_id_offset_x = 0.5;
                branch_id_offset_y = -0.25;
            }
    };

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
    BranchAnnotation mBranchAnnotationsAll;
    std::vector<BranchAnnotation> mBranchAnnotations; // for some branch ids

    void draw_node(TreeImage& aMain, const Node& aNode, double aLeft, const Coloring& aColoring, int aNumberStrainsThreshold, bool aShowBranchIds, double aEdgeLength = -1.0);
    double tree_width(TreeImage& aMain, const Node& aNode, double aEdgeLength = -1.0) const;
    const BranchAnnotation& find_branch_annotation(std::string branch_id) const;
    void show_branch_annotation(Surface& surface, std::string branch_id, std::string branch_annotation, double branch_left, double branch_right, double branch_y);
    void show_branch_id(Surface& surface, std::string id, double branch_left, double branch_y);

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
    void text(const Location& a, std::string aText, const Color& aColor, double aSize, double aRotation = 0, bool aMonospace = false);

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

    void make_pdf(std::string aFilename, const Tree& aTre, const Coloring& aColoring, int aNumberStrainsThreshold, bool aShowBranchIds, bool aShowSubtreesTopBottom, const Size& aCanvasSize = {72 * 8.5, 72 * 11.0});

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
    static Coloring* coloring_by_continent();
    static Coloring* coloring_by_pos(std::string aPos, const Tree& aTree);

    // static inline Coloring coloring_by_posX(std::string aPos, const Tree& aTree)
    //     {
    //         return std::bind(coloring_by_pos, aPos, std::placeholders::_1);
    //     }

    json dump_to_json() const;
    void load_from_json(const json& j);

 private:
    struct Title
    {
        bool show;
        std::string label;
        Color label_color;
        double font_size;
        double label_x;
        double label_y;

        inline Title() = default;

        inline void load_from_json(const json& j)
            {
                from_json(j, "show", show, true);
                from_json(j, "label", label);
                from_json(j, "label_color", label_color, Color(0));
                from_json(j, "font_size", font_size, 20.0);
                from_json(j, "label_x", label_x, 10.0);
                from_json(j, "label_y", label_y, 10.0);
            }

        inline operator json() const
            {
                return json {
                    {"show", show},
                    {"label", label},
                    {"label_color", label_color},
                    {"font_size", font_size},
                    {"label_x", label_x},
                    {"label_y", label_y},
                };
            }
    };

    double mBorder;             // around all parts, relative to the canvas size

    Viewport mViewport;
    double mSpaceTreeTs;
    double mSpaceTsClades;

    Surface mSurface;
    TreePart mTree;
    TimeSeries mTimeSeries;
    Clades mClades;
    Title mTitle;

    void setup(std::string aFilename, const Tree& aTre, const Size& aCanvasSize);
    void draw_title();
};

// ----------------------------------------------------------------------
