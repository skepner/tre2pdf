#include <iostream>
#include <fstream>
#include <cctype>
#include <ctime>
#include <cstdlib>
#include <map>
#include <list>

#include "tree.hh"
#include "tree-image.hh"

// ----------------------------------------------------------------------

std::pair<double, size_t> Node::width_height() const
{
    size_t height = 0;
    double width = 0;
    if (is_leaf()) {
        ++height;
    }
    else {
        for (auto node = subtree.begin(); node != subtree.end(); ++node) {
            auto const wh = node->width_height();
            if (wh.first > width)
                width = wh.first;
            height += wh.second;
        }
    }
    return std::make_pair(width + edge_length, height);

} // Node::width_height

// ----------------------------------------------------------------------

int Node::months_from(const Date& aStart) const
{
    return date.empty() ? -1 : months_between_dates(aStart, date);

} // Node::month_no

// ----------------------------------------------------------------------

void Tree::analyse()
{
      // set line_no for each name node
    size_t current_line = 0;
    auto set_line_no = [&current_line](Node& aNode) {
        aNode.line_no = current_line;
        ++current_line;
    };
      // set top and bottom for each subtree node
    auto set_top_bottom = [](Node& aNode) {
        aNode.top = aNode.subtree.begin()->middle();
        aNode.bottom = aNode.subtree.rbegin()->middle();
    };
    iterate<Node&>(*this, set_line_no, &nope, set_top_bottom);

} // Tree::analyse

// ----------------------------------------------------------------------

std::pair<Date, Date> Tree::min_max_date() const
{
    Date min_date, max_date;
    auto min_max_date = [&min_date, &max_date](const Node& aNode) -> void {
        if (!aNode.date.empty()) {
            if (min_date.empty() || aNode.date < min_date)
                min_date = aNode.date;
            if (max_date.empty() || max_date < aNode.date)
                max_date = aNode.date;
        }
    };
    iterate<const Node&>(*this, min_max_date);
    return std::make_pair(min_date, max_date);

} // Tree::min_max_date

// ----------------------------------------------------------------------

std::pair<double, double> Tree::min_max_edge() const
{
    double min_edge = 1e99, max_edge = 0.0;
    auto min_max_edge = [&min_edge, &max_edge](const Node& aNode) -> void {
        if (aNode.edge_length > 0.0) {
            if (aNode.edge_length < min_edge)
                min_edge = aNode.edge_length;
            if (max_edge < aNode.edge_length)
                max_edge = aNode.edge_length;
        }
    };
    iterate<const Node&>(*this, min_max_edge, min_max_edge);
    return std::make_pair(min_edge, max_edge);

} // Tree::min_max_edge

// ----------------------------------------------------------------------

void Tree::print(std::ostream& out) const
{
    size_t indent = 0;
    auto p_name = [&out, &indent](const Node& aNode) {
        out << std::string(indent, ' ') << /* aNode.line_no << " -- " << */ aNode.display_name();
        if (aNode.edge_length >= 0)
            out << ':' << aNode.edge_length;
        out << std::endl;
    };
    auto p_subtree_pre = [&out, &indent](const Node& /*aNode*/) {
        out << std::string(indent, ' ') << '(' << /* ' ' << aNode.top << ' ' << aNode.bottom << */ std::endl;
        indent += 2;
    };
    auto p_subtree_post = [&out, &indent](const Node& aNode) {
        indent -= 2;
        out << std::string(indent, ' ') << ')';
        if (aNode.edge_length >= 0)
            out << ':' << aNode.edge_length;
        out << std::endl;
    };
    iterate(*this, p_name, p_subtree_pre, p_subtree_post);

} // Tree::print

// ----------------------------------------------------------------------

