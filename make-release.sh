#!/bin/bash

if [ -d make-release ] ; then
	echo "make-release directory already exists, aborting..."
	exit 1
fi

svn export . make-release
rm make-release/make-release.sh
make -f Makefile.cvs -C make-release
( cd make-release && ./configure --without-arts )
make -C make-release dist-bzip2
mv -v make-release/*.tar.bz2 .
rm -r make-release

for file in $(ls -1 *.tar.bz2); do md5sum $file > $file.md5; done
