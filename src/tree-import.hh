#pragma once

#include "read-file.hh"
#include "newick.hh"

// ----------------------------------------------------------------------

class TreeImage;

inline void import_tree(Tree& tree, std::string buffer, TreeImage& aTreeImage)
{
    if (buffer == "-")
        buffer = read_stdin();
    else if (file_exists(buffer))
        buffer = read_file(buffer);
    if (buffer[0] == '(')
        parse_newick(tree, std::begin(buffer), std::end(buffer));
    else if (buffer[0] == '{')
        tree_from_json(tree, buffer, aTreeImage);
    else
        throw std::runtime_error("cannot import tree: unrecognized source format");
}

// ----------------------------------------------------------------------
