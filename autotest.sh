#!/bin/sh

# The MSYS_NO_PATHCONV doesn't work at all in my MinGW so I figured out 
# another workaround. Since this problem only happen on, .e.g,
#   -s/domain/field/eig
# which turned out by Posix path conversion to be something like
#   -s/C:\MinGW\msys\1.0\home\user\domain/field/eig
# The harmless approach is topping every "-s" option with 'v', .i.e "-vs/..."

export MSYS_NO_PATHCONV=1

success=0
failure=0

ATROOT="testing scene"
RENAME="renamex -v"

verify()
{
  if [ -f "$1" ]; then
    echo successful
    success=`expr $success + 1`
  elif [ -d "$1" ]; then
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

  echo > "$ATROOT/Desktop/Screenshot at 2016-06-15 12-58-08.png"
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
  #echo > "$ATROOT/Musics/平原綾香 ふたたび 千と千尋.mp4"
  #echo > "$ATROOT/Musics/（小苹果）- 健身舞蹈教学版.mp4"
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
$RENAME -vs/1600/---- "$ATROOT/Documents/Datasheets/ST3160023AS.pdf"
verify "$ATROOT/Documents/Datasheets/ST3----23AS.pdf"

echo "[Test010]: search and replace with specified occurances"
make_scenario
$RENAME -vs/./^/3 "$ATROOT/Downloads/installation/VirtualBox-4.3-4.3.20_96996_el7-1.x86_64.rpm"
verify "$ATROOT/Downloads/installation/VirtualBox-4^3-4^3^20_96996_el7-1.x86_64.rpm"

echo "[Test011]: search and replace with specified occurances and ignore cases"
make_scenario
$RENAME -vs/E/#/4i "$ATROOT/Downloads/installation/FreeBSD-10.1-RELEASE-amd64-bootonly.iso"
verify "$ATROOT/Downloads/installation/Fr##BSD-10.1-R#L#ASE-amd64-bootonly.iso"

echo "[Test012]: backward search and replace once and ignore cases"
make_scenario
$RENAME -vs/i/1/bi "$ATROOT/Downloads/mushroom_picking.pdf"
verify "$ATROOT/Downloads/mushroom_pick1ng.pdf"

echo "[Test013]: backward search and replace some occurance and ignore cases"
make_scenario
$RENAME -vs/e/-E-/bi4 "$ATROOT/Downloads/installation/FreeBSD-10.1-RELEASE-amd64-bootonly.iso"
verify "$ATROOT/Downloads/installation/Fre-E-BSD-10.1-R-E-L-E-AS-E--amd64-bootonly.iso"

echo "[Test014]: changing extension name"
make_scenario
$RENAME -vs/.pdf/.ps/e "$ATROOT/Documents/doxygen_manual-1.8.7.pdf"
verify "$ATROOT/Documents/doxygen_manual-1.8.7.ps"

echo "[Test015]: changing tail part of extension name"
make_scenario
$RENAME -vs/domain/field/eig "$ATROOT/Desktop/kde/cache-localhost.localdomain"
verify "$ATROOT/Desktop/kde/cache-localhost.localfield"

echo "[Test016]: the regular expression testing"
make_scenario
$RENAME -vs/.o//rg "$ATROOT/Desktop/kde/cache-localhost.localdomain"
verify "$ATROOT/Desktop/kde/cache-calst.calmain"
$RENAME -vs/^[A-Z].*cal/heatpump/r "$ATROOT/Documents/LibreCAD/Electrical1-LCAD.zip"
verify "$ATROOT/Documents/LibreCAD/heatpump1-LCAD.zip"
$RENAME -vs/^[A-Z].+[0-9]/builder/xg "$ATROOT/Documents/LibreCAD/Architect3-LCAD.zip"
verify "$ATROOT/Documents/LibreCAD/builder-LCAD.zip"
$RENAME -vs/e.*e/-/rig "$ATROOT/Downloads/installation/FreeBSD-10.1-RELEASE-amd64-bootonly.iso"
verify "$ATROOT/Downloads/installation/Fr--amd64-bootonly.iso"

echo "[Test017]: Test the space inside the pattern"
make_scenario
$RENAME -vs"/lin con/hello/1" "$ATROOT/Musics/Mozart/Concerto/violin concerto nº 3.mp4"
verify "$ATROOT/Musics/Mozart/Concerto/viohellocerto nº 3.mp4"

echo "[Test018]: Test the recursive operation"
echo Note the difference between \"$ATROOT/Documents/\" and \"$ATROOT/Documents\"
echo The former won\'t change the \"Documents\"
make_scenario
$RENAME -lR "$ATROOT/Documents/"
verify "$ATROOT/Documents/stb release notes .pdf"
verify "$ATROOT/Documents/train_timetable.txt"
verify "$ATROOT/Documents/Datasheets/st3160023as.pdf"
verify "$ATROOT/Documents/Datasheets/seagatesmartexternal 04 24 06.rtf"
verify "$ATROOT/Documents/Datasheets/signal integrity design guidelines.pdf"
verify "$ATROOT/Documents/LibreCAD/architect3-lcad.zip"
verify "$ATROOT/Documents/LibreCAD/electrical1-lcad.zip"

echo "[Test020]: rename a directory"
make_scenario
$RENAME -vs/ents/ains/1 "$ATROOT/Documents"
verify "$ATROOT/Documains"

echo "[Test021]: rename from a file"
make_scenario
find "$ATROOT" -name *.mp4 > "$ATROOT/filename.lst"
$RENAME -vs/mp4/utube/e -f "$ATROOT/filename.lst"
verify "$ATROOT/Musics/Mozart/Concerto/violin concerto nº 3.utube"
verify "$ATROOT/Musics/Mozart/Concerto/Concerto for Flute Harp and Orchestra in C major K299.utube"
verify "$ATROOT/Musics/Niccolò Paganini/Caprice/Caprice no. 5.utube"
verify "$ATROOT/Musics/Niccolò Paganini/Caprice/Caprice no. 24 - with piano accompaniment.utube"
verify "$ATROOT/Musics/Niccolò Paganini/Concerto/La Campanella.utube"
verify "$ATROOT/Westlife - You Raise Me Up.utube"

echo "Total $success successful and $failure failed"
clean_scenario
exit $failure

