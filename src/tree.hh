#pragma once

#include <string>
#include <vector>

#include "json.hh"
#include "date.hh"

// ----------------------------------------------------------------------

class Node
{
 public:
    typedef std::vector<Node> Subtree;

    inline Node() : edge_length(0), line_no(0) {}
    inline Node(Node&&) = default;
      // inline Node(const Node& a) : edge_length(a.edge_length), name(a.name), date(a.date), line_no(a.line_no), subtree(a.subtree) { std::cout << "COPY " << (void*)&a << " --> " << (void*)this << ' ' << a.line_no << ' ' << line_no << std::endl; }
    inline Node(std::string aName, double aEdgeLength, const Date& aDate = Date()) : edge_length(aEdgeLength), name(aName), date(aDate), line_no(0) {}

    double edge_length;              // indent of node or subtree

      // name part
    std::string name;
    Date date;
    size_t line_no;             // line at which the name is drawn

      // for coloring
    std::string continent;
    std::vector<std::string> clades;

      // subtree part
    Subtree subtree;
    double top, bottom;         // subtree boundaries

    inline bool is_leaf() const { return subtree.empty() && !name.empty(); }
    inline double middle() const { return is_leaf() ? static_cast<double>(line_no) : ((top + bottom) / 2.0); }
    std::pair<double, size_t> width_height() const;
    int months_from(const Date& aStart) const; // returns negative if date of the node is earlier than aStart

    inline std::string display_name() const
        {
            if (is_leaf()) {
                auto r = name;
                if (!date.empty()) {
                    r.append(" ");
                    r.append(date);
                }
                return r;
            }
            else {
                throw std::runtime_error("Node is not a name node");
            }
        }

 private:
    inline Node(const Node&) = default;

}; // class Node

// ----------------------------------------------------------------------

class Tree : public Node
{
 public:
    void analyse();
    void print(std::ostream& out) const;
    std::pair<Date, Date> min_max_date() const;

}; // class Tree

// ----------------------------------------------------------------------

template <typename N> inline void nope(N&) {}

  // Calls f_name for name nodes only, calls f_subtree for subtrees nodes only
template <typename N, typename F1 = void(*)(N&), typename F2 = void(*)(N&), typename F3 = void(*)(N&)> inline void iterate(N& aNode, F1 f_name, F2 f_subtree_pre = nope, F3 f_subtree_post = nope)
{
    if (aNode.is_leaf()) {
        f_name(aNode);
    }
    else {
        f_subtree_pre(aNode);
        for (auto node = aNode.subtree.begin(); node != aNode.subtree.end(); ++node) {
            iterate(*node, f_name, f_subtree_pre, f_subtree_post);
        }
        f_subtree_post(aNode);
    }
}

// ----------------------------------------------------------------------

class TreeImage;

constexpr const char* TREE_JSON_DUMP_VERSION = "phylogenetic-tree-v1";

json dump_to_json(const Node& aNode);
void load_from_json(Node& aNode, const json& j);
void tree_from_json(Tree& aTree, std::string aSource, TreeImage& aTreeImage);
void tree_to_json(Tree& aTree, std::string aFilename, std::string aCreator, const TreeImage& aTreeImage);

// ----------------------------------------------------------------------
