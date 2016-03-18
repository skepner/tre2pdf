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

    inline Node() : edge_length(0), line_no(0), number_strains(1) {}
    inline Node(Node&&) = default;
      // inline Node(const Node& a) : edge_length(a.edge_length), name(a.name), date(a.date), line_no(a.line_no), subtree(a.subtree) { std::cout << "COPY " << (void*)&a << " --> " << (void*)this << ' ' << a.line_no << ' ' << line_no << std::endl; }
    inline Node(std::string aName, double aEdgeLength, const Date& aDate = Date()) : edge_length(aEdgeLength), name(aName), date(aDate), line_no(0), number_strains(1) {}
    inline Node& operator=(Node&&) = default; // needed for swap needed for sort

    double edge_length;              // indent of node or subtree
    std::string name;                // node name or branch annotation

      // name part
    Date date;
    size_t line_no;             // line at which the name is drawn

      // for coloring
    std::string continent;
    std::vector<std::string> clades;
    json aa_at;

      // subtree part
    Subtree subtree;
    double top, bottom;         // subtree boundaries
    int number_strains;         // number of strains in subtree (generated by tre-seqdb --pos)
    std::string branch_id;      // generated by tre-seqdb

      // for ladderizing
    double ladderize_max_edge_length;
    Date ladderize_max_date;
    std::string ladderize_max_name_alphabetically;

    inline bool is_leaf() const { return subtree.empty() && !name.empty(); }
    inline double middle() const { return is_leaf() ? static_cast<double>(line_no) : ((top + bottom) / 2.0); }
    std::pair<double, size_t> width_height() const;
    int months_from(const Date& aStart) const; // returns negative if date of the node is earlier than aStart
    void ladderize();

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
    void print_edges(std::ostream& out) const;
    void fix_labels();
    std::pair<Date, Date> min_max_date() const;
    std::pair<double, double> min_max_edge() const;
    std::pair<const Node*, const Node*> top_bottom_nodes_of_subtree(std::string branch_id) const;


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

template <typename P> inline const Node* find_node(const Node& aNode, P predicate)
{
    const Node* r = nullptr;
    if (predicate(aNode)) {
        r = &aNode;
    }
    else if (! aNode.is_leaf()) {
        for (auto node = aNode.subtree.begin(); r == nullptr && node != aNode.subtree.end(); ++node) {
            r = find_node(*node, predicate);
        }
    }
    return r;
}

// ----------------------------------------------------------------------

inline const Node& find_first_leaf(const Node& aNode)
{
    return aNode.is_leaf() ? aNode : find_first_leaf(aNode.subtree.front());
}

inline const Node& find_last_leaf(const Node& aNode)
{
    return aNode.is_leaf() ? aNode : find_last_leaf(aNode.subtree.back());
}

// ----------------------------------------------------------------------

class TreeImage;

constexpr const char* TREE_JSON_DUMP_VERSION = "phylogenetic-tree-v1";

json dump_to_json(const Node& aNode);
void load_from_json(Node& aNode, const json& j);
void tree_from_json(Tree& aTree, std::string aSource, TreeImage& aTreeImage);
void tree_to_json(Tree& aTree, std::string aFilename, std::string aCreator, const TreeImage& aTreeImage);

// ----------------------------------------------------------------------
