/*
**	$Id$
**	Generated by IDLTool 51.6
**	Do not edit
**
**	AutoInit stub for dopus
**
**	(C) Copyright 2003-2005 Amiga, Inc.
**	    All Rights Reserved
*/

#include <libraries/dopus.h>
#include <intuition/intuition.h>
#include <dopus/requesters.h>
#include <dopus/config.h>
#include <dopus/stringdata.h>

#include <interfaces/dopus.h>
#include <proto/exec.h>
#include <assert.h>

/****************************************************************************/

__attribute__((weak)) struct Library * DOpusBase = NULL;

void __init_dopus_base(void) __attribute__((constructor));
void __exit_dopus_base(void) __attribute__((destructor));

/****************************************************************************/

void __init_dopus_base(void)
{
    if (DOpusBase != NULL) return; /* Someone was quicker */
    DOpusBase = IExec->OpenLibrary("dopus.library", 0L);
    assert(DOpusBase != NULL);
}

/****************************************************************************/

void __exit_dopus_base(void)
{
    if (DOpusBase) IExec->CloseLibrary((struct Library *)DOpusBase);
}

/****************************************************************************/

