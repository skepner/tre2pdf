#include <iostream>
#include <cstdio>
#include <cmath>
#include <cassert>
#include <map>

#include "tree-image.hh"
#include "tree.hh"

// ----------------------------------------------------------------------

void TreeImage::make_pdf(std::string aFilename, const Tree& aTre, const Coloring& aColoring, int aNumberStrainsThreshold, bool aShowBranchIds, bool aShowSubtreesTopBottom, const Size& aCanvasSize)
{
    setup(aFilename, aTre, aCanvasSize);

    draw_title();
    tree().draw(*this, aTre, aColoring, aNumberStrainsThreshold, aShowBranchIds);
    draw_legend(aColoring);
    if (time_series().show())
        time_series().draw(*this, aTre, aColoring, aShowSubtreesTopBottom);
    if (clades().show())
        clades().draw(*this, aTre);

} // TreeImage::make_pdf

// ----------------------------------------------------------------------

void TreeImage::setup(std::string aFilename, const Tree& aTre, const Size& aCanvasSize)
{
    mViewport = Viewport(Location(0, 0) + aCanvasSize * mBorder * 0.5, aCanvasSize * (1.0 - mBorder * 0.5) - Location(10.0, aCanvasSize.height * mBorder * 0.5));

    mSurface.setup(aFilename, aCanvasSize);
      // mSurface.test();
    tree().setup(*this, aTre);
    time_series().setup(*this, aTre);
    clades().setup(*this, aTre);

    double clades_origin_x = viewport().origin.x + viewport().size.width;
    if (clades().show() && clades().width() > 1.0) {
        if (clades().origin().x > 0.0)
            clades_origin_x = clades().origin().x;
        else
            clades_origin_x -= clades().width();
    }
    const double clades_separator_width = clades().show() && clades().width() > 1.0 ? space_ts_clades() : 0.0;

    double time_series_origin_x = clades_origin_x;
    if (time_series().show()) {
        if (time_series().origin().x > 0.0)
            time_series_origin_x = time_series().origin().x;
        else
            time_series_origin_x -= time_series().width() + clades_separator_width;
    }
    const double time_series_separator_width = time_series().show() ? space_tree_ts() : 0.0;

    const double tree_right_margin = time_series_origin_x - time_series_separator_width;
    tree().adjust_label_scale(*this, aTre, tree_right_margin);
    tree().adjust_horizontal_step(*this, aTre, tree_right_margin);

    double x = tree_right_margin; // tree().origin().x + tree().width();
    if (time_series().show()) {
        if (time_series().origin().x > 0.0) {
            x = time_series().origin().x;
            time_series().origin().y = viewport().origin.y;
        }
        else {
            x += space_tree_ts();
            time_series().origin({x, viewport().origin.y});
        }
        x += time_series().width();
    }
    if (clades().show()) {
        if (clades().origin().x > 0.0) {
            x = clades().origin().x;
            clades().origin().y = viewport().origin.y;
        }
        else {
            x += space_ts_clades();
            clades().origin({x, viewport().origin.y});
        }
        x += clades().width();
    }
    std::cout << "Image width: " << x << std::endl;
    std::cout << "Canvas width: " << aCanvasSize.width << std::endl;

} // TreeImage::setup

// ----------------------------------------------------------------------

void TreeImage::draw_title()
{
    if (mTitle.show and !mTitle.label.empty()) {
        surface().text({mTitle.label_x, mTitle.label_y}, mTitle.label, mTitle.label_color, mTitle.font_size);
    }

} // TreeImage::draw_title

// ----------------------------------------------------------------------

void TreeImage::draw_legend(const Coloring& aColoring)
{
    aColoring.draw_legend(surface(), {tree().origin().x, tree().origin().y + tree().vertical_step() * tree().number_of_lines()}, mColoringSettings);

} // TreeImage::draw_legend

// ----------------------------------------------------------------------

class ColoringByContinent : public Coloring
{
 public:
    virtual inline Color operator()(const Node& aNode) const
        {
            return colors().continent(aNode.continent);
        }

    virtual inline void draw_legend(Surface& aSurface, const Location& aLocation, const ColoringSettings& aSettings) const
        {
            if (aSettings.legend_show) {
                const char* labels[] = {"EUROPE", "CENTRAL-AMERICA", "MIDDLE-EAST", "NORTH-AMERICA", "AFRICA", "ASIA", "RUSSIA", "AUSTRALIA-OCEANIA", "SOUTH-AMERICA"};
                auto n_labels = sizeof(labels) / sizeof(labels[0]);
                auto const label_size = aSurface.text_size("W", aSettings.legend_font_size, aSettings.legend_font_style, aSettings.legend_font_slant, aSettings.legend_font_weight);
                auto x = aLocation.x + aSettings.legend_offset_x;
                auto y = aLocation.y + aSettings.legend_offset_y - label_size.height * aSettings.legend_interline * n_labels;
                for (size_t index = 0; index < n_labels; ++index) {
                    aSurface.text({x, y + label_size.height * aSettings.legend_interline * index}, labels[index], colors().continent(labels[index]), aSettings.legend_font_size, aSettings.legend_font_style, aSettings.legend_font_slant, aSettings.legend_font_weight);
                }
            }
        }
};

