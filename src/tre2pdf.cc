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
                Arg<bool>("continents", false, Help("color code by continent")),
                Arg<std::string>("pos", std::string(), Help("color code by pos")),
                Arg<bool>("clades", false, Help("show clades")),
                Arg<bool>("edges", false, Help("print edges")),
                Arg<bool>("show-branch-ids", false, Help("show branch ids")),
                Arg<bool>("show-subtree-top-bottom", false, Help("show subtree top bottom (see _settings.time_series.subtree_top_bottom in json)")),
                Arg<bool>("fix-labels", false, Help("Remove /HUMAN/ from labels, remove (H3N2) atc. from labels before drawing them")),
                Arg<bool>("ladderize", false, Help("Ladderize the tree before drawing")),
                Arg<int>("number-strains-threshold", 0, Help("Do not put branch annotation if \"number_strains\" for the branch is less than this value.")),
                Arg<std::string>("save", std::string(), Help("Save ladderized tree, - for stdout")),
                Arg<command_line_arguments::PrintHelp>('h', "help", "Usage: {progname} [options] <source.json> <output.pdf>", Help("print this help screen"))
             );
    cl->min_max(2, 2);                  // one argument expected
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
        Tree tre;
        TreeImage tree_image;
        import_tree(tre, cl->arg(0), tree_image);
        if (cl->get<bool>("ladderize")) {
            tre.ladderize();
        }
        if (!cl->get<std::string>("save").empty()) {
            std::string creator(argv[0]);
            auto const last_slash = creator.rfind('/');
            if (last_slash != std::string::npos)
                creator.erase(0, last_slash + 1);
            tree_to_json(tre, cl->get<std::string>("save"), creator, tree_image);
        }
        tre.analyse();

          // auto const wh = tre.width_height();
          // std::cout << "w:" << wh.first << " h:" << wh.second << std::endl;

        if (cl->get<bool>('p'))
            tre.print(std::cout);

        if (cl->get<bool>("edges")) {
            tre.print_edges(std::cout);
            auto min_max = tre.min_max_edge();
            std::cout << "min: " << min_max.first << "  max: " << min_max.second << std::endl;
        }

        std::unique_ptr<Coloring> coloring(new ColoringBlack());
        if (cl->get<bool>("continents"))
            coloring = std::unique_ptr<Coloring>(TreeImage::coloring_by_continent());
        else if (!cl->get<std::string>("pos").empty())
            coloring = std::unique_ptr<Coloring>(TreeImage::coloring_by_pos(cl->get<std::string>("pos"), tre));

        if (cl->get<bool>("fix-labels"))
            tre.fix_labels();

        tree_image.clades().show(cl->get<bool>("clades"));
        tree_image.make_pdf(cl->arg(1), tre, *coloring, cl->get<int>("number-strains-threshold"), cl->get<bool>("show-branch-ids"), cl->get<bool>("show-subtree-top-bottom"));
        std::cout << "Computed values (can be inserted into source.json at \"_settings\" key):" << std::endl << tree_image.dump_to_json().dump(2) << std::endl;
    }
    catch (std::exception& err) {
        std::cerr << err.what() << std::endl;
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
