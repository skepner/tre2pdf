#include <iostream>
#include <fstream>
#include <cctype>
#include <ctime>
#include <cstdlib>
#include <map>
#include <list>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "tree.hh"
#include "tree-image.hh"
#include "xz.hh"

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

void Node::ladderize()
{
    auto set_max_edge = [](Node& aNode) {
        aNode.ladderize_max_edge_length = aNode.edge_length;
        aNode.ladderize_max_date = aNode.date;
        aNode.ladderize_max_name_alphabetically = aNode.name;
    };

    auto compute_max_edge = [](Node& aNode) {
        auto const max_subtree_edge_node = std::max_element(aNode.subtree.begin(), aNode.subtree.end(), [](auto const& a, auto const& b) { return a.ladderize_max_edge_length < b.ladderize_max_edge_length; });
        aNode.ladderize_max_edge_length = aNode.edge_length + max_subtree_edge_node->ladderize_max_edge_length;
        aNode.ladderize_max_date = std::max_element(aNode.subtree.begin(), aNode.subtree.end(), [](auto const& a, auto const& b) { return a.ladderize_max_date < b.ladderize_max_date; })->ladderize_max_date;
        aNode.ladderize_max_name_alphabetically = std::max_element(aNode.subtree.begin(), aNode.subtree.end(), [](auto const& a, auto const& b) { return a.ladderize_max_name_alphabetically < b.ladderize_max_name_alphabetically; })->ladderize_max_name_alphabetically;
    };

      // set max_edge_length field for every node
    iterate<Node&>(*this, set_max_edge, &nope, compute_max_edge);

    auto reorder_subtree_cmp = [](const Node& a, const Node& b) -> bool {
        bool r = false;
        if (std::abs(a.ladderize_max_edge_length - b.ladderize_max_edge_length) < std::numeric_limits<double>::epsilon()) {
            if (a.ladderize_max_date == b.ladderize_max_date) {
                r = a.ladderize_max_name_alphabetically < b.ladderize_max_name_alphabetically;
            }
            else {
                r = a.ladderize_max_date < b.ladderize_max_date;
            }
        }
        else {
            r = a.ladderize_max_edge_length < b.ladderize_max_edge_length;
        }
        return r;
    };

    auto reorder_subtree = [&reorder_subtree_cmp](Node& aNode) {
        std::sort(aNode.subtree.begin(), aNode.subtree.end(), reorder_subtree_cmp);
    };

      // re-order subtree based on max_edge_length of its nodes
    iterate<Node&>(*this, &nope, &nope, reorder_subtree);

} // Node::ladderize

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

std::pair<const Node*, const Node*> Tree::top_bottom_nodes_of_subtree(std::string branch_id) const
{
    const Node* root = find_node(*this, [branch_id](const Node& aNode) -> bool { return aNode.branch_id == branch_id; });
    return std::make_pair(root != nullptr ? &find_first_leaf(*root) : nullptr, root != nullptr ? &find_last_leaf(*root) : nullptr);

} // Tree::top_bottom_nodes_of_subtree

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

void Tree::fix_labels()
{
    const std::list<std::pair<std::string, size_t>> to_remove {
        {"/HUMAN/", 6},
        {"(H3N2)/", 6},
        {"(H1N1)/", 6},
    };

    auto fix_human = [&to_remove](Node& aNode) -> void {
        for (auto e: to_remove) {
            auto const pos = aNode.name.find(e.first);
            if (pos != std::string::npos)
                aNode.name.erase(pos, e.second);
              // replace __ with a space to handle seq_id
            auto const pos__ = aNode.name.find("__");
            if (pos__ != std::string::npos)
                aNode.name.replace(pos__, 2, " ");
        }
    };
    iterate<Node&>(*this, fix_human);

} // Tree::fix_labels

// ----------------------------------------------------------------------

void load_from_json(Node& aNode, const json& j)
{
    if (j.count("edge_length"))
        aNode.edge_length = j["edge_length"];
    if (j.count("name"))
        aNode.name = j["name"];
    if (j.count("aa_at"))
        aNode.aa_at = j["aa_at"];
    if (j.count("subtree")) {
        auto subtree = j["subtree"];
        if (!subtree.is_array())
            throw std::runtime_error(std::string("cannot import tree: unrecognized subtree: ") + subtree.dump());
        for (auto e = subtree.begin(); e != subtree.end(); ++e) {
            Node node;
            load_from_json(node, *e);
            aNode.subtree.push_back(std::forward<Node>(node));
        }
        if (j.count("number_strains"))
            aNode.number_strains = j["number_strains"];
        if (j.count("id"))
            aNode.branch_id = j["id"];
    }
    else {
        if (j.count("date"))
            aNode.date = j["date"];
        if (j.count("continent"))
            aNode.continent = j["continent"];
        if (j.count("clades"))
            aNode.clades = static_cast<const std::vector<std::string>&>(j["clades"]);
    }

} // load_from_json

// ----------------------------------------------------------------------

json dump_to_json(const Node& aNode)
{
    json j = {{"edge_length", aNode.edge_length}};
    if (!aNode.name.empty())
        j["name"] = aNode.name;
    if (aNode.is_leaf()) {
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
        if (aNode.number_strains >= 0)
            j["number_strains"] = aNode.number_strains;
        if (!aNode.branch_id.empty())
            j["id"] = aNode.branch_id;
    }
    return j;
}

// ----------------------------------------------------------------------

void tree_from_json(Tree& aTree, std::string aSource, TreeImage& aTreeImage)
{
    auto j = json::parse(aSource);
    if (j["  version"] != TREE_JSON_DUMP_VERSION)
        throw std::runtime_error(std::string("cannot import tree: unsupported version") + j["version"].get<std::string>());
    load_from_json(aTree, j["tree"]);
    if (j.count("_settings"))
        aTreeImage.load_from_json(j["_settings"]);

} // tree_from_json

// ----------------------------------------------------------------------

void tree_to_json(Tree& aTree, std::string aFilename, std::string aCreator, const TreeImage& aTreeImage)
{
    char date_buf[100];
    std::time_t t = std::time(nullptr);
    std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M %Z", std::localtime(&t));

    json j = {
        {"  version", TREE_JSON_DUMP_VERSION},
        {"_settings", aTreeImage.dump_to_json()},
        {"updated", {{
                {"user", std::getenv("USER")},
                {"date", date_buf},
                {"creator", aCreator},
                }}},
        {"tree", dump_to_json(aTree)},
    };
    std::string output = j.dump(2);
    if (aFilename == "-") {
        std::cout << output << std::endl;
    }
    else {
        if (aFilename.substr(aFilename.size() - 3) == ".xz")
            output = xz_compress(output);
        int fd = open(aFilename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
            throw std::runtime_error(std::string("cannot write ") + aFilename + ": " + std::strerror(errno));
        write(fd, output.c_str(), output.size());
        close(fd);
    }

} // tree_to_json

// ----------------------------------------------------------------------
