/*

Directory Opus 4
Original GPL release version 4.12
Copyright 1993-2000 Jonathan Potter

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

All users of Directory Opus 4 (including versions distributed
under the GPL) are entitled to upgrade to the latest version of
Directory Opus version 5 at a reduced price. Please see
http://www.gpsoft.com.au for more information.

The release of Directory Opus 4 under the GPL in NO WAY affects
the existing commercial status of Directory Opus 5.

*/

#include "diskop.h"

char *version="$VER: DOpus_Disk 3.0beta (" __DATE__ ") made under GPL license by Jacek Rzeuski";

struct DOpusBase *DOpusBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;
struct Library *IconBase = NULL;
struct Library *GadToolsBase = NULL;

struct RequesterBase req;

struct TagItem commonGTtags[] = {
                    { GT_Underscore, '_' },
                    { TAG_END, 0 }
};

/*
ULONG BitTable[32]={
    0xfffffffe,0xfffffffd,0xfffffffb,0xfffffff7,
    0xffffffef,0xffffffdf,0xffffffbf,0xffffff7f,
    0xfffffeff,0xfffffdff,0xfffffbff,0xfffff7ff,
    0xffffefff,0xffffdfff,0xffffbfff,0xffff7fff,
    0xfffeffff,0xfffdffff,0xfffbffff,0xfff7ffff,
    0xffefffff,0xffdfffff,0xffbfffff,0xff7fffff,
    0xfeffffff,0xfdffffff,0xfbffffff,0xf7ffffff,
    0xefffffff,0xdfffffff,0xbfffffff,0x7fffffff};
*/
USHORT
    trashcanicon_1_data[]={
        0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
        0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
        0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
        0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
        0x03ff,0xfff5,0x5500,0x0000,0x07ff,0xffff,0xaa80,0x0000,
        0x0000,0x0000,0x0000,0x0000,0x03ff,0xff55,0x5400,0x0000,
        0x03ff,0xffff,0xaa80,0x0000,0x038f,0xfc7d,0x4200,0x0000,
        0x0317,0xf8be,0x8880,0x0000,0x0337,0xf9bf,0x9100,0x0000,
        0x0313,0xf9bf,0x1000,0x0000,0x019b,0xf9be,0x9100,0x0000,
        0x019b,0xf93f,0x1200,0x0000,0x019b,0xf9be,0x9100,0x0000,
        0x019b,0xf93f,0x2200,0x0000,0x019b,0xf9be,0x2500,0x0000,
        0x0189,0xf93f,0x2200,0x0000,0x01cd,0xf9be,0x2400,0x0000,
        0x00c9,0xf93d,0x2a00,0x0000,0x00cd,0xf9be,0x4400,0x0000,
        0x00c7,0xf9fe,0x4a00,0x0000,0x00e3,0xfe75,0x1400,0x0000,
        0x007f,0xffea,0xa800,0x0000,0x0000,0x0000,0x0000,0x0000,
        0x0000,0x0000,0x0000,0x0000,
        0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
        0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
        0x0000,0x0000,0x0000,0x0000,0x0000,0x3fe0,0x0000,0x0000,
        0x0000,0xe038,0x0000,0x0000,0x07ff,0xffff,0xffc0,0x0000,
        0x0fff,0xffff,0xffe0,0x0000,0x1fff,0xffff,0xfff0,0x0000,
        0x1fff,0xffff,0xfff0,0x0000,0x0fff,0xffff,0xffe0,0x0000,
        0x0fff,0xffff,0xffe0,0x0000,0x0fff,0xffff,0xffe0,0x0000,
        0x0fff,0xffff,0xffe0,0x0000,0x0fff,0xffff,0xffe0,0x0000,
        0x0fff,0xffff,0xffe0,0x0000,0x07ff,0xffff,0xffe0,0x0000,
        0x07ff,0xffff,0xffc0,0x0000,0x07ff,0xffff,0xffc0,0x0000,
        0x07ff,0xffff,0xffc0,0x0000,0x07ff,0xffff,0xffc0,0x0000,
        0x07ff,0xffff,0xffc0,0x0000,0x07ff,0xffff,0xffc0,0x0000,
        0x03ff,0xffff,0xff80,0x0000,0x03ff,0xffff,0xff80,0x0000,
        0x03ff,0xffff,0xff80,0x0000,0x03ff,0xffff,0xfff8,0x0000,
        0x03ff,0xffff,0xffff,0xe000,0x01ff,0xffff,0xffff,0xe000,
        0x000f,0xffff,0xfffc,0x0000},

    trashcanicon_2_data[]={
        0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
        0x0003,0xffff,0x8000,0x0000,0x00fc,0x0000,0x7e00,0x0000,
        0x0702,0xaaaa,0x81c0,0x0000,0x0855,0x6ab5,0x5420,0x0000,
        0x02aa,0xaaaa,0xabc0,0x0000,0x00fd,0x5555,0x7e00,0x0000,
        0x0003,0xffff,0x8000,0x0000,0x0000,0x0000,0x0000,0x0000,
        0x0000,0x0000,0x0000,0x0000,0x03ff,0xff55,0x5400,0x0000,
        0x03ff,0xffff,0xaa80,0x0000,0x038f,0xfc7d,0x4200,0x0000,
        0x0317,0xf8be,0x8880,0x0000,0x0337,0xf9bf,0x9100,0x0000,
        0x0313,0xf9bf,0x1000,0x0000,0x019b,0xf9be,0x9100,0x0000,
        0x019b,0xf93f,0x1200,0x0000,0x019b,0xf9be,0x9100,0x0000,
        0x019b,0xf93f,0x2200,0x0000,0x019b,0xf9be,0x2500,0x0000,
        0x0189,0xf93f,0x2200,0x0000,0x01cd,0xf9be,0x2400,0x0000,
        0x00c9,0xf93d,0x2a00,0x0000,0x00cd,0xf9be,0x4400,0x0000,
        0x00c7,0xf9fe,0x4a00,0x0000,0x00e3,0xfe75,0x1400,0x0000,
        0x007f,0xffea,0xa800,0x0000,0x0000,0x0000,0x0000,0x0000,
        0x0000,0x0000,0x0000,0x0000,
        0x0000,0x0000,0x0000,0x0000,0x0003,0xffff,0x8000,0x0000,
        0x00ff,0xffff,0xfe00,0x0000,0x07ff,0xffff,0xffc0,0x0000,
        0x0fff,0xffff,0xffe0,0x0000,0x1fff,0xffff,0xfff0,0x0000,
        0x1fff,0xffff,0xfff0,0x0000,0x0fff,0xffff,0xffe0,0x0000,
        0x00ff,0xffff,0xfe00,0x0000,0x0003,0xffff,0x8000,0x0000,
        0x1fff,0xffff,0xfff0,0x0000,0x0fff,0xffff,0xffe0,0x0000,
        0x0fff,0xffff,0xffe0,0x0000,0x0fff,0xffff,0xffe0,0x0000,
        0x0fff,0xffff,0xffe0,0x0000,0x0fff,0xffff,0xffe0,0x0000,
        0x0fff,0xffff,0xffe0,0x0000,0x07ff,0xffff,0xffe0,0x0000,
        0x07ff,0xffff,0xffc0,0x0000,0x07ff,0xffff,0xffc0,0x0000,
        0x07ff,0xffff,0xffc0,0x0000,0x07ff,0xffff,0xffc0,0x0000,
        0x07ff,0xffff,0xffc0,0x0000,0x07ff,0xffff,0xffc0,0x0000,
        0x03ff,0xffff,0xff80,0x0000,0x03ff,0xffff,0xff80,0x0000,
        0x03ff,0xffff,0xff80,0x0000,0x03ff,0xffff,0xfff8,0x0000,
        0x03ff,0xffff,0xffff,0xe000,0x01ff,0xffff,0xffff,0xe000,
        0x000f,0xffff,0xfffc,0x0000};

