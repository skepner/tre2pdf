// Newick tree parser

#pragma once

#include <iostream>
#include <sstream>
#include <stack>
#include <regex>

#include "axe/axe.h"

#include "tree.hh"

// ----------------------------------------------------------------------

typedef std::stack<Node::Subtree*> NodeStack;

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wweak-vtables"
#endif
class ParsingError : public std::runtime_error
{
 public: using std::runtime_error::runtime_error;
};
#pragma GCC diagnostic pop

template <typename I> void parse_newick(Tree& tree, I begin, I end)
{
      // auto print = axe::e_ref([](I b, I e) { std::string n(b, e); std::cout << "-->" << n << std::endl;});

    auto parsing_failure = [begin](const char* message) -> auto {
        return axe::e_ref([begin, message](I b, I e) {
                std::stringstream m;
                m << message << " at " << (b - begin) << ": \"" << std::string(b, std::min(b + 40, e)) << "\"";
                throw ParsingError(m.str());
            });
    };

    std::regex re_date(".+-[12][09][0-9][0-9]-[01][0-9]-[0-3][0-9]$");
    auto decode_name = [&re_date](std::string aName) -> std::pair<std::string, Date> {
        std::string::size_type pos = 0;
        while ((pos = aName.find('%', pos)) != std::string::npos && (pos + 2) < aName.size()) {
            if (std::isxdigit(aName[pos + 1]) && std::isxdigit(aName[pos + 2])) {
                auto const c = static_cast<char>(std::strtoul(aName.substr(pos + 1, 2).c_str(), nullptr, 16));
                aName.replace(pos, 3, 1, c);
            }
            ++pos;
        }
        Date date;
        if (std::regex_match(aName, re_date)) {
            date.parse(aName.substr(aName.size() - 10));
            aName.erase(aName.size() - 11);
        }
        return std::make_pair(aName, date);
    };


    auto space = axe::r_any(" \t\n\r");
    auto open_paren = axe::r_lit('(');
    auto close_paren = axe::r_lit(')');
    auto colon = axe::r_lit(':');
    auto semicolon = axe::r_lit(';');
    auto comma = axe::r_lit(',');

    auto name = +(axe::r_any("!\"#$%&'*+-./<=>?@[\\]^_`{|}~") | axe::r_alnum());

    NodeStack current_node;
    current_node.push(&tree.subtree);

    constexpr double default_edge_length = 0.0;
    double extracted_edge_length = default_edge_length;
    std::string extracted_name;

    auto add_name = axe::e_ref([&current_node, &extracted_name, &extracted_edge_length, &decode_name](I, I) {
              // std::cout << "+>" << extracted_name << ':' << extracted_edge_length << std::endl;
            auto const name_date = decode_name(extracted_name);
            current_node.top()->push_back(Node(name_date.first, extracted_edge_length, name_date.second));
            extracted_name.clear();
            extracted_edge_length = default_edge_length;
        });

    auto new_subtree = axe::e_ref([&current_node](I, I) {
              // std::cout << "+(" << std::endl;
            current_node.top()->push_back(Node());
            current_node.push(&current_node.top()->rbegin()->subtree);
        });

    auto end_subtree = axe::e_ref([&current_node, &extracted_edge_length](I, I) {
              // std::cout << "+):" << extracted_edge_length << std::endl;
            current_node.pop();
            current_node.top()->rbegin()->edge_length = extracted_edge_length;
            extracted_edge_length = default_edge_length;
        });

    auto end_root_tree = axe::e_ref([&tree, &extracted_edge_length](I, I) {
              // std::cout << "end_root_tree " << extracted_edge_length << std::endl;
            if (extracted_edge_length >= 0.0)
                tree.edge_length = extracted_edge_length;
            extracted_edge_length = default_edge_length;
        });

      // auto element_edge_length = axe::r_double(extracted_edge_length) | axe::r_ufixed(extracted_edge_length) | axe::r_udecimal(extracted_edge_length);
    auto element_edge_length = axe::r_double(extracted_edge_length);
    auto element_edge_length_with_colon = colon > axe::r_many(space, 0) > element_edge_length > axe::r_many(space, 0);
    auto element_name = ((name >> extracted_name) > *space > ~element_edge_length_with_colon) >> add_name;
    axe::r_rule<I> subtree;
    auto element_subtree = subtree > *space > ~element_edge_length_with_colon >> end_subtree;
    auto element = *space & (element_name | element_subtree | axe::r_fail(parsing_failure("either name or subtree expected"))) & *space;
    auto subtree_content = element > axe::r_many(comma & (element | axe::r_fail(parsing_failure("either name or subtree expected"))), 0);
    subtree = (open_paren >> new_subtree) > subtree_content > close_paren;
    auto root_tree = open_paren > subtree_content > close_paren > *space > ~element_edge_length_with_colon >> end_root_tree;
    auto tre = *space > root_tree > *space > semicolon > *space;

    try {
        tre(begin, end);
    }
    catch (axe::failure<char>& err) {
        throw ParsingError(err.message());
    }
}

// ----------------------------------------------------------------------
