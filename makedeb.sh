#!/bin/bash
if test `whoami` != "root"; then echo "You need to run this target using fakeroot: fakeroot -u make deb"; exit 1; fi
mkdir -pv livetikz/usr/bin
mkdir -pv livetikz/usr/share/doc/livetikz/
mkdir -pv livetikz/usr/share/man/man1/
mkdir -pv livetikz/usr/share/applications/
mkdir -pv livetikz/usr/share/icons/hicolor/256x256/apps
mkdir -pv livetikz/usr/share/livetikz
cp build/livetikz livetikz/usr/bin
strip livetikz/usr/bin/livetikz
mkdir -p livetikz/DEBIAN
sed "s/%VERSION%/$(cat VERSION)/" docs/debian-control > livetikz/DEBIAN/control
echo "initial version" > livetikz/usr/share/doc/livetikz/changelog
echo "Copyright 2020, Michael Schwarz" > livetikz/usr/share/doc/livetikz/copyright
gzip -c -9 -n livetikz/usr/share/doc/livetikz/changelog > livetikz/usr/share/doc/livetikz/changelog.gz
gzip -c -9 -n docs/livetikz.1 > livetikz/usr/share/man/man1/livetikz.1.gz
cp data/livetikz.desktop livetikz/usr/share/applications/
cp data/livetikz.png livetikz/usr/share/icons/hicolor/256x256/apps
rm livetikz/usr/share/doc/livetikz/changelog
cp data/templates/* livetikz/usr/share/livetikz/
chmod -R 0755 livetikz/usr
chmod 0644 livetikz/usr/share/doc/livetikz/*
chmod 0644 livetikz/usr/share/man/man1/livetikz.1.gz
chmod 0644 livetikz/usr/share/icons/hicolor/256x256/apps/*
chmod 0644 livetikz/usr/share/applications/*
chmod 0644 livetikz/usr/share/livetikz/*
chown -R root:root livetikz/
dpkg-deb --build livetikz
rm -rf livetikz
lintian livetikz.deb 
mv livetikz.deb dist/livetikz_$(cat VERSION)_amd64.deb
