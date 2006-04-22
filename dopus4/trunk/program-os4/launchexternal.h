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

#define FS_CURDIR   1
#define FS_SEGMENT  2
#define FS_LAUNCHED 3

#define FF_SAVESEG  1

struct DOpusStartup
{
	struct WBStartup wbstartup;
	int retcode;
};

struct dopus_func_start
{
	BPTR segment;		/* Pointer to pre-loaded segment */
	char *procname;		/* Name to start process as      */
	char *segname;		/* Name to load segment from     */
	int argcount;		/* Argument count                */
	char **args;		/* Argument array                */
	ULONG stack;		/* Stack size for process        */
	char flags;		/* Function flags                */

	struct DOpusRemember *key;	/* Memory key                    */
	struct MsgPort *replyport;	/* Process reply port            */
	struct DOpusStartup startup;	/* Process Startup message       */
	char status;		/* Status byte                   */
	BPTR olddir;		/* Old current directory         */
	struct Segment *resseg;	/* Resident Segment pointer      */
};

#define SEG_DISK  0
#define SEG_PRINT 1
#define SEG_ICON  2
