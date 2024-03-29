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

#include "dopus.h"
#include "DirectoryOpus_rev.h"

#define HOTKEY_UNICONIFY   1
#define HOTKEY_ABORT     3
#define HOTKEY_MMB     5
#define HOTKEY_HOTKEY    10

struct ProgressBar
{
	int barX, barY;
	int descY;
	int curr, max;
	int last_w;
	BOOL incignore;
	BOOL hide;
};

void progressbar(struct ProgressBar *bar);

struct Gadget abortopgad =
{
	NULL, 0, 0, 104, 0, GFLG_GADGHCOMP, GACT_RELVERIFY, GTYP_BOOLGADGET, NULL, NULL, NULL, 0, NULL, 0, NULL
};

static struct NewWindow progresswindow =
{
	0, 0, 0, 0, 255, 255, IDCMP_GADGETUP | IDCMP_RAWKEY, WFLG_RMBTRAP | WFLG_ACTIVATE, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, CUSTOMSCREEN
};

static struct Window *pwindow;
static struct RastPort *prp;
static struct DOpusRemember *prog_key;
static struct ProgressBar bar[2];
static int prog_xoff, prog_yoff, prog_xextra, prog_yextra, prog_areax;

static struct NewBroker hotkey_broker =
{
	NB_VERSION,
	NULL,
	"Directory Opus � Jonathan Potter",
	"The most amazing program ever written",
	0, COF_SHOW_HIDE, 100, NULL, 0
};

static IX hotkey_ix = { IX_VERSION };

