## Process this file with automake to produce Makefile.in

bin_PROGRAMS = advzip advpng advmng advdef

advzip_SOURCES = \
	rezip.cc \
	zip.cc \
	file.cc \
	data.cc \
	siglock.cc \
	compress.cc \
	zipsh.cc \
	getopt.c \
	snprintf.c \
	7z/7zdeflate.cc \
	7z/7zlzma.cc \
	7z/AriBitCoder.cc \
	7z/CRC.cc \
	7z/DeflateDecoder.cc \
	7z/DeflateEncoder.cc \
	7z/HuffmanEncoder.cc \
	7z/IInOutStreams.cc \
	7z/InByte.cc \
	7z/LSBFDecoder.cc \
	7z/LSBFEncoder.cc \
	7z/LZMA.cc \
	7z/LZMADecoder.cc \
	7z/LZMAEncoder.cc \
	7z/LenCoder.cc \
	7z/LiteralCoder.cc \
	7z/OutByte.cc \
	7z/WindowIn.cc \
	7z/WindowOut.cc

advpng_SOURCES = \
	repng.cc \
	pngex.cc \
	file.cc \
	data.cc \
	siglock.cc \
	compress.cc \
	getopt.c \
	snprintf.c \
	lib/fz.c \
	lib/png.c \
	lib/error.c \
	lib/snstring.c \
	7z/7zdeflate.cc \
	7z/7zlzma.cc \
	7z/AriBitCoder.cc \
	7z/CRC.cc \
	7z/DeflateDecoder.cc \
	7z/DeflateEncoder.cc \
	7z/HuffmanEncoder.cc \
	7z/IInOutStreams.cc \
	7z/InByte.cc \
	7z/LSBFDecoder.cc \
	7z/LSBFEncoder.cc \
	7z/LZMA.cc \
	7z/LZMADecoder.cc \
	7z/LZMAEncoder.cc \
	7z/LenCoder.cc \
	7z/LiteralCoder.cc \
	7z/OutByte.cc \
	7z/WindowIn.cc \
	7z/WindowOut.cc

advmng_SOURCES = \
	remng.cc \
	pngex.cc \
	mngex.cc \
	scroll.cc \
	file.cc \
	data.cc \
	siglock.cc \
	compress.cc \
	getopt.c \
	snprintf.c \
	lib/fz.c \
	lib/png.c \
	lib/mng.c \
	lib/error.c \
	lib/snstring.c \
	7z/7zdeflate.cc \
	7z/7zlzma.cc \
	7z/AriBitCoder.cc \
	7z/CRC.cc \
	7z/DeflateDecoder.cc \
	7z/DeflateEncoder.cc \
	7z/HuffmanEncoder.cc \
	7z/IInOutStreams.cc \
	7z/InByte.cc \
	7z/LSBFDecoder.cc \
	7z/LSBFEncoder.cc \
	7z/LZMA.cc \
	7z/LZMADecoder.cc \
	7z/LZMAEncoder.cc \
	7z/LenCoder.cc \
	7z/LiteralCoder.cc \
	7z/OutByte.cc \
	7z/WindowIn.cc \
	7z/WindowOut.cc

advdef_SOURCES = \
	redef.cc \
	file.cc \
	data.cc \
	siglock.cc \
	compress.cc \
	getopt.c \
	snprintf.c \
	pngex.cc \
	lib/fz.c \
	lib/error.c \
	lib/snstring.c \
	lib/png.c \
	lib/mng.c \
	7z/7zdeflate.cc \
	7z/7zlzma.cc \
	7z/AriBitCoder.cc \
	7z/CRC.cc \
	7z/DeflateDecoder.cc \
	7z/DeflateEncoder.cc \
	7z/HuffmanEncoder.cc \
	7z/IInOutStreams.cc \
	7z/InByte.cc \
	7z/LSBFDecoder.cc \
	7z/LSBFEncoder.cc \
	7z/LZMA.cc \
	7z/LZMADecoder.cc \
	7z/LZMAEncoder.cc \
	7z/LenCoder.cc \
	7z/LiteralCoder.cc \
	7z/OutByte.cc \
	7z/WindowIn.cc \
	7z/WindowOut.cc

