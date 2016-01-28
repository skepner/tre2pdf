# -*- Makefile -*-
# Eugene Skepner 2015
# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

SOURCES_DIR = src

TRE2PDF_SOURCES = tre2pdf.cc tree.cc tree-image.cc color.cc
NEWICK2JSON_SOURCES = newick2json.cc tree.cc tree-image.cc color.cc

# ----------------------------------------------------------------------

CLANG = $(shell if g++ --version 2>&1 | grep -i llvm >/dev/null; then echo Y; else echo N; fi)
ifeq ($(CLANG),Y)
  WEVERYTHING = -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded
  STD = c++14
else
  WEVERYTHING = -Wall -Wextra
  STD = c++14 # c++11
endif

WARNINGS = # -Wno-padded
OPTIMIZATION = # -O3
CXXFLAGS = -MMD -g $(OPTIMIZATION) -std=$(STD) $(WEVERYTHING) $(WARNINGS) -I$(BUILD)/include $$(pkg-config --cflags cairo)
LDFLAGS =
TRE2PDF_LDLIBS = $$(pkg-config --libs cairo)
NEWICK2JSON_LDLIBS = $$(pkg-config --libs cairo)

# ----------------------------------------------------------------------

BUILD = build
DIST = dist

all: $(DIST)/newick2json $(DIST)/tre2pdf

-include $(BUILD)/*.d

# ----------------------------------------------------------------------

EUPA_MAKEFILE ?= eupa/Makefile.eupa
EUPA_DIST ?= dist
EUPA_BUILD ?= build

ifeq ($(findstring clean,$(MAKECMDGOALS)),)
  ifeq ($(wildcard $(EUPA_MAKEFILE)),)
    ifeq ($(shell hostname):$(USER),jagd:eu)
      $(shell git clone git@github.com:skepner/eupa.git)
    else
      $(shell git clone https://github.com/skepner/eupa.git)
    endif
  endif
  include $(EUPA_MAKEFILE)
endif

# ----------------------------------------------------------------------

$(DIST)/newick2json: $(patsubst %.cc,$(BUILD)/%.o,$(NEWICK2JSON_SOURCES)) | $(DIST)
	g++ $(LDFLAGS) -o $@ $^ $(NEWICK2JSON_LDLIBS)

$(DIST)/tre2pdf: $(patsubst %.cc,$(BUILD)/%.o,$(TRE2PDF_SOURCES)) | $(DIST)
	g++ $(LDFLAGS) -o $@ $^ $(TRE2PDF_LDLIBS)

test: all
	# $(DIST)/tre2pdf --continents --clades /tmp/d.json /tmp/t.pdf && open /tmp/t.pdf
	$(DIST)/newick2json trees/a.tre -
	# $(DIST)/newick2json trees/a.tre - | ./scripts/tre-continent - -
	# $(DIST)/newick2json trees/a.tre - | ./scripts/tre-continent - - | ./scripts/tre-clade - ~/ac/results/ssm/2015-1215-ssm-nh-2016-tc1/sequences/fasta-all/B.fas trees/clade-desc.json -
	#$(DIST)/newick2json trees/a.tre - | ./scripts/tre-continent - - | ./scripts/tre-clade - ~/ac/results/ssm/2015-1215-ssm-nh-2016-tc1/sequences/fasta-all/B.fas trees/clade-desc.json - | $(DIST)/tre2pdf --continents --clades - /tmp/t.pdf && open /tmp/t.pdf

clean:
	rm -rf $(DIST) $(BUILD)/*.o $(BUILD)/*.d

distclean: clean
	rm -rf $(BUILD) eupa

# ----------------------------------------------------------------------

$(BUILD)/%.o: $(SOURCES_DIR)/%.cc | $(BUILD) axe command-line-arguments cairo jsonhpp
	g++ $(CXXFLAGS) -c -o $@ $<

# ----------------------------------------------------------------------

$(DIST):
	$(call make_target_dir,$(DIST),DIST)

$(BUILD):
	$(call make_target_dir,$(BUILD),BUILD)
