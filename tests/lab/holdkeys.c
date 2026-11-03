/*
 * HoldKeys: holds keys down for a time, as a person playing would, for lab
 * play tests driven from a Shell (Cradle's tools can only tap a key).
 *
 *   HoldKeys SECONDS/N/A,HOLD/K,SWAY/S,PERIOD/N,CYCLE/K
 *
 * HOLD is a comma-separated list of raw key codes in hex (40 Space, 44
 * Return, 4C Up, 4D Down, 4E Right, 4F Left, 60 Left Shift, 63 Ctrl, 64
 * Left Alt); they go down at the start and up at the end. SWAY also holds
 * Left and Right in turn, PERIOD ticks (1/50 s, default 35) each. CYCLE is
 * a list like HOLD whose keys are held one after another, PERIOD ticks
 * each, round and round (for a game that moves in all directions; "4C,4E,
 * 4D,4F" goes up, right, down, left). Ctrl-C stops early. The events go
 * through input.device, to whichever window is active.
 *
 * From DalsinAI/amigapowermanga tests/holdkeys.c (itself from
 * DalsinAI/amigaopentyrian tests/lab/holdkeys.c), with CYCLE added.
 * MIT, Copyright (c) 2026 AmiCradle Team.
 */
#include <exec/types.h>
#include <exec/io.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>

static struct IOStdReq *io;

static void key(UWORD code, BOOL up)
{
	struct InputEvent ev;
	memset(&ev, 0, sizeof(ev));
	ev.ie_Class = IECLASS_RAWKEY;
	ev.ie_Code = code | (up ? IECODE_UP_PREFIX : 0);
	io->io_Command = IND_WRITEEVENT;
	io->io_Data = &ev;
	io->io_Length = sizeof(ev);
	io->io_Flags = 0;
	DoIO((struct IORequest *)io);
}

static int hexval(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

/* a comma-separated list of hex raw key codes, at most 16 */
static int parse(const char *list, UWORD *codes)
{
	int n = 0, v = -1;
	for (const char *s = list ? list : ""; ; ++s)
	{
		int h = *s ? hexval(*s) : -1;
		if (h >= 0)
			v = (v < 0 ? 0 : v * 16) + h;
		else
		{
			if (v >= 0 && v < 0x80 && n < 16)
				codes[n++] = (UWORD)v;
			v = -1;
			if (*s == '\0')
				break;
		}
	}
	return n;
}

int main(void)
{
	LONG args[5] = { 0 };
	struct RDArgs *rd = ReadArgs("SECONDS/N/A,HOLD/K,SWAY/S,PERIOD/N,CYCLE/K", args, NULL);
	if (rd == NULL)
	{
		PrintFault(IoErr(), "HoldKeys");
		return RETURN_FAIL;
	}
	LONG ticks = *(LONG *)args[0] * 50;
	const char *hold = (const char *)args[1];
	BOOL sway = args[2] != 0;
	LONG period = args[3] ? *(LONG *)args[3] : 35;
	if (period < 1)
		period = 1;

	UWORD codes[16], cycle[16];
	int n = parse(hold, codes), nc = parse((const char *)args[4], cycle);

	int rc = RETURN_FAIL;
	struct MsgPort *port = CreateMsgPort();
	if (port != NULL && (io = (struct IOStdReq *)CreateIORequest(port, sizeof(*io))) != NULL)
	{
		if (OpenDevice("input.device", 0, (struct IORequest *)io, 0) == 0)
		{
			for (int i = 0; i < n; ++i)
				key(codes[i], FALSE);
			UWORD side = 0x4F;
			int ci = 0;
			if (sway)
				key(side, FALSE);
			if (nc > 0)
				key(cycle[0], FALSE);
			for (LONG t = 0; t < ticks; ++t)
			{
				if (SetSignal(0, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
					break;
				Delay(1);
				if (sway && (t + 1) % period == 0)
				{
					key(side, TRUE);
					side = side == 0x4F ? 0x4E : 0x4F;
					key(side, FALSE);
				}
				if (nc > 0 && (t + 1) % period == 0)
				{
					key(cycle[ci], TRUE);
					ci = (ci + 1) % nc;
					key(cycle[ci], FALSE);
				}
			}
			if (sway)
				key(side, TRUE);
			if (nc > 0)
				key(cycle[ci], TRUE);
			for (int i = 0; i < n; ++i)
				key(codes[i], TRUE);
			CloseDevice((struct IORequest *)io);
			rc = RETURN_OK;
		}
		DeleteIORequest((struct IORequest *)io);
	}
	if (port != NULL)
		DeleteMsgPort(port);
	FreeArgs(rd);
	return rc;
}