struct Image
    trashcanicon_1_image={
        0,0,51,31,2,trashcanicon_1_data,3,0,NULL},
    trashcanicon_2_image={
        0,0,51,31,2,trashcanicon_2_data,3,0,NULL};

struct DrawerData
    trashcanicon_drawer={
        {50,50,400,100,255,255,0,0x240027f,
            NULL,NULL,NULL,NULL,NULL,90,40,65535,65535,WBENCHSCREEN},
        0,0,0,0};

struct DiskObject
    trashcanicon_icon={
        WB_DISKMAGIC,WB_DISKVERSION,
        {NULL,0,0,51,31,
            GFLG_GADGIMAGE|GFLG_GADGHIMAGE,
            GACT_RELVERIFY|GACT_IMMEDIATE,
            GTYP_BOOLGADGET,
            (APTR)&trashcanicon_1_image,(APTR)&trashcanicon_2_image,
            NULL,0,NULL,0,NULL},
        WBGARBAGE,NULL,NULL,
        NO_ICON_POSITION,NO_ICON_POSITION,&trashcanicon_drawer,NULL,0};

ULONG
    bootblock_13[13]={
        0x00000000,
        0x00000000,
        0x00000370,

        0x43fa0018,0x4eaeffa0,0x4a80670a,0x20402068,
        0x00167000,0x4e7570ff,0x60fa646f,0x732e6c69,
        0x62726172,0x79000000},

    bootblock_20[24]={
        0x00000000,
        0x00000000,
        0x00000370,

        0x43fa003e,0x70254eae,0xfdd84a80,0x670c2240,
        0x08e90006,0x00224eae,0xfe6243fa,0x00184eae,
        0xffa04a80,0x670a2040,0x20680016,0x70004e75,
        0x70ff4e75,0x646f732e,0x6c696272,0x61727900,
        0x65787061,0x6e73696f,0x6e2e6c69,0x62726172,
        0x79000000};
