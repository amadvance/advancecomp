Name
	advcomp - History For AdvanceCOMP

AdvanceCOMP Version 1.10 2004/04
	) Added support for alpha channel and the new -n option
		at advmng.
	) Fixed the uncompressing error "Invalid compressed data in ..."
		with some GZIP files [Christian Lestrade].

AdvanceCOMP Version 1.9 2003/11
	) Added support for .tgz files at advdef.
	) Added the -a option at advmng to create .mng files from
		a sequence of .png files.

AdvanceCOMP Version 1.8 2003/10
	) Added support for .gz files at advdef.
	) Fixed support for .png files with splitted IDATs.

AdvanceCOMP Version 1.7 2003/08
	) Fixed a Segmentation Fault bug on the advmng -r option on
		the latest gcc.
	) Better and faster (MMX) move recognition in the advmng scroll
		compression.
	) The frame reduction of the advmng utility is now done only if possible.
		The compression process never fails.
	) Added a new -S (--scroll-square) option at advmng.
	) Added a new -v (--verbose) option at advmng to show the
		compression status.
	) Changed the internal ID for the bzip2 and LZMA compression.
		The bzip2 ID is now compatible with the PKWARE specification.
	) Added support for RGB images with alpha channel at the advpng utility.
	) Updated with automake 1.7.6.

AdvanceCOMP Version 1.6 2003/05
	) Added the `-x' option at the advmng utility to export .png files
		from a .mng clip. Useful to compress it in an MPEG file.
	) Fixed the support for zips with additional data descriptors.
	) Updated with autoconf 2.57 and automake 1.7.4.
	) Some fixes for the gcc 3.3 compiler.

AdvanceCOMP Version 1.5 2003/01
	) Splitted from AdvanceSCAN
	) Added the `advdef' compression utility.

AdvanceSCAN Version 1.4 2002/12
	) Fixed a bug in the advmng utility when it was called with
		more than one file in the command line. The program
		was incorrectly adding a PLTE chunk at rgb images.

AdvanceSCAN Version 1.3 2002/11
	) Added the support for the transparency tRNS chunk at the
		advpng utility.
	) Upgraded at the latest Advance Library.
	) Fixes at the docs. [by Filipe Estima]
	) Minor changes at the autoconf/automake scripts.

AdvanceSCAN Version 1.2 2002/08
	) Added the advpng utility to compress the PNG files.
	) Added the advmng utility to compress the MNG files.
	) Added a Windows version.
	) Other minor fixes.

AdvanceSCAN Version 1.1 2002/06
	) Fixed an infinite loop bug testing some small damaged zips.
	) Removed some warning compiling with gcc 3.1.

AdvanceSCAN Version 1.0 2002/05
	) First public release.
	) Fixed the compression percentage computation on big files.
	) Added the --pedantic option at the advzip utility. These
		tests are only done if requested.

AdvanceSCAN Version 0.6-beta 2002/05
	) Added the AdvanceZIP utility.