Coloring* TreeImage::coloring_by_continent()
{
    return new ColoringByContinent();

} // TreeImage::coloring_by_continent

// ----------------------------------------------------------------------

class ColoringByPos : public Coloring
{
 public:
    inline ColoringByPos(std::string aPos, const Tree& aTree)
        : mPos(aPos), mAllAA(aTree.aa_at[mPos].is_string() ? aTree.aa_at[mPos].get<std::string>() : "")
        {
        }

    inline Color operator()(const Node& aNode) const
        {
            Color c(0);
            if (!mAllAA.empty() && aNode.aa_at[mPos].is_string()) {
                auto index = mAllAA.find(aNode.aa_at[mPos]);
                if (index != std::string::npos)
                    c = colors().distinct_by_index(index);
            }
            return c;
        }


    virtual inline void draw_legend(Surface& aSurface, const Location& aLocation, const ColoringSettings& aSettings) const
        {
            if (aSettings.legend_show) {
                auto const label_size = aSurface.text_size("W", aSettings.legend_font_size, aSettings.legend_font_style, aSettings.legend_font_slant, aSettings.legend_font_weight);
                auto x = aLocation.x + aSettings.legend_offset_x;
                auto y = aLocation.y + aSettings.legend_offset_y - label_size.height * aSettings.legend_interline * mAllAA.size();
                for (size_t index = 0; index < mAllAA.size(); ++index) {
                    aSurface.text({x, y + label_size.height * aSettings.legend_interline * index}, std::string(1, mAllAA[index]), colors().distinct_by_index(index), aSettings.legend_font_size, aSettings.legend_font_style, aSettings.legend_font_slant, aSettings.legend_font_weight);
                }
                aSurface.text(aLocation, "X", 0, aSettings.legend_font_size, aSettings.legend_font_style, aSettings.legend_font_slant, aSettings.legend_font_weight);
            }
        }

 private:
    std::string mPos;
    std::string mAllAA;
};

Coloring* TreeImage::coloring_by_pos(std::string aPos, const Tree& aTree)
{
    return new ColoringByPos(aPos, aTree);

} // TreeImage::coloring_by_pos

// ----------------------------------------------------------------------

json TreeImage::dump_to_json() const
{
    json j = {
        {"clades", clades().dump_to_json()},
        {"time_series", time_series().dump_to_json()},
        {"tree", tree().dump_to_json()},
        {"title", mTitle},
        {"coloring", mColoringSettings},
    };
    return j;

} // TreeImage::dump_to_json

// ----------------------------------------------------------------------

void TreeImage::load_from_json(const json& j)
{
    if (j.count("title"))
        mTitle.load_from_json(j["title"]);
    if (j.count("coloring"))
        mColoringSettings.load_from_json(j["coloring"]);
    if (j.count("tree"))
        tree().load_from_json(j["tree"]);
    if (j.count("time_series"))
        time_series().load_from_json(j["time_series"]);
    if (j.count("clades"))
        clades().load_from_json(j["clades"]);

} // TreeImage::load_from_json

// ----------------------------------------------------------------------

void Surface::setup(std::string aFilename, const Size& aCanvasSize)
{
    auto surface = cairo_pdf_surface_create(aFilename.c_str(), aCanvasSize.width, aCanvasSize.height);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
        throw TreeImageError("cannot create pdf surface");
    mContext = cairo_create(surface);
    cairo_surface_destroy(surface);
    if (cairo_status(mContext) != CAIRO_STATUS_SUCCESS)
        throw TreeImageError("cannot create mContext");

    mCanvasSize = aCanvasSize;

} // Surface::setup

// ----------------------------------------------------------------------

void Surface::line(const Location& a, const Location& b, const Color& aColor, double aWidth, cairo_line_cap_t aLineCap)
{
    cairo_save(mContext);
    cairo_set_line_width(mContext, aWidth);
    aColor.set_source_rgba(mContext);
    cairo_set_line_cap(mContext, aLineCap);
    cairo_move_to(mContext, a.x, a.y);
    cairo_line_to(mContext, b.x, b.y);
    cairo_stroke(mContext);
    cairo_restore(mContext);

} // Surface::line

// ----------------------------------------------------------------------

