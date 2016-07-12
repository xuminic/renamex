#!/bin/sh

success=0
failure=0

ATROOT="testing scene"
RENAME="renamex -v"

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

clean_scenario()
{
  if [ -d "$ATROOT" ]; then
    rm -rf "$ATROOT"
  fi
}

make_scenario()
{
  clean_scenario
  mkdir "$ATROOT"
  echo > "$ATROOT"/"Unsaved Document 1"
  echo > "$ATROOT"/"Cyprinid Weekly Report.txt"
  mkdir  "$ATROOT/Desktop" "$ATROOT/Documents" "$ATROOT/.dropbox" 
  mkdir  "$ATROOT/Downloads" "$ATROOT/Musics"
  echo > "$ATROOT/Foundation_drop2.pdf"
  echo > "$ATROOT/Westlife - You Raise Me Up.mp4"

  echo > "$ATROOT/Desktop/Screenshot at 2016-06-15 12:58:08.png"
  mkdir  "$ATROOT/Desktop/gnome" "$ATROOT/Desktop/kde"
  echo > "$ATROOT/Desktop/MyRenameTest.txt"
  echo > "$ATROOT/Desktop/gnome/backgrounds.xml"
  echo > "$ATROOT/Desktop/gnome/file-roller.xml"
  echo > "$ATROOT/Desktop/kde/cache-localhost.localdomain"
  echo > "$ATROOT/Desktop/kde/socket-localhost.localdomain"

  echo > "$ATROOT/Documents/STB Release Notes .pdf"
  mkdir  "$ATROOT/Documents/Datasheets" "$ATROOT/Documents/LibreCAD"
  echo > "$ATROOT/Documents/doxygen_manual-1.8.7.pdf"
  echo > "$ATROOT/Documents/Train_timetable.txt"
  echo > "$ATROOT/Documents/Datasheets/ST3160023AS.pdf"
  echo > "$ATROOT/Documents/Datasheets/SeagateSMARTExternal 04 24 06.rtf"
  echo > "$ATROOT/Documents/Datasheets/Signal Integrity Design Guidelines.pdf"
  echo > "$ATROOT/Documents/LibreCAD/Architect3-LCAD.zip"
  echo > "$ATROOT/Documents/LibreCAD/Electrical1-LCAD.zip"

  echo > "$ATROOT/.dropbox/command_socket"
  echo > "$ATROOT/.dropbox/dropbox.pid"
  mkdir  "$ATROOT/.dropbox/instance1" "$ATROOT/.dropbox/instance_db"
  echo > "$ATROOT/.dropbox/unlink.db"
  echo > "$ATROOT/.dropbox/instance1/aggregation.dbx"
  echo > "$ATROOT/.dropbox/instance1/notifications.dbx"
  echo > "$ATROOT/.dropbox/instance1/TO_HASH_uwWINR"
  echo > "$ATROOT/.dropbox/instance_db/hostkeys  instance.dbx"

  echo > "$ATROOT/Downloads/FOXTEL Satellite FINAL Stages Rev'AO'.xls" 
  mkdir  "$ATROOT/Downloads/installation" "$ATROOT/Downloads/PDF_GPS"
  echo > "$ATROOT/Downloads/mushroom_picking.pdf"
  echo > "$ATROOT/Downloads/installation/FreeBSD-10.1-RELEASE-amd64-bootonly.iso"
  echo > "$ATROOT/Downloads/installation/VirtualBox-4.3-4.3.20_96996_el7-1.x86_64.rpm"
  echo > "$ATROOT/Downloads/installation/debian-8.1.0-amd64-netinst.iso"
  echo > "$ATROOT/Downloads/PDF_GPS/CH_SHALLOW CROSSING.pdf"
  echo > "$ATROOT/Downloads/PDF_GPS/CH_KOONDROOK.pdf"

  echo > "$ATROOT/Musics/Sample Music.lnk"
  mkdir  "$ATROOT/Musics/Niccolò Paganini" 
  mkdir  "$ATROOT/Musics/Niccolò Paganini/Caprice" "$ATROOT/Musics/Niccolò Paganini/Concerto"
  mkdir  "$ATROOT/Musics/Mozart" "$ATROOT/Musics/Mozart/Concerto"
  echo > "$ATROOT/Musics/平原綾香 ふたたび 千と千尋.mp4"
  echo > "$ATROOT/Musics/（小苹果）- 健身舞蹈教学版.mp4"
  echo > "$ATROOT/Musics/Niccolò Paganini/DevilMusic.m3u"
  echo > "$ATROOT/Musics/Niccolò Paganini/Caprice/Caprice no. 24 - with piano accompaniment.mp4"
  echo > "$ATROOT/Musics/Niccolò Paganini/Caprice/Caprice no. 5.mp4"
  echo > "$ATROOT/Musics/Niccolò Paganini/Concerto/La Campanella.mp4"
  echo > "$ATROOT/Musics/Mozart/playlist.m3u"
  echo > "$ATROOT/Musics/Mozart/Concerto/Concerto for Flute Harp and Orchestra in C major K299.mp4"
  echo > "$ATROOT/Musics/Mozart/Concerto/violin concerto nº 3.mp4"

  ln -s "Musics/Niccolò Paganini/Concerto" "$ATROOT/link_to_pananini"
  ln -s "Downloads/PDF_GPS/CH_KOONDROOK.pdf" "$ATROOT/link_to_koondrook"
}