void hotkeytaskcode()
{
	char cxname[20];
	int top, sig, waitbits, commodity = 0, command, x, run = 1;
	struct dopustaskmsg *hmsg;
	struct MsgPort *inputport;
	struct IntuiMessage *msg;
	ULONG class, msgid, msgtype;
	USHORT gadgetid = 0;
	struct dopushotkey *hotkey;
	CxObj *broker = NULL, *hotkey_filter = NULL, *mmb_filter = NULL;
	CxMsg *cxmsg;

	hotkeymsg_port = IExec->CreatePort(NULL, 0);
	inputport = IExec->CreatePort(NULL, 0);

	if(CxBase && ICommodities)
	{
		strcpy(cxname, "Directory Opus");
		if(system_dopus_runcount)
		{
			char tmp[8];

			sprintf(tmp, " (%d)", system_dopus_runcount + 1);
			strcat(cxname, tmp);
		}
		hotkey_broker.nb_Name = cxname;
		hotkey_broker.nb_Port = inputport;
		if((broker = ICommodities->CxBroker(&hotkey_broker, NULL)))
		{

			/* Initialise main hotkey */
			if((hotkey_filter = set_dopus_filter(broker, inputport, NULL, config->hotkeycode, config->hotkeyqual, HOTKEY_UNICONIFY, 1)))
			{

				hotkey_ix.ix_Code = IECODE_LBUTTON;
				hotkey_ix.ix_Qualifier = IEQUALIFIER_RBUTTON;
				hotkey_ix.ix_QualMask = 0xffff & ~(IEQUALIFIER_LEFTBUTTON | IEQUALIFIER_RELATIVEMOUSE | IEQUALIFIER_CAPSLOCK);
				if(set_dopus_filter(broker, inputport, "rawmouse lbutton", 0, 0, HOTKEY_ABORT, 0))
				{
					hotkey_ix.ix_Code = IECODE_RBUTTON;
					hotkey_ix.ix_Qualifier = IEQUALIFIER_LEFTBUTTON;
					hotkey_ix.ix_QualMask = 0xffff & ~(IEQUALIFIER_RBUTTON | IEQUALIFIER_RELATIVEMOUSE | IEQUALIFIER_CAPSLOCK);
					if(set_dopus_filter(broker, inputport, "rawmouse rbutton", 0, 0, HOTKEY_ABORT, 0))
					{

						if(config->hotkeyflags & HOTKEY_USEMMB)
						{
							hotkey_ix.ix_Code = IECODE_MBUTTON;
							hotkey_ix.ix_Qualifier = IEQUALIFIER_MIDBUTTON;
							hotkey_ix.ix_QualMask = 0xffff & ~(IEQUALIFIER_RELATIVEMOUSE | IEQUALIFIER_CAPSLOCK);
							mmb_filter = set_dopus_filter(broker, inputport, "rawmouse midbutton", 0, 0, HOTKEY_MMB, 1);
						}

						add_hotkey_objects(broker, inputport, 1);
						set_hotkey(hotkey_filter, config->hotkeycode, config->hotkeyqual);

						ICommodities->ActivateCxObj(broker, 1);
						commodity = 1;
					}
				}
			}
		}
		if(broker && (!commodity))
		{
			add_hotkey_objects(broker, inputport, 0);
			ICommodities->DeleteCxObjAll(broker);
		}
	}

	waitbits = 1 << hotkeymsg_port->mp_SigBit;
	if(commodity)
		waitbits |= 1 << inputport->mp_SigBit;

	while(run)
	{
		sig = IExec->Wait(waitbits);
		command = 0;

		if(commodity)
		{
			while((cxmsg = (CxMsg *)IExec->GetMsg(inputport)))
			{
				msgid = ICommodities->CxMsgID(cxmsg);
				msgtype = ICommodities->CxMsgType(cxmsg);
				IExec->ReplyMsg((struct Message *)cxmsg);
				switch(msgtype)
				{
				case CXM_IEVENT:
					if((command = msgid) >= HOTKEY_HOTKEY)
					{
						x = command - HOTKEY_HOTKEY;
						command = HOTKEY_HOTKEY;
						hotkey = dopus_firsthotkey;
						while(x-- && hotkey)
							hotkey = hotkey->next;
						dopus_globalhotkey = hotkey;
					}
					break;

				case CXM_COMMAND:
					switch (msgid)
					{
					case CXCMD_KILL:
						command = HOTKEY_HOTKEY;
						dopus_globalhotkey = (struct dopushotkey *)-1;
						break;

					case CXCMD_DISABLE:
						ICommodities->ActivateCxObj(broker, 0);
						break;

					case CXCMD_ENABLE:
						ICommodities->ActivateCxObj(broker, 1);
						break;

					case CXCMD_APPEAR:
						command = HOTKEY_UNICONIFY;
						break;

					case CXCMD_DISAPPEAR:
						command = HOTKEY_HOTKEY;
						dopus_globalhotkey = (struct dopushotkey *)-2;
						break;
					}
					break;
				}
			}
		}

		switch (command)
		{
		case HOTKEY_ABORT:
			if(((struct IntuitionBase *)(IIntuition->Data.LibBase))->ActiveWindow == Window)
			{
				status_haveaborted = status_rexxabort = 1;
				IExec->Signal((struct Task *)main_proc, INPUTSIG_ABORT);
			}
			break;

		case HOTKEY_MMB:
			if(!(config->hotkeyflags & HOTKEY_USEMMB))
				break;
		case HOTKEY_UNICONIFY:
			if(status_configuring == -1)
				break;
			if(status_iconified == 1)
				IExec->Signal((struct Task *)main_proc, INPUTSIG_UNICONIFY);
			else if(status_iconified == 0)
			{
				if(MainScreen)
				{
					if(!status_configuring && config->screenflags & SCRFLAGS_HALFHEIGHT)
						top = main_scr.TopEdge;
					else
						top = 0;

					if((((struct IntuitionBase *)(IIntuition->Data.LibBase))->FirstScreen == MainScreen) && (MainScreen->TopEdge == top) && (!((struct IntuitionBase *)(IIntuition->Data.LibBase))->ActiveWindow || (((struct IntuitionBase *)(IIntuition->Data.LibBase))->ActiveWindow->WScreen == MainScreen)))
					{
						IIntuition->ScreenToBack(MainScreen);
						if(Window->Parent && Window->Parent->WScreen == ((struct IntuitionBase *)(IIntuition->Data.LibBase))->FirstScreen)
							IIntuition->ActivateWindow(Window->Parent);
						else
							IIntuition->ActivateWindow(((struct IntuitionBase *)(IIntuition->Data.LibBase))->FirstScreen->FirstWindow);
					}
					else
					{
						struct Window *window;

						IIntuition->ScreenToFront(MainScreen);
						if(MainScreen->FirstWindow == ansiread_window)
							window = Window;
						else
							window = MainScreen->FirstWindow;

						if(!(window->Flags & WFLG_BACKDROP))
							IIntuition->WindowToFront(window);
						IIntuition->ActivateWindow(window);
					}
					IIntuition->MoveScreen(MainScreen, 0, top - MainScreen->TopEdge);
				}
				else
				{
					if(((struct IntuitionBase *)(IIntuition->Data.LibBase))->ActiveWindow == Window && ((struct IntuitionBase *)(IIntuition->Data.LibBase))->FirstScreen == Window->WScreen && Window->WScreen->LayerInfo.top_layer == Window->RPort->Layer)
					{
						IIntuition->WindowToBack(Window);
						if(Window->Parent)
							IIntuition->ActivateWindow(Window->Parent);
					}
					else
					{
						IIntuition->ScreenToFront(Window->WScreen);
						IIntuition->WindowToFront(Window);
						IIntuition->ActivateWindow(Window);
					}
				}
			}
			break;

		case HOTKEY_HOTKEY:
			if(!status_configuring)
				IExec->Signal((struct Task *)main_proc, INPUTSIG_HOTKEY);
			break;
		}

		if(pwindow)
		{
			while((msg = (struct IntuiMessage *)IExec->GetMsg(pwindow->UserPort)))
			{
				if((class = msg->Class) == IDCMP_GADGETUP)
					gadgetid = ((struct Gadget *)msg->IAddress)->GadgetID;
				IExec->ReplyMsg((struct Message *)msg);
				if(class == IDCMP_GADGETUP && gadgetid == 0)
				{
					status_haveaborted = status_rexxabort = 1;
					IExec->Signal((struct Task *)main_proc, INPUTSIG_ABORT);
				}
			}
		}

		while((hmsg = (struct dopustaskmsg *)IExec->GetMsg(hotkeymsg_port)))
		{

#define BAR_ID (hmsg->flag)

			switch (hmsg->command)
			{
			case PROGRESS_UPDATE:
				if(pwindow)
				{
					if(hmsg->value > -1)
					{
						bar[BAR_ID].curr = hmsg->value;
						bar[BAR_ID].max = hmsg->total;

						progressbar(&bar[BAR_ID]);
					}
					if(bar[BAR_ID].hide)
						break;
					if(BAR_ID == 0 || hmsg->data)
						progresstext(bar[BAR_ID].descY, hmsg->value, hmsg->total, hmsg->data);
				}
				break;

			case PROGRESS_INCREASE:
				if(pwindow)
				{
					if(bar[BAR_ID].incignore)
						break;

					bar[BAR_ID].curr += hmsg->value;
					if(bar[BAR_ID].curr > bar[BAR_ID].max)
						bar[BAR_ID].curr = bar[BAR_ID].max;

					progressbar(&bar[BAR_ID]);
					if(bar[BAR_ID].hide)
						break;
					progresstext(bar[BAR_ID].descY, bar[BAR_ID].curr, bar[BAR_ID].max, NULL);
				}
				break;

			case PROGRESS_OPEN:
				if(!pwindow)
				{
					openprogresswindow(hmsg->data, hmsg->value, hmsg->total, BAR_ID);
					if(pwindow)
					{
						IDOpus->SetBusyPointer(pwindow);
						waitbits |= 1 << pwindow->UserPort->mp_SigBit;
					}
				}
				break;

			case PROGRESS_CLOSE:
				if(pwindow)
				{
					waitbits &= ~(1 << pwindow->UserPort->mp_SigBit);
					IIntuition->CloseWindow(pwindow);
					pwindow = NULL;
				}
				IDOpus->LFreeRemember(&prog_key);
				break;

			case TASK_QUIT:
				run = 0;
				break;

			case HOTKEY_HOTKEYCHANGE:
				if(commodity)
				{
					set_hotkey(hotkey_filter, config->hotkeycode, config->hotkeyqual);

					if(config->hotkeyflags & HOTKEY_USEMMB)
					{
						if(!mmb_filter)
						{
							hotkey_ix.ix_Code = IECODE_MBUTTON;
							hotkey_ix.ix_Qualifier = IEQUALIFIER_MIDBUTTON;
							hotkey_ix.ix_QualMask = 0xffff & ~(IEQUALIFIER_RELATIVEMOUSE | IEQUALIFIER_CAPSLOCK);
							mmb_filter = set_dopus_filter(broker, inputport, "rawmouse midbutton", 0, 0, HOTKEY_MMB, 1);
						}
					}
					else if(mmb_filter)
					{
						ICommodities->DeleteCxObjAll(mmb_filter);
						mmb_filter = NULL;
					}
				}
				break;

			case HOTKEY_KILLHOTKEYS:
				if(commodity)
					add_hotkey_objects(broker, inputport, 0);
				break;

			case HOTKEY_NEWHOTKEYS:
				if(commodity)
					add_hotkey_objects(broker, inputport, 1);
				break;
			}
			IExec->ReplyMsg((struct Message *)hmsg);
		}
	}

	if(commodity)
	{
		add_hotkey_objects(broker, inputport, 0);
		ICommodities->DeleteCxObjAll(broker);
		while((cxmsg = (CxMsg *)IExec->GetMsg(inputport)))
			IExec->ReplyMsg((struct Message *)cxmsg);
	}

	if(pwindow)
		IIntuition->CloseWindow(pwindow);
	IDOpus->LFreeRemember(&prog_key);
	IExec->DeletePort(inputport);
	IExec->DeletePort(hotkeymsg_port);
	IExec->Wait(0);
}

