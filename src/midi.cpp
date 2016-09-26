/* subroutines to write standard MIDI file
 * Copyright (C) 1998 Kengo ICHIKI (ichiki@geocities.com)
 * $Id: midi.c,v 1.7 1998/12/29 04:49:07 ichiki Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h> /* fputc(), etc  */
#include <unistd.h> /* write()  */
#include <math.h> /* log()  */
#include <stdlib.h>
#include <fcntl.h>
#include "midi.h"

/* write midi header
 */
int
smf_header_fmt (int fd,
		unsigned short format,
		unsigned short tracks,
		unsigned short divisions)
{
  int num;

  num = write (fd, "MThd", 4);
  num += wblong (fd, 6); /* head data size (= 6)  */
  num += wbshort (fd, format);
  num += wbshort (fd, tracks);
  num += wbshort (fd, divisions);
  return num;
  /* num = 14  */
}

/* write program change
 */
int
smf_prog_change (int fd, char channel, char prog)
{
  unsigned char data[3];

  data[0] = 0x00; /* delta time  */
  data[1] = 0xC0 + channel;
  data[2] = prog;
  return write (fd, data, 3);
}

int
smf_tempo_change (int fd, double tempo)
{
//  int num = write_var_len (fd, dtime);
  
  int microseconds=(int)(60.0/tempo*1000000.0 +0.5);
  unsigned char data[7];
  data[0]=0x00; // delta time
  data[1]=0xff;
  data[2]=0x51;
  data[3]=0x03;
  data[4] = (microseconds >> 16) & 0xff;
  data[5] = (microseconds >> 8)  & 0xff;
  data[6] = (microseconds >> 0)  & 0xff;
  return write (fd,data,7);
}

/* note on
 */
int
smf_note_on (int fd, long dtime, char note, char vel, char channel)
{
  unsigned char data[3];
  int num;

  num = write_var_len (fd, dtime);
  data[0] = 0x90 + channel;
  data[1] = note;
  data[2] = vel;
  num += write (fd, data, 3);
  return num;
}

/* note off
 */
int
smf_note_off (int fd, long dtime, char note, char vel, char channel)
{
  unsigned char data[3];
  int num;

  num = write_var_len (fd, dtime);
  data[0] = 0x80 + channel;
  data[1] = note;
  data[2] = vel;
  num += write (fd, data, 3);
  return num;
}

/* write track head
 */
int
smf_track_head (int fd, unsigned long size)
{
  int num;
  num = write (fd, "MTrk", 4);
  num += wblong (fd, size);
  return num;
  /* num = 8  */
}

void smf_change_head(int fd,unsigned long size)
{
  lseek(fd,18,SEEK_SET);
  wblong (fd, size);
}

/* write track end
 */
int
smf_track_end (int fd)
{
  unsigned char data[4];
  data[0] = 0x00; /* delta time  */
  data[1] = 0xff;
  data[2] = 0x2f;
  data[3] = 0x00;
  return write (fd, data, 4);
}

/* write/read variable-length variable
 */
int
write_var_len (int fd, long value)
{
  unsigned char rep[4];
  int bytes;

  bytes = 1;
  rep[3] = value & 0x7f;
  value >>= 7;
  while (value >0)
    {
      rep[3 - bytes] = (value & 0x7f) | 0x80;
      bytes ++;
      value >>= 7;
    }

  return (write (fd, &rep[4-bytes], bytes));
}

int
read_var_len (int fd, long *value)
{
  unsigned char c;
  int bytes;

  *value = 0;
  bytes = 0;
  do
    {
      (*value) <<= 7;
      bytes += read (fd, &c, 1);
      (*value) += (c & 0x7f);
    }
  while ( (c & 0x80) == 0x80 );

  return bytes;
}

/* Write long, big-endian: big end first.
 * RETURN VAULE
 *  written bytes
 */
int
wblong (int fd, unsigned long ul)
{
  unsigned char data[4];
  data[0] = (char) (ul >> 24) & 0xff;
  data[1] = (char) (ul >> 16) & 0xff;
  data[2] = (char) (ul >> 8) & 0xff;
  data[3] = (char) (ul) & 0xff;
  return write (fd, data, 4);
}

/* Write short, big-endian: big end first.
 * RETURN VAULE
 *  written bytes
 */
int
wbshort (int fd, unsigned short us)
{
  unsigned char data[2];
  data[0] = (char) (us >> 8) & 0xff;
  data[1] = (char) (us) & 0xff;
  return write (fd, data, 2);
}

