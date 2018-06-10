#!/bin/bash

# Example use of grab which performs automatic mirroring
# of sprites along the horizontal plane.
#
# Assumes ImageMagick is installed.
# Assumes grab is compiled as "grab" in the current directory.

for i in *.png; do
    width=$(magick identify -format "%w" $i)
    grab=$(./grab $i)

    # Horizontally flip image but retain grab chunk
    # Warning: this assumes the grab chunk is the first after the IHDR
    dd if=$i of=grab.chunk bs=1 count=20 skip=0x21
    mogrify -flop $i
    xxd -p $i > p
    xxd -p grab.chunk > g
    sed '2s/.\{6\}/&'$(cat g)'/' p > p2
    xxd -r -p p2 > $i
    rm p g grab.chunk p2

    grab_y=${grab#*,}
    grab_x=${grab%,*}
    new_grab_x=$[ -320+width-grab_x ]
    echo $i,$width,$grab_x,$grab_y -- $new_grab_x
    ./grab $i $new_grab_x $grab_y

# Notes:

# To extract a grab chunk for future use:
# dd if=p.png of=grab.chunk bs=1 count=20 skip=0x21
# Note: The skip offset needs to be adjusted if the grab chunk
# is not the first one after the IHDR.

# Horizontal flipping, if desired:
# mogrify -flop p.png

# To reinsert an extracted grab chunk from a png without one:
# (again, replace the offset with the correct grab offset if desired)
# xxd -p p.png > p
# xxd -p grab.chunk > g
# sed '2s/.\{6\}/&'$(cat g)'/' p > p2
# xxd -r -p p2 > p2.png

done
