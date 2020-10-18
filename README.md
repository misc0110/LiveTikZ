# LiveTikZ
[![Build Status](https://travis-ci.org/misc0110/LiveTikZ.svg?branch=master)](https://travis-ci.org/misc0110/LiveTikZ)

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

LiveTikZ is a Qt application with KDE dependencies for the editor component. LiveTikZ can either be installed using the PPA, [provided Debian packages](https://github.com/misc0110/LiveTikZ/tree/master/dist) or compiled from source. 

## Install using PPA (recommended)

First, add the public key of the PPA and the PPA URL to the package manager, and update the package manager

    curl -s "https://misc0110.github.io/ppa/KEY.gpg" | sudo apt-key add -
    sudo curl -s -o /etc/apt/sources.list.d/misc0110.list "https://misc0110.github.io/ppa/file.list"
    sudo apt update

Then, simply install LiveTikZ

    sudo apt install livetikz



## Prerequisites

The following packages are required to compile from source:

* build-essential
* cmake
* extra-cmake-modules
* qt5-default
* libkf5parts-dev
* libkf5texteditor-dev
* libpoppler-qt5-dev
* libpoppler-cpp-dev
* libpoppler-glib-dev
* kdelibs5-dev
* ktexteditor-katepart
* gettext

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

![LiveTikZ 0.1](https://raw.github.com/misc0110/LiveTikZ/master/screenshots/livetikz_0.1.png)
![LiveTikZ 0.1 - Beamer support](https://raw.github.com/misc0110/LiveTikZ/master/screenshots/livetikz_0.1_multi.png)
