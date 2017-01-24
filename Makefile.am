bin_PROGRAMS = advzip advpng advmng advdef

7z_SOURCES = \
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

libdeflate_SOURCES = \
	libdeflate/adler32.c \
	libdeflate/aligned_malloc.c \
	libdeflate/crc32.c \
	libdeflate/deflate_compress.c \
	libdeflate/deflate_decompress.c \
	libdeflate/gzip_compress.c \
	libdeflate/gzip_decompress.c \
	libdeflate/x86_cpu_features.c \
	libdeflate/zlib_compress.c \
	libdeflate/zlib_decompress.c

zopfli_SOURCES = \
	zopfli/blocksplitter.c \
	zopfli/cache.c \
	zopfli/deflate.c \
	zopfli/gzip_container.c \
	zopfli/hash.c \
	zopfli/katajainen.c \
	zopfli/lz77.c \
	zopfli/squeeze.c \
	zopfli/tree.c \
	zopfli/util.c \
	zopfli/zlib_container.c \
	zopfli/zopfli.h \
	zopfli/zopfli_lib.c

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
	portable.c \
	$(7z_SOURCES) \
	$(libdeflate_SOURCES) \
	$(zopfli_SOURCES)

advpng_SOURCES = \
	repng.cc \
	pngex.cc \
	file.cc \
	data.cc \
	siglock.cc \
	compress.cc \
	getopt.c \
	snprintf.c \
	portable.c \
	lib/fz.c \
	lib/png.c \
	lib/error.c \
	lib/snstring.c \
	$(7z_SOURCES) \
	$(libdeflate_SOURCES) \
	$(zopfli_SOURCES)

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
	portable.c \
	lib/fz.c \
	lib/png.c \
	lib/mng.c \
	lib/error.c \
	lib/snstring.c \
	$(7z_SOURCES) \
	$(libdeflate_SOURCES) \
	$(zopfli_SOURCES)

advdef_SOURCES = \
	redef.cc \
	file.cc \
	data.cc \
	siglock.cc \
	compress.cc \
	getopt.c \
	snprintf.c \
	pngex.cc \
	portable.c \
	lib/fz.c \
	lib/error.c \
	lib/snstring.c \
	lib/png.c \
	lib/mng.c \
	$(7z_SOURCES) \
	$(libdeflate_SOURCES) \
	$(zopfli_SOURCES)

# Add the .version file in the distribution
dist-hook:
	$(srcdir)/autover.sh > $(distdir)/.version

EXTRA_DIST = \
	README AUTHORS HISTORY INSTALL COPYING \
	doc/advdef.d doc/advzip.d doc/advpng.d doc/advmng.d doc/history.d doc/readme.d doc/authors.d doc/install.d \
	doc/advdef.1 doc/advzip.1 doc/advpng.1 doc/advmng.1 doc/history.1 doc/readme.1 doc/authors.1 doc/install.1 \
	doc/advdef.txt doc/advzip.txt doc/advpng.txt doc/advmng.txt doc/history.txt doc/readme.txt doc/authors.txt doc/install.txt \
	autogen.sh autover.sh \
	7z/README \
	libdeflate/README.md \
	libdeflate/COPYING \
	libdeflate/NEWS \
	zopfli/README \
	zopfli/COPYING \
	zopfli/CONTRIBUTORS \
	test/test.lst \
	test/archive.zip \
	test/italy.png \
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
	7z/WindowOut.h \
	libdeflate/adler32_impl.h \
	libdeflate/aligned_malloc.h \
	libdeflate/bt_matchfinder.h \
	libdeflate/common_defs.h \
	libdeflate/compiler_gcc.h \
	libdeflate/crc32_impl.h \
	libdeflate/crc32_table.h \
	libdeflate/decompress_impl.h \
	libdeflate/deflate_compress.h \
	libdeflate/deflate_constants.h \
	libdeflate/gzip_constants.h \
	libdeflate/hc_matchfinder.h \
	libdeflate/lib_common.h \
	libdeflate/libdeflate.h \
	libdeflate/matchfinder_avx2.h \
	libdeflate/matchfinder_common.h \
	libdeflate/matchfinder_neon.h \
	libdeflate/matchfinder_sse2.h \
	libdeflate/unaligned.h \
	libdeflate/x86_cpu_features.h \
	libdeflate/zlib_constants.h \
	zopfli/blocksplitter.h \
	zopfli/cache.h \
	zopfli/deflate.h \
	zopfli/gzip_container.h \
	zopfli/hash.h \
	zopfli/katajainen.h \
	zopfli/lz77.h \
	zopfli/symbols.h \
	zopfli/squeeze.h \
	zopfli/tree.h \
	zopfli/util.h \
	zopfli/zlib_container.h \
	zopfli/zopfli.h

