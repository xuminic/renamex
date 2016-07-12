#!/bin/sh

success=0
failure=0

verify()
{
  if [ -f "$1" ]; then
    echo successful
    success=`expr $success + 1`
  else
    echo failed
    failure=`expr $failure + 1`
  fi
}

echo "[Test001]"
echo Testing direct rename mode with quoted file names
echo > "Plan your trip.txt"
renamex -v "Plan your trip.txt" "my trip.txt"
verify "my trip.txt"
rm -f "my trip.txt" "Plan your trip.txt"

echo "[Test002]"
echo Testing moving file by renaming
echo > "Plan your trip.txt"
mkdir "trip per"
renamex -v "Plan your trip.txt" "trip per"
verify "trip per/Plan your trip.txt"
rm -rf "trip per"

echo "[Test003]"
echo Testing renaming symbol link as common file
echo > /tmp/myrenametest.txt
ln -s /tmp/myrenametest.txt locallink
renamex -v locallink quicklink
verify quicklink
rm -f quicklink /tmp/myrenametest.txt

echo "[Test004]"
echo Testing symbol link as directory
echo > myrenametest.txt
mkdir dir1
mkdir dir1/dir2
ln -s dir1/dir2 locallink
renamex -v myrenametest.txt locallink
verify locallink/myrenametest.txt
rm -rf dir1 locallink

echo "[Test005]"
echo Testing Uppercase a file name
mkdir dir1
mkdir dir1/dir2
echo > dir1/dir2/MyRenameTest.txt
renamex -v -u dir1/dir2/MyRenameTest.txt
verify dir1/dir2/MYRENAMETEST.TXT
rm -rf dir1

echo "[Test006]"
echo Testing Lowercase a file name
mkdir dir1
mkdir dir1/dir2
echo > dir1/dir2/MyRenameTest.txt
renamex -v -l dir1/dir2/MyRenameTest.txt
verify dir1/dir2/myrenametest.txt
rm -rf dir1

echo "[Test007]"
echo Inserting Prefix and suffix to a file name
mkdir dir1
mkdir dir1/dir2
echo > dir1/dir2/MyRenameTest.txt
renamex -v -p This -x Shit dir1/dir2/MyRenameTest.txt
verify dir1/dir2/ThisMyRenameTestShit.txt
rm -rf dir1

echo "[Test008]"
echo Make sure uppercase/lowercase not involving insertion
mkdir dir1
mkdir dir1/dir2
echo > dir1/dir2/MyRenameTest
renamex -v -l -p This -x Shit dir1/dir2/MyRenameTest
verify dir1/dir2/ThismyrenametestShit
rm -rf dir1

echo "[Test009]"
echo Simple search and replace method
mkdir dir1
mkdir dir1/rename
echo > dir1/rename/MyRenameNamenameTest
renamex -v -s/name/abc dir1/rename/MyRenameNamenameTest
verify dir1/rename/MyReabcNamenameTest
rm -rf dir1

echo "[Test010]"
echo Search and replace with specified occurances
mkdir dir1
mkdir dir1/rename
echo > dir1/rename/MyRenameNamenameTest
renamex -v -s/name/abc/3 dir1/rename/MyRenameNamenameTest
verify dir1/rename/MyReabcNameabcTest
rm -rf dir1

echo "[Test011]"
echo Search and replace with specified occurances and ignore cases
mkdir dir1
mkdir dir1/rename
echo > dir1/rename/MyRenameNamenameTest
renamex -v -s/name/abc/3i dir1/rename/MyRenameNamenameTest
verify dir1/rename/MyReabcabcabcTest
rm -rf dir1

echo "[Test012]"
echo Backward search and replace once and ignore cases
mkdir dir1
mkdir dir1/rename
echo > dir1/rename/MyRenameNameNameTest
renamex -v -s/name/abc/bi dir1/rename/MyRenameNameNameTest
verify dir1/rename/MyRenameNameabcTest
rm -rf dir1

echo "[Test013]"
echo Backward search and replace all occurance and ignore cases
mkdir dir1
mkdir dir1/rename
echo > dir1/rename/MyRenameNameNameTest
renamex -v -s/name/abc/big dir1/rename/MyRenameNameNameTest
verify dir1/rename/MyReabcabcabcTest
rm -rf dir1

echo "[Test014]"
echo Change extension name
mkdir dir1
mkdir dir1/rename
echo > dir1/rename/MyRenameName.extName
renamex -v -s/.extName/.txt/e dir1/rename/MyRenameName.extName
verify dir1/rename/MyRenameName.txt
rm -rf dir1

echo "[Test015]"
echo Change part of extension name
mkdir dir1
mkdir dir1/rename
echo > dir1/rename/MyRenameName.extName
renamex -v -s/name/abc/eig dir1/rename/MyRenameName.extName
verify dir1/rename/MyRenameName.extabc
rm -rf dir1

echo "[Test016]"
echo Test the regular expression
mkdir dir1
mkdir dir1/rename
echo > dir1/rename/MyRenameName.extName
echo > dir1/rename/myRenameName.extName
renamex -v -s/^[A-Z].*name/abc/rg dir1/rename/MyRenameName.extName
renamex -v -s/^[A-Z].*name/abc/rg dir1/rename/myRenameName.extName
verify dir1/rename/abcName.extName
verify dir1/rename/myRenameName.extName
rm -rf dir1

echo "[Test017]"
echo Test the extended regular expression
mkdir dir1
mkdir dir1/rename
echo > dir1/rename/MyRenameName.extName
renamex -v -s/[A-Z].+name/abc/xg dir1/rename/MyRenameName.extName
verify dir1/rename/abcName.extName
rm -rf dir1

echo "[Test018]"
echo Test the recursive operation
mkdir dir1 dir1/rename dir1/paganini dir1/mozart
mkdir dir1/paganini/Caprice dir1/mozart/concerto
echo > dir1/rename/MyRenameName.extName
echo > dir1/paganini/DevilMusic.list
echo > dir1/paganini/Caprice/No.24.mlst
echo > dir1/paganini/Caprice/No.5.mlst
echo > dir1/mozart/Poison.list
echo > dir1/mozart/concerto/flute_and_harp.mlst
renamex -v -lR dir1
verify dir1/rename/myrenamename.extname
verify dir1/paganini/devilmusic.list
verify dir1/paganini/Caprice/no.24.mlst
verify dir1/paganini/Caprice/no.5.mlst
verify dir1/mozart/poison.list
rm -rf dir1

echo "[Test019]"
echo Test the space inside the pattern
mkdir dir1
mkdir dir1/rename
echo > dir1/rename/"My Rename Name.ext Name"
renamex -v -s"/me Name/abc/1" dir1/rename/*
verify "dir1/rename/My Renaabc.ext Name"
rm -rf dir1

echo "Total $success successful and $failure failed"