void Surface::double_arrow(const Location& a, const Location& b, const Color& aColor, double aLineWidth, double aArrowWidth)
{
    const bool x_eq = std::fabs(b.x - a.x) < 1e-10;
    const double sign2 = x_eq ? (a.y < b.y ? 1.0 : -1.0) : (b.x < a.x ? 1.0 : -1.0);
    const double angle = x_eq ? -M_PI_2 : std::atan((b.y - a.y) / (b.x - a.x));

    auto const la = arrow_head(a, angle, - sign2, aColor, aArrowWidth);
    auto const lb = arrow_head(b, angle,   sign2, aColor, aArrowWidth);

    line(la, lb, aColor, aLineWidth);

} // Surface::double_arrow

// ----------------------------------------------------------------------

Location Surface::arrow_head(const Location& a, double angle, double sign, const Color& aColor, double aArrowWidth)
{
    constexpr double ARROW_WIDTH_TO_LENGTH_RATIO = 2.0;

    const double arrow_length = aArrowWidth * ARROW_WIDTH_TO_LENGTH_RATIO;
    const Location b(a.x + sign * arrow_length * std::cos(angle), a.y + sign * arrow_length * std::sin(angle));
    const Location c(b.x + sign * aArrowWidth * std::cos(angle + M_PI_2) * 0.5, b.y + sign * aArrowWidth * std::sin(angle + M_PI_2) * 0.5);
    const Location d(b.x + sign * aArrowWidth * std::cos(angle - M_PI_2) * 0.5, b.y + sign * aArrowWidth * std::sin(angle - M_PI_2) * 0.5);

    cairo_save(mContext);
    aColor.set_source_rgba(mContext);
    cairo_set_line_join(mContext, CAIRO_LINE_JOIN_MITER);
    cairo_move_to(mContext, a.x, a.y);
    cairo_line_to(mContext, c.x, c.y);
    cairo_line_to(mContext, d.x, d.y);
    cairo_close_path(mContext);
    cairo_fill(mContext);
    cairo_restore(mContext);

    return b;

} // Surface::arrow_head

// ----------------------------------------------------------------------

void Surface::text(const Location& a, std::string aText, const Color& aColor, double aSize, FontStyle aFontStyle, cairo_font_slant_t aSlant, cairo_font_weight_t aWeight, double aRotation)
{
    cairo_save(mContext);
    context_prepare_for_text(aSize, aFontStyle, aSlant, aWeight);
    cairo_move_to(mContext, a.x, a.y);
    cairo_rotate(mContext, aRotation);
    aColor.set_source_rgba(mContext);
    cairo_show_text(mContext, aText.c_str());
    cairo_restore(mContext);

} // Surface::text

// ----------------------------------------------------------------------

Size Surface::text_size(std::string aText, double aSize, FontStyle aFontStyle, cairo_font_slant_t aSlant, cairo_font_weight_t aWeight, double* x_bearing)
{
    cairo_save(mContext);
    context_prepare_for_text(aSize, aFontStyle, aSlant, aWeight);
    cairo_text_extents_t text_extents;
    cairo_text_extents(mContext, aText.c_str(), &text_extents);
    cairo_restore(mContext);
    if (x_bearing != nullptr)
        *x_bearing = text_extents.x_bearing;
    return {text_extents.x_advance, - text_extents.y_bearing};

} // Surface::text_size

// ----------------------------------------------------------------------

void Surface::context_prepare_for_text(double aSize, FontStyle aFontStyle, cairo_font_slant_t aSlant, cairo_font_weight_t aWeight)
{
    switch (aFontStyle) {
      case FONT_MONOSPACE:
          cairo_select_font_face(mContext, "monospace", aSlant, aWeight);
          break;
      case FONT_DEFAULT:
          cairo_select_font_face(mContext, "sans-serif", aSlant, aWeight);
          break;
    }
    cairo_set_font_size(mContext, aSize);

} // Surface::context_prepare_for_text

// ----------------------------------------------------------------------

