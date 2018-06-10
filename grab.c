/*
 * grab - get and set PNG grAb offsets
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This software or document includes material copied from or derived from
 * PNG (Portable Network Graphics) Specification Version 1.0 
 * (https://www.w3.org/TR/PNG-CRCAppendix.html).
 * Copyright © 1996 W3C® (MIT, ERCIM, Keio, Beihang).
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define INVALID_OFFSET (-99999)
#define MIN_OFFSET (-9999)
#define MAX_OFFSET (9999)

int32_t swapbytes( int32_t swap );

void make_crc_table(void);
unsigned long update_crc(unsigned long crc, unsigned char *buf, int len);
unsigned long crc(unsigned char *buf, int len);

int main(int argc, char** argv) {
    if( argc < 2 ) {
        printf("Usage: %s file.png [new_x] [new_y]\n",argv[0]);
        return 1;
    }

    FILE* f = fopen(argv[1], "r+");
    int32_t new_x = INVALID_OFFSET;
    int32_t new_y = INVALID_OFFSET;

    if( argc >= 3 ) new_x = atoi(argv[2]);
    if( argc >= 4 ) new_y = atoi(argv[3]);

    if( f == NULL || feof(f) ) {
        printf("Could not open file (%s).\n", argv[1]);
        return 1;
    }

    if( new_x != INVALID_OFFSET && (new_x < MIN_OFFSET || new_x > MAX_OFFSET) ) {
        printf("Invalid offset. X offset was %d, (min: %d, max: %d).\n",new_x,MIN_OFFSET,MAX_OFFSET);
        return 1;
    }

    if( new_y != INVALID_OFFSET && (new_y < MIN_OFFSET || new_y > MAX_OFFSET) ) {
        printf("Invalid offset. Y offset was %d, (min: %d, max: %d).\n",new_y,MIN_OFFSET,MAX_OFFSET);
        return 1;
    }

    /* find grAb marker in the file and stop */
    char searchBytes[] = { 'g', 'r', 'A', 'b' };
    unsigned int searchPos = 0;
    unsigned int searchLen = 4;

    char found = 0;
    char c;
    while( (c = fgetc(f)) != EOF ) {
        if( c == searchBytes[searchPos] ) {
            searchPos++;
            if( searchPos >= searchLen ) {
                found = 1;
                break;
            }
        } else {
            searchPos = 0;
        }
    }

    if( !found ) {
        printf("No grAb found.\n");
        return 0;
    }

    /* grAb found, get offsets */
    int32_t grab[2];
    fread(&grab, sizeof(int32_t), 2, f);
    grab[0] = swapbytes(grab[0]);
    grab[1] = swapbytes(grab[1]);
    printf("%d, %d\n",grab[0],grab[1]);


    /* if a new offset is provided, use it */
    if( new_x != INVALID_OFFSET && new_y != INVALID_OFFSET ) {


        /* rewind back to the beginning of the block */
        fseek( f, -sizeof(int32_t)*2, SEEK_CUR );
        printf("Writing X: %d, Y: %d\n",new_x,new_y);
        grab[0] = swapbytes(new_x);
        grab[1] = swapbytes(new_y);
        int fwrote = fwrite( &grab, sizeof(int32_t), 2, f);

        uint8_t bytes[] = {
            'g',
            'r',
            'A',
            'b',
            ((uint8_t*) grab)[0],
            ((uint8_t*) grab)[1],
            ((uint8_t*) grab)[2],
            ((uint8_t*) grab)[3],
            ((uint8_t*) grab)[4],
            ((uint8_t*) grab)[5],
            ((uint8_t*) grab)[6],
            ((uint8_t*) grab)[7],
        };

        uint32_t crcbytes = swapbytes(crc(bytes,12));
        fwrote += fwrite( &crcbytes, sizeof(uint32_t), 1, f );
        /* printf("Wrote: %d\n", fwrote); */
        fseek( f, -sizeof(int32_t)*fwrote, SEEK_CUR );
        fread(&grab, sizeof(int32_t), 2, f);
        grab[0] = swapbytes(grab[0]);
        grab[1] = swapbytes(grab[1]);
        printf("New X: %d, Y: %d\n",grab[0],grab[1]);
    }

    fclose(f);
}

int32_t swapbytes( int32_t swap ) {
    return ( ((swap>>24) & 0xff) |
             ((swap>> 8) & 0xff00) |
             ((swap<< 8) & 0xff0000) |
             ((swap<<24) & 0xff000000)
            );
}


/* The CRC Code below is the Sample CRC Code from the
 * PNG (Portable Network Graphics) Specification Version 1.0; see below.
 *
 * https://www.w3.org/TR/PNG-CRCAppendix.html
 *
* "This software or document includes material copied from or derived from
* PNG (Portable Network Graphics) Specification Version 1.0 
* (https://www.w3.org/TR/PNG-CRCAppendix.html).
* Copyright © 1996 W3C® (MIT, ERCIM, Keio,
* Beihang)."
*/

/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void)
{
    unsigned long c;
    int n, k;

    for (n = 0; n < 256; n++) {
        c = (unsigned long) n;
        for (k = 0; k < 8; k++) {
            if (c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc_table[n] = c;
    }
    crc_table_computed = 1;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
   should be initialized to all 1's, and the transmitted value
   is the 1's complement of the final running CRC (see the
   crc() routine below)). */

unsigned long update_crc(unsigned long crc, unsigned char *buf, int len)
{
    unsigned long c = crc;
    int n;

    if (!crc_table_computed)
        make_crc_table();
    for (n = 0; n < len; n++) {
        c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long crc(unsigned char *buf, int len)
{
    return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}
