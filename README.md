## pcf2bmfont

A command line tool to generate bitmap font (as a comply format to what [BMFont](http://www.angelcode.com/products/bmfont/) outputs) from given PCF file.

## Usage

pcf2bmfont [-W width] [-H height] [-n atlas_file] [-x xml_file] [-C] -i char_selection_file pcf_file

* -W, -H : Control the dimensions of the output atlas (as PNG)
* -x, -n : Specify the filename of the output XML atlas description file and the bitmap
* -C : Translate unicde to multi-bytes based on the Active Code Page of current OS before querying to PCF for glyph.
* -i : A text file in UTF-8 listing all needed chars. [Required]

Example:
> pcf2bmfont -W 1024 -H 1024 -n atlas.png -x myfont.fnt -C -i chars.txt some_cool_font.pcf

## About PCF Parser

The source code of the PCF parser used in this command-line tool can work out of the project -- just take the 'PCFFont.h' and 'PCFFont.cpp' out and add them in your project.

Some useful document for parsing PCF:

https://fontforge.github.io/en-US/documentation/reference/pcf-format/

## Credits

* [bmfm](https://gitlab.com/matthklo/bmfm): Sister project. Deal with bitmap font generation.
* [zlib](https://zlib.net/) and [libpng](https://sourceforge.net/projects/libpng/): To read/write PNGs
* [rapidxml](https://sourceforge.net/projects/rapidxml/files/latest/download): To parse and generate the XML atlas description files
* [RectangleBinPack](https://github.com/juj/RectangleBinPack): A versatile rectangle packing algorithm with many variants.
* [xgetopt](https://github.com/matthklo/xgetopt): A cross-platform implementation of getopt().

## License

zlib
