# Where Things Live: build both editions of the book.
#
#   make          build the PDF and the web edition into dist/
#   make pdf      dist/where-things-live.pdf
#   make html     dist/html/index.html
#   make test     compile and run every C++ program in the book (with sanitizers)
#   make check    test the code, build everything, and load every page headlessly
#   make clean    remove build/ and dist/
#
# Needs: python3 (with PyYAML), pandoc 3+, pdflatex + bibtex (TeX Live),
# g++ 11+ or clang++ 14+.  `make check` also needs Google Chrome.

.PHONY: all pdf html test check clean

all:
	python3 tools/build.py all

pdf:
	python3 tools/build.py pdf

html:
	python3 tools/build.py html

test:
	$(MAKE) -C code test

check: test all
	tools/vizcheck.sh
	tools/vizstress.sh

clean:
	rm -rf build dist