man_MANS = doc/advdef.1 doc/advzip.1 doc/advpng.1 doc/advmng.1

clean-local:
	rm -f check.lst check.zip archive.zip mappy.mng italy.png
	rm -f basn2c08.png basn3p01.png basn3p02.png basn3p04.png basn3p08.png basn6a08.png basn6a04.png
	rm -f advdef.exe advzip.exe advpng.exe advmng.exe
	rm -f mappy*.png

maintainer-clean-local:
	rm -f README AUTHORS HISTORY INSTALL doc/copying.txt
	rm -f doc/*.hh

# Rules for documentation

if HAVE_ADVD2
%.1 : %.d
	advd2 man < $(srcdir)/$< > $@

%.txt : %.d
	advd2 txt < $(srcdir)/$< | todos > $@

%.html : %.d
	advd2 html < $(srcdir)/$< > $@

%.hh : %.d
	advd2 frame < $(srcdir)/$< > $@
endif

# Archives

DTOU = tr -d '\015'

README: doc/readme.txt
	cat $< | $(DTOU) > $@

AUTHORS: doc/authors.txt
	cat $< | $(DTOU) > $@

INSTALL: doc/install.txt
	cat $< | $(DTOU) > $@

HISTORY: doc/history.txt
	cat $< | $(DTOU) > $@

doc/copying.txt: COPYING
	cat $< | todos > $@

# We don't test wth -L the with -3 and -4 because the result is depending on
# the compiler and/or optimization options.
# This is likely a defect of 7z/zopfli implementation.
# For zopfli a possible cause is the use of qsort() (not stable sorting),
# that may be different on different platforms.
check-local: ./advzip$(EXEEXT) test/test.lst
	@cp $(srcdir)/test/archive.zip .
	@echo CRC SIZE > check.lst
	$(TESTENV) ./advzip$(EXEEXT) -z archive.zip
	$(TESTENV) ./advzip$(EXEEXT) -L archive.zip >> check.lst
	$(TESTENV) ./advzip$(EXEEXT) -t -p archive.zip
	$(TESTENV) ./advzip$(EXEEXT) -z -3 archive.zip
	$(TESTENV) ./advzip$(EXEEXT) -t -p archive.zip
	$(TESTENV) ./advzip$(EXEEXT) -z -4 archive.zip
	$(TESTENV) ./advzip$(EXEEXT) -t -p archive.zip
	$(TESTENV) ./advzip$(EXEEXT) -a check.zip $(srcdir)/COPYING
	$(TESTENV) ./advzip$(EXEEXT) -t -p check.zip
	$(TESTENV) ./advzip$(EXEEXT) -L check.zip >> check.lst
	$(TESTENV) ./advzip$(EXEEXT) -a -N check.zip $(srcdir)/COPYING
	$(TESTENV) ./advzip$(EXEEXT) -t -p check.zip
	$(TESTENV) ./advzip$(EXEEXT) -L check.zip >> check.lst
	$(TESTENV) ./advzip$(EXEEXT) -z -3 check.zip
	$(TESTENV) ./advzip$(EXEEXT) -t -p check.zip
	$(TESTENV) ./advzip$(EXEEXT) -z -4 check.zip
	$(TESTENV) ./advzip$(EXEEXT) -t -p check.zip
	@cp $(srcdir)/test/mappy.mng .
	$(TESTENV) ./advmng$(EXEEXT) -v -z -f -S 8 mappy.mng
	$(TESTENV) ./advmng$(EXEEXT) -L mappy.mng >> check.lst
	$(TESTENV) ./advmng$(EXEEXT) -v -x mappy.mng
	$(TESTENV) ./advmng$(EXEEXT) -v -a 30 -S 8 mappy.mng mappy-*.png
	$(TESTENV) ./advmng$(EXEEXT) -L mappy.mng >> check.lst
	$(TESTENV) ./advmng$(EXEEXT) -v -a 30 -n -S 8 mappy.mng mappy-*.png
	$(TESTENV) ./advmng$(EXEEXT) -L mappy.mng >> check.lst
	$(TESTENV) ./advmng$(EXEEXT) -v -a 30 -n -r -S 8 mappy.mng mappy-*.png
	$(TESTENV) ./advmng$(EXEEXT) -L mappy.mng >> check.lst
	@cp $(srcdir)/test/italy.png .
	$(TESTENV) ./advpng$(EXEEXT) -z italy.png
	$(TESTENV) ./advpng$(EXEEXT) -L italy.png >> check.lst
	@cp $(srcdir)/test/basn2c08.png $(srcdir)/test/basn3p01.png $(srcdir)/test/basn3p02.png $(srcdir)/test/basn3p04.png $(srcdir)/test/basn3p08.png $(srcdir)/test/basn6a08.png $(srcdir)/test/basn6a04.png .
	$(TESTENV) ./advpng$(EXEEXT) -f -z basn2c08.png basn3p01.png basn3p02.png basn3p04.png basn3p08.png basn6a08.png basn6a04.png
	$(TESTENV) ./advpng$(EXEEXT) -L basn2c08.png basn3p01.png basn3p02.png basn3p04.png basn3p08.png basn6a08.png basn6a04.png >> check.lst
	cat check.lst | $(DTOU) | cmp $(srcdir)/test/test.lst
	@echo Success!

DISTDOS_ROOT = \
	doc/readme.txt doc/authors.txt doc/history.txt doc/copying.txt \
	advdef.exe advzip.exe advpng.exe advmng.exe

DISTDOS_DOC = \
	doc/advdef.txt doc/advzip.txt doc/advpng.txt doc/advmng.txt \
	doc/readme.txt doc/authors.txt doc/history.txt

distdos: $(DISTDOS_ROOT) $(DISTDOS_DOC)
	rm -f $(PACKAGE)-$(VERSION)-dos-x86.zip
	mkdir tmp
	cp $(DISTDOS_ROOT) tmp
	mkdir tmp/doc
	cp $(DISTDOS_DOC) tmp/doc
	cd tmp && zip -r ../$(PACKAGE)-$(VERSION)-dos-x86.zip *
	rm -r tmp

distwindows-x86: $(DISTDOS_ROOT) $(DISTDOS_DOC)
	rm -f $(PACKAGE)-$(VERSION)-windows-x86.zip
	mkdir tmp
	cp $(DISTDOS_ROOT) tmp
	mkdir tmp/doc
	cp $(DISTDOS_DOC) tmp/doc
	cd tmp && zip -r ../$(PACKAGE)-$(VERSION)-windows-x86.zip *
	rm -r tmp

distwindows-x64: $(DISTDOS_ROOT) $(DISTDOS_DOC)
	rm -f $(PACKAGE)-$(VERSION)-windows-x64.zip
	mkdir tmp
	cp $(DISTDOS_ROOT) tmp
	mkdir tmp/doc
	cp $(DISTDOS_DOC) tmp/doc
	cd tmp && zip -r ../$(PACKAGE)-$(VERSION)-windows-x64.zip *
	rm -r tmp

DISTWEB = \
	doc/advdef.hh doc/advzip.hh doc/advpng.hh doc/advmng.hh doc/history.hh

web: $(DISTWEB)