echo "[Test001]: direct rename mode with quoted file names"
make_scenario
$RENAME "$ATROOT/Westlife - You Raise Me Up.mp4" "$ATROOT/Westlife.mp4"
verify "$ATROOT/Westlife.mp4"

echo "[Test002]: moving file by renaming"
make_scenario
$RENAME "$ATROOT/Westlife - You Raise Me Up.mp4" "$ATROOT/Downloads/PDF_GPS"
verify "$ATROOT/Downloads/PDF_GPS/Westlife - You Raise Me Up.mp4"

echo "[Test003]: renaming symbol link as common file"
make_scenario
$RENAME "$ATROOT/link_to_koondrook" "$ATROOT/map_link"
verify "$ATROOT/map_link"

echo "[Test004]: renaming symbol link a directory"
make_scenario
$RENAME "$ATROOT/link_to_pananini" "$ATROOT/violin_concerto"
verify "$ATROOT/violin_concerto/La Campanella.mp4"

echo "[Test005]: uppercasing file names"
make_scenario
$RENAME -u "$ATROOT/Desktop/kde/cache-localhost.localdomain"
verify "$ATROOT/Desktop/kde/CACHE-LOCALHOST.LOCALDOMAIN"

echo "[Test006]: lowercasing a file name"
make_scenario
$RENAME -l "$ATROOT/Documents/Datasheets/ST3160023AS.pdf"
verify "$ATROOT/Documents/Datasheets/st3160023as.pdf"

echo "[Test007]: inserting prefix and suffix to a file name"
make_scenario
$RENAME -p This -x Happen "$ATROOT/Documents/Datasheets/ST3160023AS.pdf"
verify "$ATROOT/Documents/Datasheets/ThisST3160023ASHappen.pdf"

echo "[Test008]: make sure uppercase/lowercase not involving insertion"
make_scenario
$RENAME -l -p This -x Happen "$ATROOT/Documents/Datasheets/ST3160023AS.pdf"
verify "$ATROOT/Documents/Datasheets/Thisst3160023asHappen.pdf"

echo "[Test009]: simple search and replace"
make_scenario
$RENAME -s/1600/---- "$ATROOT/Documents/Datasheets/ST3160023AS.pdf"
verify "$ATROOT/Documents/Datasheets/ST3----23AS.pdf"

echo "[Test010]: search and replace with specified occurances"
make_scenario
$RENAME -s/./^/3 "$ATROOT/Downloads/installation/VirtualBox-4.3-4.3.20_96996_el7-1.x86_64.rpm"
verify "$ATROOT/Downloads/installation/VirtualBox-4^3-4^3^20_96996_el7-1.x86_64.rpm"

echo "[Test011]: search and replace with specified occurances and ignore cases"
make_scenario
$RENAME -s/E/#/4i "$ATROOT/Downloads/installation/FreeBSD-10.1-RELEASE-amd64-bootonly.iso"
verify "$ATROOT/Downloads/installation/Fr##BSD-10.1-R#L#ASE-amd64-bootonly.iso"

echo "[Test012]: backward search and replace once and ignore cases"
make_scenario
$RENAME -s/i/1/bi "$ATROOT/Downloads/mushroom_picking.pdf"
verify "$ATROOT/Downloads/mushroom_pick1ng.pdf"

echo "[Test013]: backward search and replace some occurance and ignore cases"
make_scenario
$RENAME -s/e/-E-/bi4 "$ATROOT/Downloads/installation/FreeBSD-10.1-RELEASE-amd64-bootonly.iso"
verify "$ATROOT/Downloads/installation/Fre-E-BSD-10.1-R-E-L-E-AS-E--amd64-bootonly.iso"

echo "[Test014]: changing extension name"
make_scenario
$RENAME -s/.pdf/.ps/e "$ATROOT/Documents/doxygen_manual-1.8.7.pdf"
verify "$ATROOT/Documents/doxygen_manual-1.8.7.ps"

#echo "[Test015]: changing part of extension name"
#make_scenario
#$RENAME -ts/local/remote/eig "$ATROOT/Desktop/kde/cache-localhost.localdomain"
#verify "$ATROOT/Desktop/kde/cache-localhost.remotedomain"

echo "[Test016]: the regular expression testing"
make_scenario
$RENAME -s/.?o//r "$ATROOT/Desktop/kde/cache-localhost.localdomain"
verify "$ATROOT/Desktop/kde/cache-calst.calmain"
exit

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
clean_scenario
exit $failure
