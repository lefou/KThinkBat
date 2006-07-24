rm -r make-release
svn export . make-release
make -f Makefile.cvs -C make-release
( cd make-release && ./configure )
make -C make-release dist-bzip2
mv -v make-release/*.tar.bz2 .
rm -r make-release
