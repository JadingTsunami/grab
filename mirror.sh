#!/bin/bash

# Example use of grab which performs automatic mirroring
# of sprites along the horizontal plane.
#
# Assumes ImageMagick is installed.
# Assumes grab is compiled as "grab" in the current directory.

for i in *.PNG; do
    width=$(magick identify -format "%w" $i)
    grab=$(./grab $i)
    grab_y=${grab#*,}
    grab_x=${grab%,*}
    new_grab_x=$[ -320+width-grab_x ]
    echo $i,$width,$grab_x,$grab_y -- $new_grab_x
    ./grab $i $new_grab_x $grab_y
done