void add_hotkey_objects(CxObj *broker, struct MsgPort *port, int add)
{
	static CxObj **filter_table;
	static int hotkey_count;
	struct dopushotkey *hotkey;
	int a;

	if(filter_table)
	{
		for(a = 0; a < hotkey_count; a++)
			ICommodities->DeleteCxObjAll(filter_table[a]);
		IExec->FreeMem(filter_table, hotkey_count * sizeof(CxObj *));
		filter_table = NULL;
	}

	if(add == 0)
		return;

	for(hotkey = dopus_firsthotkey, hotkey_count = 0; hotkey; hotkey = hotkey->next)
		hotkey_count++;
	if(hotkey_count == 0)
		return;

	filter_table = IExec->AllocMem(hotkey_count * sizeof(CxObj *), MEMF_CLEAR);
	if(filter_table == NULL)
		return;

	for(a = 0, hotkey = dopus_firsthotkey; hotkey; hotkey = hotkey->next, a++)
		filter_table[a] = set_dopus_filter(broker, port, NULL, hotkey->code, hotkey->qualifier, HOTKEY_HOTKEY + a, 1);
}

CxObj *set_dopus_filter(CxObj *broker, struct MsgPort *port, STRPTR string, USHORT code, USHORT qual, int command, int translate)
{
	CxObj *filter;

	if((filter = CxFilter(string)))
	{
		if(!string)
		{
			set_hotkey(filter, code, qual);
		}
		else
		{
			hotkey_ix.ix_Class = IECLASS_RAWMOUSE;
			hotkey_ix.ix_CodeMask = 0xff;
			hotkey_ix.ix_QualSame = 0;
			ICommodities->SetFilterIX(filter, &hotkey_ix);
		}
		ICommodities->AttachCxObj(broker, filter);
		ICommodities->AttachCxObj(filter, CxSender(port, command));
		if(translate)
			ICommodities->AttachCxObj(filter, CxTranslate(NULL));

		return (filter);
	}
	return (NULL);
}

