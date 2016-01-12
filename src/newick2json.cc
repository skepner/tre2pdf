#include <iostream>
#include <string>

#include "command-line-arguments.hh"

#include "tree.hh"
#include "tree-image.hh"
#include "tree-import.hh"

// ----------------------------------------------------------------------

int main(int argc, const char *argv[])
{
    using command_line_arguments::Help;
    using command_line_arguments::Arg;
    auto cl = command_line_arguments::make_command_line_arguments
            (
                Arg<bool>('p', false, Help("print tree")),
                Arg<command_line_arguments::PrintHelp>('h', "help", "Reads tree from newick formatted file and outputs its representation into json for furhter processing.\nUsage: {progname} [options] <source.tre> <output.json>\nUse - for input and/or output files to use stdin/stdout.", Help("print this help screen"))
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

      // --------------------------------------------------

    int exit_code = 0;
    try {
        Tree tre;
        TreeImage tree_image;
        import_tree(tre, cl->arg(0), tree_image);
        if (cl->get<bool>('p'))
            tre.print(std::cout);
        tree_to_json(tre, cl->arg(1), "newick2json", tree_image);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << std::endl;
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