void Surface::test()
{
    line({100, 100}, {300, 100}, 0xFF00FF, 1);
    text({100, 100}, "May 99", 0xFFA500, 20);
    text({100, 100}, "May 99", 0x00A5FF, 30, FONT_DEFAULT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, M_PI_2);
    auto const tsize = text_size("May 99", 20, FONT_DEFAULT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    text({100 + tsize.width, 100 - tsize.height}, "May 99", 0xFF00A5, 20);

    double_arrow({100, 350}, {300, 550}, 0xFF0000, 1, 4);
    double_arrow({100, 550}, {300, 350}, 0x0000FF, 1, 4);
    double_arrow({100, 450}, {300, 450}, 0x00FF00, 1, 4);
    double_arrow({200, 350}, {200, 550}, 0x008000, 1, 4);

} // Surface::test

// ----------------------------------------------------------------------

void TreePart::setup(TreeImage& aMain, const Tree& aTre)
{
    auto const tre_wh = aTre.width_height();
    mNumberOfLines = tre_wh.second;
    mVerticalStep = aMain.viewport().size.height / (mNumberOfLines + 2); // +2 to add space at the top and bottom
    if (mOrigin.x < 0.0)
        mOrigin = {aMain.viewport().origin.x, aMain.viewport().origin.y + mVerticalStep};
    else
        mOrigin.y = aMain.viewport().origin.y + mVerticalStep;

} // TreePart::setup

// ----------------------------------------------------------------------

void TreePart::draw(TreeImage& aMain, const Tree& aTre, const Coloring& aColoring, int aNumberStrainsThreshold, bool aShowBranchIds)
{
    draw_node(aMain, aTre, origin().x, aColoring, aNumberStrainsThreshold, aShowBranchIds, mRootEdge);

} // TreePart::draw

// ----------------------------------------------------------------------

void TreePart::draw_node(TreeImage& aMain, const Node& aNode, double aLeft, const Coloring& aColoring, int aNumberStrainsThreshold, bool aShowBranchIds, double aEdgeLength)
{
    Surface& surface = aMain.surface();
    const double right = aLeft + (aEdgeLength < 0.0 ? aNode.edge_length : aEdgeLength) * mHorizontalStep;
    const double y = mOrigin.y + mVerticalStep * aNode.middle();

    surface.line({aLeft, y}, {right, y}, mLineColor, mLineWidth);
    if (aNode.is_leaf()) {
        const std::string text = aNode.display_name();
        auto const font_size = mVerticalStep * mLabelScale;
        auto const tsize = surface.text_size(text, font_size, Surface::FONT_DEFAULT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        surface.text({right + name_offset(), y + tsize.height * 0.5}, text, aColoring(aNode), font_size, Surface::FONT_DEFAULT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
          // std::cerr << (right + name_offset() + tsize.width) << " " << text << std::endl;
    }
    else {
        if (aShowBranchIds && !aNode.branch_id.empty()) {
            show_branch_id(surface, aNode.branch_id, aLeft, y);
        }
        if (!aNode.name.empty() && aNode.number_strains > aNumberStrainsThreshold) {
            show_branch_annotation(surface, aNode.branch_id, aNode.name, aLeft, right, y);
        }
        surface.line({right, mOrigin.y + mVerticalStep * aNode.top}, {right, mOrigin.y + mVerticalStep * aNode.bottom}, mLineColor, mLineWidth);
        for (auto node = aNode.subtree.begin(); node != aNode.subtree.end(); ++node) {
            draw_node(aMain, *node, right, aColoring, aNumberStrainsThreshold, aShowBranchIds);
        }
    }

} // TreePart::draw_node

// ----------------------------------------------------------------------

void TreePart::show_branch_annotation(Surface& surface, std::string branch_id, std::string branch_annotation, double branch_left, double branch_right, double branch_y)
{
    auto const ba = find_branch_annotation(branch_id); // mBranchAnnotationsAll; //
    if (ba.show) {
        auto const label = ba.label.empty() ? branch_annotation : ba.label;
        auto const branch_center = (branch_right + branch_left) / 2.0;
        auto text_y = branch_y;
        std::string::size_type pos = 0;
        while (true) {
            std::string::size_type end = label.find('\n', pos);
            auto font_size = ba.font_size > 0 ? ba.font_size : mVerticalStep * mLabelScale * (-ba.font_size);
            auto text = end == std::string::npos ? std::string(label, pos) : std::string(label, pos, end - pos);
            auto const ts = surface.text_size(text.empty() ? "I" : text, font_size, Surface::FONT_MONOSPACE, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
            auto text_x = branch_center - ts.width / 2.0;
            if (ba.label_offset_x == 0.0 && (text_x + ts.width) > branch_right)
                text_x = branch_right - ts.width;
            text_y += ts.height * ba.label_interleave;
            surface.text({text_x + ba.label_offset_x, text_y + ba.label_offset_y}, text, ba.color, font_size, Surface::FONT_MONOSPACE, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
            if ((text_x + ba.label_offset_x) < 0)
                std::cerr << text << " " << (text_x + ba.label_offset_x) << std::endl;
            if (end == std::string::npos)
                break;
            pos = end + 1;
        }
        if (ba.show_line) {
            auto line_y = branch_y + ba.line_width * 1.0;
            surface.line({branch_center, line_y}, {branch_center + ba.line_x, line_y + ba.line_y}, ba.line_color, ba.line_width);
        }
    }

} // TreePart::show_branch_annotation

// ----------------------------------------------------------------------

void TreePart::show_branch_id(Surface& surface, std::string id, double branch_left, double branch_y)
{
    auto const ba = mBranchAnnotationsAll;
    auto font_size = ba.branch_id_font_size > 0 ? ba.branch_id_font_size : mVerticalStep * mLabelScale * (-ba.branch_id_font_size);
    surface.text({branch_left + ba.branch_id_offset_x, branch_y + ba.branch_id_offset_y}, id, ba.branch_id_color, font_size);

} // TreePart::show_branch_id

// ----------------------------------------------------------------------

void TreePart::adjust_label_scale(TreeImage& aMain, const Tree& aTre, double tree_right_margin)
{
      // std::cerr << "right_margin: " << tree_right_margin << " viewport: " << aMain.viewport().size.width << " ts width: " << aMain.time_series().width() << "  ts space: " << aMain.space_tree_ts() << "  clades width: " << aMain.clades().width() << "  clades space: " << aMain.space_ts_clades() << std::endl;
    mWidth = tree_width(aMain, aTre);
    for (int i = 0; (mLabelScale * mVerticalStep) > 1.0 && (mWidth + mOrigin.x) > tree_right_margin; ++i) {
        mLabelScale *= 0.95;
        mWidth = tree_width(aMain, aTre, mRootEdge);
          // std::cerr << i << " label scale: " << mLabelScale << "  width:" << mWidth << " right:" << tree_right_margin << std::endl;
    }
      // std::cerr << "Label scale: " << mLabelScale << "  width:" << mWidth << " right:" << tree_right_margin << std::endl;

} // TreePart::adjust_label_scale

// ----------------------------------------------------------------------

void TreePart::adjust_horizontal_step(TreeImage& aMain, const Tree& aTre, double tree_right_margin)
{
    while (true) {
        const double save_h_step = mHorizontalStep;
        const double save_width = mWidth;
        mHorizontalStep *= 1.05;
        mWidth = tree_width(aMain, aTre, mRootEdge);
        if ((mWidth + mOrigin.x) >= tree_right_margin) {
            mHorizontalStep = save_h_step;
            mWidth = save_width;
            break;
        }
    }

} // TreePart::adjust_horizontal_step

// ----------------------------------------------------------------------

double TreePart::tree_width(TreeImage& aMain, const Node& aNode, double aEdgeLength) const
{
    Surface& surface = aMain.surface();
    double r = 0;
    const double right = (aEdgeLength < 0.0 ? aNode.edge_length : aEdgeLength) * mHorizontalStep;
    if (aNode.is_leaf()) {
        auto const font_size = mVerticalStep * mLabelScale;
        r = surface.text_size(aNode.display_name(), font_size, Surface::FONT_DEFAULT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL).width + name_offset();
    }
    else {
        for (auto node = aNode.subtree.begin(); node != aNode.subtree.end(); ++node) {
            const double node_r = tree_width(aMain, *node);
            if (node_r > r)
                r = node_r;
        }
    }
    return r + right;

} // TreePart::tree_width

// ----------------------------------------------------------------------

const TreePart::BranchAnnotation& TreePart::find_branch_annotation(std::string branch_id) const
{
    auto i = std::find_if(mBranchAnnotations.cbegin(), mBranchAnnotations.cend(), [&branch_id](const auto& ba) { return ba.id == branch_id; });
    return i == mBranchAnnotations.cend() ? mBranchAnnotationsAll : *i;

} // TreePart::find_branch_annotation

// ----------------------------------------------------------------------

json TreePart::dump_to_json() const
{
    json j = {
        {"_comment", "Tree settings, negative values mean default"},
        {"horizontal_step", mHorizontalStep},
        {"line_width", mLineWidth},
        {"label_scale", mLabelScale},
        {"line_color", mLineColor},
        {"name_offset", mNameOffset},
        {"root_edge", mRootEdge},

        {"origin_x", mOrigin.x},
          // for information, not re-read
        {"width", width()},
        {"width_comment", "width is for information only, it is always re-calculated"},
        {"number_of_lines", mNumberOfLines},
        {"number_of_lines_comment", "number_of_lines is for information only"},
        {"vertical_step", mVerticalStep},
        {"vertical_step_comment", "vertical_step is for information only"},
        {"branch_annotations_all", mBranchAnnotationsAll.make_json_for_branch_annotations_all()},
        {"branch_annotations", mBranchAnnotations},
    };

    return j;

} // TreePart::dump_to_json

// ----------------------------------------------------------------------

void TreePart::load_from_json(const json& j)
{
    from_json_if_non_negative(j, "horizontal_step", mHorizontalStep);
    from_json_if_non_negative(j, "line_width", mLineWidth);
    from_json_if_non_negative(j, "label_scale", mLabelScale);
    from_json(j, "line_color", mLineColor);
    from_json_if_non_negative(j, "name_offset", mNameOffset);
    from_json_if_non_negative(j, "root_edge", mRootEdge);
    from_json_if_non_negative(j, "origin_x", mOrigin.x);
    from_json(j, "branch_annotations_all", mBranchAnnotationsAll);

    mBranchAnnotations.clear();
    if (j.count("branch_annotations")) {
        for (auto i = j["branch_annotations"].begin(); i != j["branch_annotations"].end(); ++i) {
            mBranchAnnotations.push_back(*i);
        }
    }

} // TreePart::load_from_json

// ----------------------------------------------------------------------

void TimeSeries::setup(TreeImage& /*aMain*/, const Tree& aTre)
{
    if (show()) {
        auto const mmd = aTre.min_max_date();
        std::cout << "dates in source tree: " << mmd.first << " .. " << mmd.second << "  months: " << (months_between_dates(mmd.first, mmd.second) + 1) << std::endl;
        if (mBegin.empty())
            mBegin.assign_and_remove_day(mmd.first);
        if (mEnd.empty())
            mEnd.assign_and_remove_day(mmd.second);
        mNumberOfMonths = static_cast<size_t>(months_between_dates(mBegin, mEnd)) + 1;
        if (mNumberOfMonths > mMaxNumberOfMonths) {
            mBegin.assign_and_subtract_months(mEnd, mMaxNumberOfMonths - 1);
            assert(months_between_dates(mBegin, mEnd) == static_cast<int>(mMaxNumberOfMonths - 1));
            mNumberOfMonths = mMaxNumberOfMonths;
        }
        std::cout << "dates to show: " << mBegin << " .. " << mEnd << "  months: " << mNumberOfMonths << std::endl;
    }

} // TimeSeries::setup

// ----------------------------------------------------------------------

void TimeSeries::draw(TreeImage& aMain, const Tree& aTre, const Coloring& aColoring, bool aShowSubtreesTopBottom)
{
    if (mNumberOfMonths > 1) {
        draw_labels(aMain);
        draw_month_separators(aMain);
        draw_dashes(aMain, aTre, aColoring);
        if (aShowSubtreesTopBottom)
            draw_subtree_top_bottom(aMain, aTre);
    }

} // TimeSeries::draw

// ----------------------------------------------------------------------

void TimeSeries::draw_labels(TreeImage& aMain)
{
    Surface& surface = aMain.surface();
    const double label_font_size = mMonthWidth * mMonthLabelScale;
    auto const month_max_width = surface.text_size("May ", label_font_size, Surface::FONT_DEFAULT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL).width;
    double x_bearing;
    auto const big_label_size = surface.text_size("May 99", label_font_size, Surface::FONT_DEFAULT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, &x_bearing);
    auto const text_up = (mMonthWidth - big_label_size.height) * 0.5;
    const Viewport& viewport = aMain.viewport();

    draw_labels_at_side(surface, {text_up, viewport.origin.y - big_label_size.width - x_bearing}, label_font_size, month_max_width);
    draw_labels_at_side(surface, {text_up, viewport.opposite().y + x_bearing}, label_font_size, month_max_width);

} // TimeSeries::draw_labels

// ----------------------------------------------------------------------

void TimeSeries::draw_labels_at_side(Surface& surface, const Location& a, double label_font_size, double month_max_width)
{
    Date current_month = mBegin;
    for (size_t month_no = 0; month_no < mNumberOfMonths; ++month_no, current_month.increment_month()) {
        const double left = origin().x + month_no * mMonthWidth + a.x;
        surface.text({left, a.y}, current_month.month_3(), 0, label_font_size, Surface::FONT_DEFAULT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, M_PI_2);
        surface.text({left, a.y + month_max_width}, current_month.year_2(), 0, label_font_size, Surface::FONT_DEFAULT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, M_PI_2);
    }

} // TimeSeries::draw_labels

// ----------------------------------------------------------------------

void TimeSeries::draw_month_separators(TreeImage& aMain)
{
    Surface& surface = aMain.surface();
    auto const bottom = aMain.viewport().opposite().y;
    for (size_t month_no = 0; month_no <= mNumberOfMonths; ++month_no) {
        const double left = origin().x + month_no * mMonthWidth;
        surface.line({left, origin().y}, {left, bottom}, mMonthSeparatorColor, mMonthSeparatorWidth);
    }

} // TimeSeries::draw_month_separators

// ----------------------------------------------------------------------

void TimeSeries::draw_dashes(TreeImage& aMain, const Tree& aTre, const Coloring& aColoring)
{
    Surface& surface = aMain.surface();
    auto const base_x = origin().x + mMonthWidth * (1.0 - mDashWidth) / 2;
    auto const base_y = aMain.tree().origin().y;
    auto const vertical_step = aMain.tree().vertical_step();

    auto draw_dash = [&](const Node& aNode) {
        const int month_no = aNode.months_from(mBegin);
        if (month_no >= 0) {
            const Location a {base_x + mMonthWidth * month_no, base_y + vertical_step * aNode.line_no};
            surface.line(a, {a.x + mMonthWidth * mDashWidth, a.y}, aColoring(aNode), mDashLineWidth, CAIRO_LINE_CAP_ROUND);
        }
    };
    iterate<const Node&>(aTre, draw_dash);

} // TimeSeries::draw_dashes

// ----------------------------------------------------------------------

void TimeSeries::draw_subtree_top_bottom(TreeImage& aMain, const Tree& aTre)
{
    Surface& surface = aMain.surface();
    auto const base_y = aMain.tree().origin().y;
    auto const vertical_step = aMain.tree().vertical_step();
    for (auto entry = mSubtreeTopBottom.cbegin(); entry != mSubtreeTopBottom.end(); ++entry) {
        if (entry->show) {
            auto const nodes = aTre.top_bottom_nodes_of_subtree(entry->branch_id);
            if (nodes.first != nullptr && entry->draw_top)
                surface.line({origin().x, base_y + vertical_step * nodes.first->line_no - vertical_step * 0.5}, {origin().x + width(), base_y + vertical_step * nodes.first->line_no - vertical_step * 0.5}, entry->line_color, entry->line_width);
            if (nodes.second != nullptr && entry->draw_bottom)
                surface.line({origin().x, base_y + vertical_step * nodes.second->line_no + vertical_step * 0.5}, {origin().x + width(), base_y + vertical_step * nodes.second->line_no + vertical_step * 0.5}, entry->line_color, entry->line_width);
        }
    }

} // TimeSeries::draw_subtree_top_bottom

// ----------------------------------------------------------------------

json TimeSeries::dump_to_json() const
{
    json j = {
        {"_comment", "Time series settings, negative values mean default"},
        {"show", mShow},
        {"begin", mBegin},
        {"end", mEnd},
        {"month_width", mMonthWidth},
        {"dash_width", mDashWidth},
        {"dash_width_comment", "relative to month_width"},
        {"month_label_scale", mMonthLabelScale},
        {"max_number_of_months", mMaxNumberOfMonths},
        {"month_separator_color", mMonthSeparatorColor},
        {"month_separator_width", mMonthSeparatorWidth},

        {"origin_x", mOrigin.x},
          // for information, not re-read
        {"width", width()},
        {"width_comment", "width is for information only, it is always re-calculated"},
        {"number_of_months", mNumberOfMonths},
        {"number_of_months_comment", "number_of_months is for information only"},
        {"subtree_top_bottom", mSubtreeTopBottom},
    };

    return j;

} // TimeSeries::dump_to_json

// ----------------------------------------------------------------------

void TimeSeries::load_from_json(const json& j)
{
    from_json(j, "show", mShow, true);
    from_json_if_not_empty(j, "begin", mBegin);
    from_json_if_not_empty(j, "end", mEnd);
    from_json(j, "month_width", mMonthWidth);
    from_json_if_non_negative(j, "dash_width", mDashWidth);
    from_json_if_non_negative(j, "month_label_scale", mMonthLabelScale);
    from_json_if_non_negative(j, "max_number_of_months", mMaxNumberOfMonths);
    from_json(j, "month_separator_color", mMonthSeparatorColor);
    from_json_if_non_negative(j, "month_separator_width", mMonthSeparatorWidth);
    from_json_if_non_negative(j, "origin_x", mOrigin.x);

    mSubtreeTopBottom.clear();
    if (j.count("subtree_top_bottom")) {
        for (auto i = j["subtree_top_bottom"].begin(); i != j["subtree_top_bottom"].end(); ++i) {
            mSubtreeTopBottom.push_back(*i);
        }
    }

} // TimeSeries::load_from_json

// ----------------------------------------------------------------------

void Clades::setup(TreeImage& aMain, const Tree& aTre)
{
      // extract clades from aTre
    std::map<std::string, std::pair<size_t, size_t>> clades; // clade name -> (first line, last-line)
    auto scan = [&clades](const Node& aNode) {
        for (auto c: aNode.clades) {
            auto p = clades.insert(std::make_pair(c, std::make_pair(aNode.line_no, aNode.line_no)));
            if (!p.second && p.first->second.second < aNode.line_no) {
                p.first->second.second = aNode.line_no;
            }
        }
    };
    iterate<const Node&>(aTre, scan);

    for (auto c = clades.begin(); c != clades.end(); ++c) {
          // std::cerr << c->first << ' ' << c->second.first << ' ' << c->second.second << std::endl;
        add_clade(static_cast<int>(c->second.first), static_cast<int>(c->second.second), c->first, c->first);
    }

    assign_slots(aMain);

} // Clades::setup

// ----------------------------------------------------------------------

void Clades::draw(TreeImage& aMain, const Tree& /*aTre*/)
{
    for (auto c = mClades.cbegin(); c != mClades.cend(); ++c) {
        if (c->show) {
            draw_clade(aMain, *c);
        }
    }

} // Clades::draw

// ----------------------------------------------------------------------

void Clades::draw_clade(TreeImage& aMain, const CladeArrow& aClade)
{
    Surface& surface = aMain.surface();
    auto const x = origin().x + aClade.slot * mSlotWidth;
    auto const base_y = aMain.tree().origin().y;
    auto const vertical_step = aMain.tree().vertical_step();
    auto const top = base_y + vertical_step * aClade.begin - mArrowExtra * vertical_step;
    auto const bottom = base_y + vertical_step * aClade.end + mArrowExtra * vertical_step;

    double label_vpos;
    if (aClade.label_position == "top")
        label_vpos = top;
    else if (aClade.label_position == "bottom")
        label_vpos = bottom;
    else
        label_vpos = (top + bottom) / 2.0;
    auto const label_size = surface.text_size(aClade.label, mLabelFontSize, Surface::FONT_DEFAULT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    label_vpos += label_size.height / 2.0 + aClade.label_position_offset;

    surface.double_arrow({x, top}, {x, bottom}, mArrowColor, mLineWidth, mArrowWidth);
    surface.text({x + aClade.label_offset, label_vpos}, aClade.label, mLabelColor, mLabelFontSize, Surface::FONT_DEFAULT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, aClade.label_rotation);
    double separator_left = aMain.tree().origin().x;
    double separator_right = x;
    if (mSeparatorJustInTree)
        separator_right = aMain.tree().origin().x + aMain.tree().width();
    else if (mSeparatorJustInTimeSeries)
        separator_left = aMain.time_series().origin().x;
    if (aClade.begin > 0)
        surface.line({separator_right, top}, {separator_left, top}, mSeparatorColor, mSeparatorWidth);
    if (aClade.end < static_cast<int>(aMain.tree().number_of_lines() - 1))
        surface.line({separator_right, bottom}, {separator_left, bottom}, mSeparatorColor, mSeparatorWidth);

} // Clades::draw_clade

// ----------------------------------------------------------------------

void Clades::assign_slots(TreeImage& aMain)
{
    std::sort(mClades.begin(), mClades.end(), [](const CladeArrow& a, const CladeArrow& b) -> bool { return a.begin == b.begin ? a.end > b.end : a.begin < b.begin; });
    for (auto c = mClades.begin(); c != mClades.end(); ++c) {
        if (c->slot < 0 && c->show)
            c->slot = static_cast<int>(c - mClades.begin());
    }

    Surface& surface = aMain.surface();
      // calculate width
    mWidth = 0;
    for (auto c = mClades.begin(); c != mClades.end(); ++c) {
        auto const width = c->show ? c->slot * mSlotWidth + c->label_offset + surface.text_size(c->label, mLabelFontSize, Surface::FONT_DEFAULT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL).width : 0;
        if (width > mWidth)
            mWidth = width;
    }

} // Clades::assign_slots

// ----------------------------------------------------------------------

json Clades::dump_to_json() const
{
    json j = {
        {"_comment", "Clade marking settings, negative values mean default"},
        {"slot_width", mSlotWidth},
        {"line_width", mLineWidth},
        {"arrow_width", mArrowWidth},
        {"arrow_color", mArrowColor},
        {"arrow_extra", mArrowExtra},
        {"arrow_extra_comment", "fraction of vertical_step to extend arrow up and down"},
        {"label_color", mLabelColor},
        {"label_size", mLabelFontSize},
        {"separator_color", mSeparatorColor},
        {"separator_width", mSeparatorWidth},
        {"separator_just_in_tree", mSeparatorJustInTree},
        {"separator_just_in_tree_comment", "draw clade separator just to cover the tree area and not extend to the time series area"},
        {"separator_just_in_time_series", mSeparatorJustInTimeSeries},
        {"separator_just_in_time_series_comment", "draw clade separator just to cover the time series area and not extend to the tree area"},
        {"origin_x", mOrigin.x},
        {"per_clade", mClades},
          // for information, not re-read
        {"width", width()},
        {"width_comment", "width is for information only, it is always re-calculated"},
    };
    return j;

} // Clades::dump_to_json

// ----------------------------------------------------------------------

void Clades::load_from_json(const json& j)
{
    from_json_if_non_negative(j, "slot_width", mSlotWidth);
    from_json_if_non_negative(j, "line_width", mLineWidth);
    from_json_if_non_negative(j, "arrow_width", mArrowWidth);
    from_json_if_non_negative(j, "arrow_extra", mArrowExtra);
    from_json_if_non_negative(j, "label_size", mLabelFontSize);
    from_json_if_non_negative(j, "separator_width", mSeparatorWidth);

    from_json(j, "arrow_color", mArrowColor);
    from_json(j, "label_color", mLabelColor);
    from_json(j, "separator_color", mSeparatorColor);
    from_json(j, "separator_just_in_tree", mSeparatorJustInTree);
    from_json(j, "separator_just_in_time_series", mSeparatorJustInTimeSeries);

    from_json_if_non_negative(j, "origin_x", mOrigin.x);

    mPerClade.clear();
    if (j.count("per_clade")) {
        for (auto i = j["per_clade"].begin(); i != j["per_clade"].end(); ++i) {
            CladeArrow c(*i);
            mPerClade[c.id] = c;
        }
    }

} // Clades::load_from_json

// ----------------------------------------------------------------------