void set_hotkey(CxObj *filter, USHORT code, USHORT qual)
{
	if(filter)
	{
		hotkey_ix.ix_Class = IECLASS_RAWKEY;
		if(qual == 0 && code == (USHORT) ~ 0)
		{
			hotkey_ix.ix_Code = 0xffff;
			hotkey_ix.ix_CodeMask = 0xffff;
			hotkey_ix.ix_Qualifier = 0xffff;
		}
		else
		{
			if(code == (USHORT) ~ 0)
			{
				hotkey_ix.ix_Code = 0;
				hotkey_ix.ix_CodeMask = 0;
			}
			else
			{
				hotkey_ix.ix_Code = code;
				hotkey_ix.ix_CodeMask = 0xff;
			}
			hotkey_ix.ix_Qualifier = qual & VALID_QUALIFIERS;
			hotkey_ix.ix_QualMask = VALID_QUALIFIERS;
		}
		ICommodities->SetFilterIX(filter, &hotkey_ix);
	}
}

void openprogresswindow(STRPTR title, int value, int total, int flag)
{
	struct TextFont *font;
	char *gadtxt[] = { globstring[STR_ABORT], NULL };
	int a, incignore = 0;

	if(flag & 0x80)
	{
		incignore = 1;
		flag &= ~0x80;
	}

	if(config->generalscreenflags & SCR_GENERAL_REQDRAG)
	{
		prog_xoff = Window->WScreen->WBorLeft + 2;
		prog_yoff = Window->WScreen->WBorTop + Window->WScreen->Font->ta_YSize + 2;
		prog_xextra = prog_xoff + Window->WScreen->WBorRight - 2;
	}
	else
	{
		prog_xoff = 2;
		prog_yoff = 1;
		prog_xextra = 0;
	}

	progresswindow.Width = 384 + (scr_font[FONT_REQUEST]->tf_XSize * 8) + prog_xextra;

	if(progresswindow.Width > Window->WScreen->Width)
	{
		font = scr_font[FONT_GENERAL];
		progresswindow.Width = 432;
		if(config->generalscreenflags & SCR_GENERAL_REQDRAG)
			progresswindow.Width += prog_xextra;
	}
	else
		font = scr_font[FONT_REQUEST];

	progresswindow.Height = (font->tf_YSize * 5) + 22;
	if(flag)
	{
		progresswindow.Height += (font->tf_YSize * 2) + 8;
		prog_yextra = (font->tf_YSize * 2) + 8;
	}
	else
		prog_yextra = 0;

	if(config->generalscreenflags & SCR_GENERAL_REQDRAG)
	{
		progresswindow.Height += prog_yoff + Window->WScreen->WBorBottom - 1;
		progresswindow.Flags |= WFLG_DRAGBAR | WFLG_DEPTHGADGET;
		progresswindow.Title = title;
	}
	else
	{
		progresswindow.Flags |= WFLG_BORDERLESS;
		progresswindow.Title = NULL;
	}

	centerwindow(&progresswindow);
	if(!(pwindow = IIntuition->OpenWindow(&progresswindow)))
		return;
	prp = pwindow->RPort;
	setupwindreq(pwindow);
	IGraphics->SetFont(prp, font);

	IDOpus->Do3DBox(prp, 26 + prog_xoff, 6 + prog_yoff, pwindow->Width - prog_xextra - 56, (font->tf_YSize * 4) + prog_yextra, screen_pens[config->gadgetbotcol].pen, screen_pens[config->gadgettopcol].pen);
	IGraphics->SetAPen(prp, screen_pens[config->requestfg].pen);

	abortopgad.LeftEdge = (pwindow->Width - abortopgad.Width) / 2;
	abortopgad.Height = font->tf_YSize + 4;
	abortopgad.TopEdge = pwindow->Height - pwindow->BorderBottom - 9 - font->tf_YSize;
	IDOpus->AddGadgetBorders(&prog_key, &abortopgad, 1, screen_pens[config->gadgettopcol].pen, screen_pens[config->gadgetbotcol].pen);
	IDOpus->AddGadgets(pwindow, &abortopgad, gadtxt, 1, screen_pens[config->gadgettopcol].pen, screen_pens[config->gadgetbotcol].pen, 1);

	IGraphics->SetAPen(prp, screen_pens[0].pen);
	IGraphics->SetDrMd(prp, JAM2);
	IGraphics->RectFill(prp, 26 + prog_xoff, 6 + prog_yoff, (prog_areax = prog_xoff + pwindow->Width - prog_xextra - 31), prog_yoff + (font->tf_YSize * 4) + prog_yextra + 5);

	for(a = 0; a < 2; a++)
	{
		bar[a].last_w = 0;
		bar[a].curr = value;
		bar[a].max = total;
		bar[a].incignore = incignore;
		bar[a].hide = (a == 0) ? 0 : !flag;
		bar[a].barX = (((pwindow->Width - 356) / 2) + 28) - font->tf_XSize;
		bar[a].barY = font->tf_YSize + 3 + prog_yoff;
		if(a == 0)
			bar[a].barY += prog_yextra + 3;

		if(!bar[a].hide)
		{
			IDOpus->Do3DBox(prp, bar[a].barX, bar[a].barY, 300, font->tf_YSize, screen_pens[config->gadgetbotcol].pen, screen_pens[config->gadgettopcol].pen);

			bar[a].descY = bar[a].barY + font->tf_YSize + (font->tf_YSize / 2) + font->tf_Baseline;

			IGraphics->SetAPen(prp, screen_pens[1].pen);
			IGraphics->SetBPen(prp, screen_pens[0].pen);
			IGraphics->Move(prp, bar[a].barX - (IGraphics->TextLength(prp, "0% ", 3)) - 4, bar[a].barY + font->tf_Baseline);
			IGraphics->Text(prp, "0%", 2);
			IGraphics->Move(prp, bar[a].barX + 302 + font->tf_XSize, bar[a].barY + font->tf_Baseline);
			IGraphics->Text(prp, "100%", 4);

			if(a == 0)
				progresstext(bar[a].descY, value, total, NULL);
			progressbar(&bar[a]);
		}
	}
}

