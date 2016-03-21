#include <iostream>
#include <string>

#include "command-line-arguments.hh"

#include "tree.hh"
#include "tree-import.hh"

// ----------------------------------------------------------------------

// fake TreeImage to avoid linking in real one
class TreeImage
{
 public:
    void load_from_json(const json&);
    json dump_to_json() const;
};

void TreeImage::load_from_json(const json&) {}
json TreeImage::dump_to_json() const { return json(); }

// ----------------------------------------------------------------------
int main(int argc, const char *argv[])
{
    using command_line_arguments::Help;
    using command_line_arguments::Arg;
    auto cl = command_line_arguments::make_command_line_arguments
            (
                Arg<command_line_arguments::PrintHelp>('h', "help", "Usage: {progname} [options] <source1.json> <source2.json>", Help("print this help screen"))
             );
    cl->min_max(2, 2);                  // two arguments expected
    try {
        cl->parse(argc, argv);
    }
    catch (command_line_arguments::CommandLineError& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        cl->print_help(std::cerr);
        return 1;
    }

    int exit_code = 0;
    try {
        Tree tre1, tre2;
        TreeImage tree_image;
        import_tree(tre1, cl->arg(0), tree_image);
        import_tree(tre2, cl->arg(1), tree_image);
    }
    catch (std::exception& err) {
        std::cerr << err.what() << std::endl;
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
