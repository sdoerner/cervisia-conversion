#!/bin/sh

# create a CVS repository
rm -rf /tmp/cvs_repository
cvs -d /tmp/cvs_repository init

# import test data
mkdir -p /tmp/import
echo "Line 1" > /tmp/import/a.txt
cd /tmp/import
cvs -d /tmp/cvs_repository import -m "Initial import" wc TEST START

# checkout working copy
rm -rf /tmp/wc
cd ..
cvs -d /tmp/cvs_repository checkout wc
rm -rf /tmp/import
cd wc

# changed file
echo "Line 1" > b.txt
cvs -d /tmp/cvs_repository add b.txt
cvs -d /tmp/cvs_repository commit -m '' b.txt
echo "Line 2" >> b.txt

# new file
echo "Line 1" > c.txt