void progresstext(int y, int val, int total, STRPTR text)
{
	char buf[80], *ptr;
	int x, y1, len;

	if(val == -1)
		ptr = globstring[total ? STR_ABORTED : STR_COMPLETED];
	else
	{
		if(text)
			IDOpus->LStrnCpy(buf, text, (pwindow->Width - prog_xextra - 56) / prp->Font->tf_XSize);
		else
			sprintf(buf, globstring[STR_REMAINING], val, total);
		ptr = buf;
	}
	x = 26 + ((pwindow->Width - prog_xextra - 56 - IGraphics->TextLength(prp, ptr, (len = strlen(ptr)))) / 2) + prog_xoff;
	y1 = y - prp->Font->tf_Baseline;

	if(x > prog_xoff + 26)
	{
		IGraphics->SetAPen(prp, screen_pens[0].pen);
		IGraphics->RectFill(prp, prog_xoff + 26, y1, x - 1, y1 + prp->Font->tf_YSize);
	}

	IGraphics->SetAPen(prp, screen_pens[1].pen);
	IGraphics->Move(prp, x, y);
	IGraphics->Text(prp, ptr, len);

	if(prp->cp_x <= prog_areax)
	{
		IGraphics->SetAPen(prp, screen_pens[0].pen);
		IGraphics->RectFill(prp, prp->cp_x, y1, prog_areax - 1, y1 + prp->Font->tf_YSize);
	}
}