EXTRA_DIST = \
	README AUTHORS HISTORY INSTALL COPYING \
	doc/advdef.d doc/advzip.d doc/advpng.d doc/advmng.d doc/history.d doc/readme.d doc/authors.d doc/install.d \
	doc/advdef.1 doc/advzip.1 doc/advpng.1 doc/advmng.1 doc/history.1 doc/readme.1 doc/authors.1 doc/install.1 \
	doc/advdef.txt doc/advzip.txt doc/advpng.txt doc/advmng.txt doc/history.txt doc/readme.txt doc/authors.txt doc/install.txt \
	doc/advdef.html doc/advzip.html doc/advpng.html doc/advmng.html doc/history.html doc/readme.html doc/authors.html doc/install.html \
	autogen.sh noautomake.sh \
	7z/README \
	test/test.lst \
	test/mappy.png \
	test/mappy.mng \
	test/basn2c08.png \
	test/basn3p01.png \
	test/basn3p02.png \
	test/basn3p04.png \
	test/basn3p08.png \
	test/basn6a08.png \
	test/basn6a04.png

noinst_HEADERS = \
	snprintf.c \
	pngex.h \
	mngex.h \
	scroll.h \
	compress.h \
	file.h \
	data.h \
	zip.h \
	except.h \
	siglock.h \
	portable.h \
	lib/png.h \
	lib/mng.h \
	lib/fz.h \
	lib/endianrw.h \
	lib/error.h \
	lib/snstring.h \
	lib/extra.h \
	7z/7z.h \
	7z/AriBitCoder.h \
	7z/AriConst.h \
	7z/AriPrice.h \
	7z/BinTree.h \
	7z/BinTree2.h \
	7z/BinTree2Main.h \
	7z/BinTree3.h \
	7z/BinTree3Main.h \
	7z/BinTree3Z.h \
	7z/BinTree3ZMain.h \
	7z/BinTree4.h \
	7z/BinTree4Main.h \
	7z/BinTree4b.h \
	7z/BinTree4bMain.h \
	7z/BinTreeMF.h \
	7z/BinTreeMFMain.h \
	7z/BinTreeMain.h \
	7z/BitTreeCoder.h \
	7z/CRC.h \
	7z/Const.h \
	7z/DeflateDecoder.h \
	7z/DeflateEncoder.h \
	7z/HuffmanDecoder.h \
	7z/HuffmanEncoder.h \
	7z/IInOutStreams.h \
	7z/InByte.h \
	7z/LSBFDecoder.h \
	7z/LSBFEncoder.h \
	7z/LZMA.h \
	7z/LZMADecoder.h \
	7z/LZMAEncoder.h \
	7z/LenCoder.h \
	7z/LiteralCoder.h \
	7z/OutByte.h \
	7z/Portable.h \
	7z/RCDefs.h \
	7z/RangeCoder.h \
	7z/WindowIn.h \
	7z/WindowOut.h

man_MANS = doc/advdef.1 doc/advzip.1 doc/advpng.1 doc/advmng.1

clean-local:
	rm -f check.lst check.zip mappy.mng mappy.png
	rm -f basn2c08.png basn3p01.png basn3p02.png basn3p04.png basn3p08.png basn6a08.png basn6a04.png
	rm -f advdef.exe advzip.exe advpng.exe advmng.exe
	rm -f mappy*.png

