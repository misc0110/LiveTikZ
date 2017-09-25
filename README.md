# LiveTikZ
A live preview for TikZ drawings.

# Usage

LiveTikZ is a split-window application, consisting of a powerful text editor for writing TikZ code and a live preview which automatically renders the TikZ code as image. 

LiveTikZ uses KDEs [KatePart](https://kate-editor.org/about-katepart/) as powerful rich-text editor component and thus provides all the features known from Kate text editor, which includes

* Syntax Highlighting
* Text and bracket completion
* Code folding
* Automatic indentation
* Search and replace including escape sequences and regular expressions
* Dynamic and static word wrap
* Block selection mode
* VI Input mode

To render the TikZ code, LiveTikZ relies on pdflatex (either directly or through the [latexrun](https://github.com/aclements/latexrun) wrapper) to compile the code and [Poppler](https://github.com/danigm/poppler) to render the PDF file. 

# Installation

LiveTikZ is a Qt application with KDE dependencies for the editor component. LiveTikZ can either be installed using the provided Debian packages or compiled from source. 

## Prerequisites

The following packages are required to compile from source:

* build-essential
* cmake
* libqt4-dev
* qt4-dev-tools
* libpoppler-qt4-dev
* kdelibs5-dev

## Compile

To compile LiveTikZ, simply run

```
mkdir build
cd build
cmake ..
make -j
```

The produced binary is `build/livetikz`.
The binary can also be installed for system-wide usage using 
```
sudo make install
```

# Screenshots

