#!/bin/sh

# create a SVN repository
rm -rf /tmp/svn_repository
svnadmin create /tmp/svn_repository

# import test data
mkdir -p /tmp/import
echo "Line 1" > /tmp/import/a.txt
cd /tmp
svn import -m "Initial import" import file:///tmp/svn_repository/wc_svn

# checkout working copy
rm -rf /tmp/wc_svn
svn checkout file:///tmp/svn_repository/wc_svn
rm -rf /tmp/import
cd wc_svn

# changed file
echo "Line 1" > b.txt
svn add b.txt
svn commit -m '' b.txt
echo "Line 2" >> b.txt

# new file
echo "Line 1" > c.txt