maintainer-clean-local:
	rm -f README AUTHORS HISTORY INSTALL doc/copying.txt
	rm -f doc/*.hh

# Docs

%.1 : %.d
	advd2 man < $(srcdir)/$< > $@

%.txt : %.d
	advd2 txt < $(srcdir)/$< | todos > $@

%.html : %.d
	advd2 html < $(srcdir)/$< > $@

%.hh : %.d
	advd2 frame < $(srcdir)/$< > $@

# Archives

README: doc/readme.txt
	cat $< | fromdos > $@

AUTHORS: doc/authors.txt
	cat $< | fromdos > $@

INSTALL: doc/install.txt
	cat $< | fromdos > $@

HISTORY: doc/history.txt
	cat $< | fromdos > $@

doc/copying.txt: COPYING
	cat $< | todos > $@

check-local: advzip test/test.lst
	@./advzip -a -1 check.zip $(srcdir)/COPYING
	@./advzip -L check.zip > check.lst
	@./advzip -t -p check.zip
	@./advzip -a -2 check.zip $(srcdir)/COPYING
	@./advzip -L check.zip >> check.lst
	@./advzip -t -p check.zip
	@./advzip -a -3 check.zip $(srcdir)/COPYING
	@./advzip -L check.zip >> check.lst
	@./advzip -t -p check.zip
	@./advzip -a -4 check.zip $(srcdir)/COPYING
	@./advzip -L check.zip >> check.lst
	@./advzip -t -p check.zip
	@./advzip -a -1 -N check.zip $(srcdir)/COPYING
	@./advzip -L check.zip >> check.lst
	@./advzip -t -p check.zip
	@./advzip -a -2 -N check.zip $(srcdir)/COPYING
	@./advzip -L check.zip >> check.lst
	@./advzip -t -p check.zip
	@./advzip -a -3 -N check.zip $(srcdir)/COPYING
	@./advzip -L check.zip >> check.lst
	@./advzip -t -p check.zip
	@./advzip -a -4 -N check.zip $(srcdir)/COPYING
	@./advzip -L check.zip >> check.lst
	@./advzip -t -p check.zip
	@cp $(srcdir)/test/mappy.mng .
	@./advmng -v -z -1 -f -S 8 mappy.mng
	@./advmng -L mappy.mng >> check.lst
	@./advmng -v -x -1 mappy.mng
	@./advmng -v -a 30 -1 -S 8 mappy.mng mappy-*.png
	@./advmng -L mappy.mng >> check.lst
	@./advmng -v -a 30 -n -1 -S 8 mappy.mng mappy-*.png
	@./advmng -L mappy.mng >> check.lst
	@./advmng -v -a 30 -n -r -1 -S 8 mappy.mng mappy-*.png
	@./advmng -L mappy.mng >> check.lst
	@cp $(srcdir)/test/mappy.png .
	@./advpng -f -z mappy.png
	@./advpng -L mappy.png >> check.lst
	@cp $(srcdir)/test/basn2c08.png $(srcdir)/test/basn3p01.png $(srcdir)/test/basn3p02.png $(srcdir)/test/basn3p04.png $(srcdir)/test/basn3p08.png $(srcdir)/test/basn6a08.png $(srcdir)/test/basn6a04.png .
	@./advpng -f -z basn2c08.png basn3p01.png basn3p02.png basn3p04.png basn3p08.png basn6a08.png basn6a04.png
	@./advpng -L basn2c08.png basn3p01.png basn3p02.png basn3p04.png basn3p08.png basn6a08.png basn6a04.png >> check.lst
	@cmp check.lst $(srcdir)/test/test.lst
	@echo Success!

DISTDOS_ROOT = \
	doc/readme.txt doc/authors.txt doc/history.txt doc/copying.txt \
	advdef.exe advzip.exe advpng.exe advmng.exe

DISTDOS_DOC = \
	doc/advdef.txt doc/advzip.txt doc/advpng.txt doc/advmng.txt \
	doc/readme.txt doc/authors.txt doc/history.txt \
	doc/advdef.html doc/advzip.html doc/advpng.html doc/advmng.html \
	doc/readme.html doc/authors.html doc/history.html 

distdos: $(DISTDOS_ROOT) $(DISTDOS_DOC)
	i586-pc-msdosdjgpp-strip advdef.exe
	i586-pc-msdosdjgpp-strip advzip.exe
	i586-pc-msdosdjgpp-strip advpng.exe
	i586-pc-msdosdjgpp-strip advmng.exe
	rm -f $(PACKAGE)-$(VERSION)-dos-pentium.zip
	mkdir tmp
	cp $(DISTDOS_ROOT) tmp
	mkdir tmp/doc
	cp $(DISTDOS_DOC) tmp/doc
	cd tmp && zip -r ../$(PACKAGE)-$(VERSION)-dos-pentium.zip *
	rm -r tmp

distwindows: $(DISTDOS_ROOT) $(DISTDOS_DOC)
	i386-mingw32msvc-strip advdef.exe
	i386-mingw32msvc-strip advzip.exe
	i386-mingw32msvc-strip advpng.exe
	i386-mingw32msvc-strip advmng.exe
	rm -f $(PACKAGE)-$(VERSION)-windows-pentium.zip
	mkdir tmp
	cp $(DISTDOS_ROOT) tmp
	mkdir tmp/doc
	cp support/zlib.dll tmp
	cp $(DISTDOS_DOC) tmp/doc
	cd tmp && zip -r ../$(PACKAGE)-$(VERSION)-windows-pentium.zip *
	rm -r tmp

DISTWEB = \
	doc/advdef.hh doc/advzip.hh doc/advpng.hh doc/advmng.hh doc/history.hh

web: $(DISTWEB)


