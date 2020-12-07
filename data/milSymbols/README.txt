This directory contains images of military symbols that will be available to the Fusion3D viewer.

There are 2 basically similar standards for military icons -- 2525B/C which is for the US and NATO APP6B.
They are heirarchical and can define procedurally a large number of different icons based on different attributes.
They span the entire universe of military things, so they tend to be pretty broad categories.

We have not been able to find a c++ library that creates icons and also this would require a lot of programming to
make a rather complex menu to construct the icons.
So we create icons external to Fusion3D and use the images in the code.

We have used the open-source code Microdem to generate the military symbols.
It uses basic symbols from MAPSYMBS.COM in the form of 'fonts'
so these fonts have to be loaded also or all the useful symbols dont appear.
You access the icon composer by running Microdem, clicking the 'file' menu, the 'tools' submenu and the 'Military icon generator'.
You should see 5 tabs, starting with the 'FR land'. 
If you only see 2 starting with 'DRMS' then the MAPSYMBS.COM symbols have not been recognized.
You can save files as BMP, JPEG, GIF, and PNG.

Fusion3D looks for military symbols in a subdirectory of its bin directory: bin/milSymbols.
It requires a BMP image file for each symbol.
One should specify a standard size in Microdem when saving symbols, but the symbols are typically different sizes,
most larger than the specified size.

Fusion3D has a menu that shows all the military symbols in directory bin/milSymbols.
One can choosed the desired symbol from this menu.
The menu can accomodata over 100 symbols, but is fixed size.  If there are too many symbols, some symbols may not appear.



