#!/bin/bash

#echo "[$*]"
#exit 0

QUIET=0

if [ ! "$1" ]; then dxf2mvg -hp; exit 0; fi
if [ "$1" == "-h" ]; then dxf2mvg -hp; exit 0; fi
if [ "$1" == "--help" ]; then dxf2mvg -hp; exit 0; fi

TMPF="/tmp/dxf.XXXXXXXX.mvg"
DELETETMPF=1
#TMPF="/tmp/dxf.mvg"

if [ "$1" == "-q" ]; then
  QUIET=1
  shift
fi

if [ "$1" == "-tomvg" ]; then
  shift
  dxf2mvg "$@"
  exit 0
fi

if [ "$1" == "-coordvars" ]; then
  dxf2mvg "$@"
  exit 0
fi

if [ "$1" == "-coordvarsc" ]; then
  dxf2mvg "$@"
  exit 0
fi

if [ "$1" == "-nocomment" ]; then
  COMMENTSTR=""
  shift
else
  COMMENTSTR="Created by: dxf2png.sh $*"
fi

if [ "$1" == "-q" ]; then
  QUIET=1
  shift
fi

if [ "$1" != "-mvg" ]; then
dxf2mvg "$@" > "$TMPF"
OUTF=""
else
TMPF="$2"
outf="$3"
DELETETMPF=0
fi

XYSIZE=`cat "$TMPF"|grep '^##xysize '|cut -d ' ' -f 2`
DPI=`cat "$TMPF"|grep '^##DPI '|cut -d ' ' -f 2`
BGCOL=`cat "$TMPF"|grep '^##BGCOL '|cut -d ' ' -f 2`
if [ "$outf" == "" ]; then
  outf=`cat "$TMPF"|grep '^##outfile '|cut -b 11-`
fi

if [ "$outf" == "" ]; then outf="dxf.png"; fi
if [ "$BGCOL" == "" ]; then BGCOL="white"; fi

if [ "$outf" == "-" ]; then outf="PNG:-"; fi

if [ "$QUIET" == "0" ]; then
  echo "Writing to file '$outf'...";
fi

convert -size $XYSIZE xc:"$BGCOL" -strokewidth 4 -stroke black \
  -draw @"$TMPF" \
  -density "$DPI" -units PixelsPerInch \
  -depth 8 \
  -set comment "$COMMENTSTR" \
$outf

#echo "Comment: $COMMENTSTR"

if [ "$DELETETMPF" == "1" ]; then
rm $TMPF
fi