void progressbar(struct ProgressBar *bar)
{
	int w;
	BOOL draw;

	if(bar->hide)
		return;
	if(bar->curr > 0)
	{
		float f = (float)(bar->curr) / (float)(bar->max);

		if((w = (int)(300 * f)) > 300)
			w = 300;
		else if(w < 1)
			w = 1;
		draw = (w != bar->last_w);
		IGraphics->SetAPen(prp, screen_pens[3].pen);
	}
	else
	{
		draw = TRUE;
		w = 300;
		IGraphics->SetAPen(prp, screen_pens[0].pen);
	}
	if(draw)
	{
		IGraphics->RectFill(prp, bar->barX, bar->barY, bar->barX + w - 1, bar->barY + prp->Font->tf_YSize - 1);
		bar->last_w = w;
	}
}

static char *Kstr = "K  ";

void clocktask()
{
	ULONG chipc, fast, wmes, h, m, s, cx, sig, cy, /*len,*/ ct, chipnum, fastnum, a, active = 1, usage;
	int len;
	USHORT clock_width, clock_height, scr_height;
	char buf[160], date[20], time[20], formstring[160], memstring[160], ampm;
	struct MsgPort *clock_time_port;
	struct timerequest ctimereq;
	struct DateTime datetime = { { 0, } };
	struct dopustaskmsg *cmsg;
	struct RastPort clock_rp;

	IExec->Forbid();
	IExec->CopyMem((STRPTR)main_rp, (STRPTR)&clock_rp, sizeof(struct RastPort));
	SetDrawModes(&clock_rp, config->clockfg, config->clockbg, JAM2);
	IGraphics->SetFont(&clock_rp, scr_font[FONT_CLOCK]);
	scr_height = scrdata_height + scrdata_yoffset;
	clock_width = scrdata_clock_width;
	clock_height = scrdata_clock_height;
	ct = scr_height - (clock_height - 1);
	cy = scrdata_clock_ypos + scr_font[FONT_CLOCK]->tf_Baseline - 1;
	IExec->Permit();

	clockmsg_port = IExec->CreatePort(NULL, 0);
	clock_time_port = IExec->CreatePort(0, 0);

	IExec->OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)&ctimereq, 0);
	ctimereq.tr_node.io_Message.mn_ReplyPort = clock_time_port;
	ctimereq.tr_node.io_Command = TR_ADDREQUEST;
	ctimereq.tr_node.io_Flags = 0;
	ctimereq.tr_time.tv_secs = 0;
	ctimereq.tr_time.tv_micro = 2;
	IExec->SendIO(&ctimereq.tr_node);

	chipnum = getmaxmem(MEMF_CHIP);
	fastnum = getmaxmem(MEMF_FAST);
	a = getmaxmem(MEMF_ANY);

	m = (config->scrclktype & SCRCLOCK_BYTES) ? 3 : 0;
	s = (config->scrclktype & SCRCLOCK_BYTES) ? 1 : 0;

	if(config->scrclktype & SCRCLOCK_C_AND_F)
	{
		sprintf(memstring, "%lc:%%-%ldld%s", globstring[STR_CLOCK_CHIP][0], chipnum + m, Kstr + s);
		if(fastnum > 1)
		{
			sprintf(memstring + strlen(memstring), "%lc:%%-%ldld%s", globstring[STR_CLOCK_FAST][0], fastnum + m, Kstr + s);
			sprintf(memstring + strlen(memstring), "%lc:%%-%ldld%s", globstring[STR_CLOCK_TOTAL][0], a + m, Kstr + s);
		}
	}
	else
	{
		sprintf(memstring, "%s%%-%ldld%s", globstring[STR_CLOCK_CHIP], chipnum + m, Kstr + s);
		if(fastnum > 1)
		{
			sprintf(memstring + strlen(memstring), "%s%%-%ldld%s", globstring[STR_CLOCK_FAST], fastnum + m, Kstr + s);
			sprintf(memstring + strlen(memstring), "%s%%-%ldld%s", globstring[STR_CLOCK_TOTAL], a + m, Kstr + s);
		}
	}

	if(!(config->scrclktype & (SCRCLOCK_MEMORY | SCRCLOCK_CPU | SCRCLOCK_DATE | SCRCLOCK_TIME)))
		sprintf(formstring, NEW_VSTRING); //"Directory Opus  Version %s  Compiled %s  %s", str_version_string, comp_time, comp_date);

	sig = 1 << clock_time_port->mp_SigBit | 1 << clockmsg_port->mp_SigBit;

	FOREVER
	{
		wmes = IExec->Wait(sig);
		if(wmes & 1 << clockmsg_port->mp_SigBit)
		{
			while((cmsg = (struct dopustaskmsg *)IExec->GetMsg(clockmsg_port)))
			{
				switch (cmsg->command)
				{
				case TASK_QUIT:
					if(!(IExec->CheckIO(&ctimereq.tr_node)))
						IExec->AbortIO(&ctimereq.tr_node);
					IExec->WaitIO(&ctimereq.tr_node);
					IExec->CloseDevice((struct IORequest *)&ctimereq);
					IExec->DeletePort(clock_time_port);
					IExec->DeletePort(clockmsg_port);
					clockmsg_port = NULL;
					IExec->ReplyMsg((struct Message *)cmsg);
					IExec->Forbid();
					return;
				case CLOCK_ACTIVE:
					active = cmsg->value;
					break;
				}
				IExec->ReplyMsg((struct Message *)cmsg);
			}
		}
		if(wmes & 1 << clock_time_port->mp_SigBit)
		{
			if(active && !(Window->Flags & WFLG_MENUSTATE))
			{
				if(scr_height > ct + 1)
				{
					if(config->scrclktype & (SCRCLOCK_MEMORY | SCRCLOCK_CPU | SCRCLOCK_DATE | SCRCLOCK_TIME))
					{
						formstring[0] = 0;
						if(config->scrclktype & SCRCLOCK_MEMORY)
						{
							chipc = IExec->AvailMem(MEMF_CHIP);
							fast = IExec->AvailMem(MEMF_FAST);
							if(!(config->scrclktype & SCRCLOCK_BYTES))
							{
								chipc /= 1024;
								fast /= 1024;
							}
							sprintf(buf, memstring, chipc, fast, chipc + fast);
							strcat(formstring, buf);
						}
						if(config->scrclktype & SCRCLOCK_CPU)
						{
							usage = getusage();

							sprintf(buf, "CPU:%3ld%%  ", usage);
							strcat(formstring, buf);
						}
						if(config->scrclktype & (SCRCLOCK_DATE | SCRCLOCK_TIME))
						{
							IDOS->DateStamp(&(datetime.dat_Stamp));
							initdatetime(&datetime, date, time, 0);

							if(config->scrclktype & SCRCLOCK_DATE)
							{
								sprintf(buf, "%-9s  ", date);
								strcat(formstring, buf);
							}
							if(config->scrclktype & SCRCLOCK_TIME)
							{
								if(config->dateformat & DATE_12HOUR)
								{
									h = datetime.dat_Stamp.ds_Minute / 60;
									m = datetime.dat_Stamp.ds_Minute % 60;
									s = datetime.dat_Stamp.ds_Tick / TICKS_PER_SECOND;
									if(h > 11)
									{
										ampm = 'P';
										h -= 12;
									}
									else
									{
										ampm = 'A';
									}
									if(h == 0)
									{
										h = 12;
									}
									sprintf(time, "%02ld:%02ld:%02ld%c", h, m, s, ampm);
								}
								strcat(formstring, time);
							}
						}
					}
					len = strlen(formstring);
					if(len > 1 && formstring[len - 2] == ' ')
						len -= 2;
					cx = (clock_width - dotextlength(&clock_rp, formstring, &len, clock_width - 4)) / 2;
					cx += scrdata_clock_xpos;
					if(cx < scrdata_clock_xpos)
						cx = scrdata_clock_xpos;

					IGraphics->SetAPen(&clock_rp, screen_pens[config->clockfg].pen);
					IGraphics->Move(&clock_rp, cx, cy);
					IGraphics->Text(&clock_rp, formstring, len);
					IGraphics->SetAPen(&clock_rp, screen_pens[config->clockbg].pen);

					if(cx > scrdata_clock_xpos)
						IGraphics->RectFill(&clock_rp, scrdata_clock_xpos, ct, cx - 1, scr_height - 2);
					if(clock_rp.cp_x < clock_width - 1)
						IGraphics->RectFill(&clock_rp, clock_rp.cp_x, ct, clock_width - 2, scr_height - 2);
				}
			}
			ctimereq.tr_time.tv_secs = 1;
			ctimereq.tr_time.tv_micro = 0;
			IExec->SendIO(&ctimereq.tr_node);
		}
	}
}
