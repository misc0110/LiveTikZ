#!/bin/bash
if test `whoami` != "root"; then echo "You need to run this target using fakeroot: fakeroot -u make deb"; exit 1; fi
mkdir -pv livetikz/usr/bin
mkdir -pv livetikz/usr/share/doc/livetikz/
cp build/livetikz livetikz/usr/bin
strip livetikz/usr/bin/livetikz
mkdir -p livetikz/DEBIAN
sed "s/%VERSION%/$(cat VERSION)/" docs/debian-control > livetikz/DEBIAN/control
echo "initial version" > livetikz/usr/share/doc/livetikz/changelog
echo "Copyright 2020, Michael Schwarz" > livetikz/usr/share/doc/livetikz/copyright
gzip -c -9 -n livetikz/usr/share/doc/livetikz/changelog > livetikz/usr/share/doc/livetikz/changelog.gz
rm livetikz/usr/share/doc/livetikz/changelog
chmod -R 0755 livetikz/usr
chmod 0644 livetikz/usr/share/doc/livetikz/copyright
chmod 0644 livetikz/usr/share/doc/livetikz/changelog.gz
chown -R root:root livetikz/
dpkg-deb --build livetikz
rm -rf livetikz
lintian livetikz.deb 
mv livetikz.deb dist/livetikz_$(cat VERSION)_amd64.deb