void Tree::print_edges(std::ostream& out) const
{
    std::map<double, size_t> edges; // edge length to number of occurences
    auto collect_edges = [&edges](const Node& aNode) -> void {
        auto iter_inserted = edges.insert(std::make_pair(aNode.edge_length, 1));
        if (!iter_inserted.second)
            ++iter_inserted.first->second;
    };
    iterate<const Node&>(*this, collect_edges, collect_edges);
    typedef std::pair<double, size_t> E;
    std::list<E> edges_l(edges.begin(), edges.end());
    edges_l.sort([](const E& a, const E& b) { return a.first < b.first; });
    for (auto e: edges_l) {
        out << e.first << " " << e.second << std::endl;
    }

} // Tree::print_edges

// ----------------------------------------------------------------------

void Tree::fix_human_in_labels()
{
    auto fix_human = [](Node& aNode) -> void {
        auto const pos = aNode.name.find("/HUMAN/");
        if (pos != std::string::npos)
            aNode.name.erase(pos, 6);
    };
    iterate<Node&>(*this, fix_human);

} // Tree::fix_human_in_labels

// ----------------------------------------------------------------------

json dump_to_json(const Node& aNode)
{
    json j = {{"edge_length", aNode.edge_length}};
    if (aNode.is_leaf()) {
        j["name"] = aNode.name;
        if (!aNode.date.empty())
            j["date"] = aNode.date.display();
        if (!aNode.continent.empty())
            j["continent"] = aNode.continent;
        if (!aNode.clades.empty())
            j["clades"] = aNode.clades;
    }
    else {
        std::vector<json> subtree;
        for (auto node = aNode.subtree.cbegin(); node != aNode.subtree.cend(); ++node) {
            subtree.push_back(dump_to_json(*node));
        }
        j["subtree"] = subtree;
    }
    return j;
}

// ----------------------------------------------------------------------

void tree_from_json(Tree& aTree, std::string aSource, TreeImage& aTreeImage)
{
    auto j = json::parse(aSource);
    if (j["version"] != TREE_JSON_DUMP_VERSION)
        throw std::runtime_error(std::string("cannot import tree: unsupported version") + j["version"].get<std::string>());
    load_from_json(aTree, j["tree"]);
    if (j.count("_settings"))
        aTreeImage.load_from_json(j["_settings"]);

} // tree_from_json

// ----------------------------------------------------------------------

void load_from_json(Node& aNode, const json& j)
{
    if (j.count("edge_length"))
        aNode.edge_length = j["edge_length"];
    if (j.count("subtree")) {
        auto subtree = j["subtree"];
        if (!subtree.is_array())
            throw std::runtime_error(std::string("cannot import tree: unrecognized subtree: ") + subtree.dump());
        for (auto e = subtree.begin(); e != subtree.end(); ++e) {
            Node node;
            load_from_json(node, *e);
            aNode.subtree.push_back(std::forward<Node>(node));
        }
    }
    else {
        aNode.name = j["name"];
        if (j.count("date"))
            aNode.date = j["date"];
        if (j.count("continent"))
            aNode.continent = j["continent"];
        if (j.count("clades"))
            aNode.clades = static_cast<const std::vector<std::string>&>(j["clades"]);
    }

} // load_from_json

// ----------------------------------------------------------------------

void tree_to_json(Tree& aTree, std::string aFilename, std::string aCreator, const TreeImage& aTreeImage)
{
    char date_buf[100];
    std::time_t t = std::time(nullptr);
    std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M %Z", std::localtime(&t));

    json j = {
        {"version", TREE_JSON_DUMP_VERSION},
        {"updated", {{
                {"user", std::getenv("USER")},
                {"date", date_buf},
                {"creator", aCreator},
                }}},
        {"tree", dump_to_json(aTree)},
        {"_settings", aTreeImage.dump_to_json()},
    };
    if (aFilename == "-") {
        std::cout << j.dump(2) << std::endl;
    }
    else {
        std::ofstream out(aFilename);
        if (!out)
            throw std::runtime_error(std::string("cannot write ") + aFilename);
        out << j.dump(2) << std::endl;
    }

} // tree_to_json

// ----------------------------------------------------------------------
