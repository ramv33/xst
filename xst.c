/* an attempt to keep some xst-only stuff out of the codebase */

int cursorblinkstate = 0;

#define XRESOURCE_LOAD_META(NAME)					\
	if(!XrmGetResource(xrdb, "st." NAME, "st." NAME, &type, &ret))	\
		XrmGetResource(xrdb, "*." NAME, "*." NAME, &type, &ret); \
	if (ret.addr != NULL && !strncmp("String", type, 64))

#define XRESOURCE_LOAD_STRING(NAME, DST)	\
	XRESOURCE_LOAD_META(NAME)		\
		DST = ret.addr;

#define XRESOURCE_LOAD_CHAR(NAME, DST)		\
	XRESOURCE_LOAD_META(NAME)		\
		DST = ret.addr[0];

#define XRESOURCE_LOAD_INTEGER(NAME, DST)		\
	XRESOURCE_LOAD_META(NAME)			\
		DST = strtoul(ret.addr, NULL, 10);

#define XRESOURCE_LOAD_FLOAT(NAME, DST)		\
	XRESOURCE_LOAD_META(NAME)		\
		DST = strtof(ret.addr, NULL);

void
xrdb_load(void)
{
	/* XXX */
	char *xrm;
	char *type;
	XrmDatabase xrdb;
	XrmValue ret;
	Display *dpy;

	if(!(dpy = XOpenDisplay(NULL)))
		die("Can't open display\n");

	XrmInitialize();
	xrm = XResourceManagerString(dpy);

	if (xrm != NULL) {
		xrdb = XrmGetStringDatabase(xrm);

		/* handling colors here without macros to do via loop. */
		int i = 0;
		char loadValue[12] = "";
		for (i = 0; i < 256; i++)
		{
			sprintf(loadValue, "%s%d", "st.color", i);

			if(!XrmGetResource(xrdb, loadValue, loadValue, &type, &ret))
			{
				sprintf(loadValue, "%s%d", "*.color", i);
				if (!XrmGetResource(xrdb, loadValue, loadValue, &type, &ret))
					/* reset if not found (unless in range for defaults). */
					if (i > 15)
						colorname[i] = NULL;
			}

			if (ret.addr != NULL && !strncmp("String", type, 64))
				colorname[i] = ret.addr;
		}

		XRESOURCE_LOAD_STRING("foreground", colorname[defaultfg]);
		XRESOURCE_LOAD_STRING("background", colorname[defaultbg]);
		XRESOURCE_LOAD_STRING("cursorfg", colorname[defaultcs])
		else {
		  // this looks confusing because we are chaining off of the if
		  // in the macro. probably we should be wrapping everything blocks
		  // so this isn't possible...
		  defaultcs = defaultfg;
		}
		XRESOURCE_LOAD_STRING("cursorbg", colorname[defaultrcs])
		else {
		  // see above.
		  defaultrcs = defaultbg;
		}

		XRESOURCE_LOAD_STRING("font", font);
		XRESOURCE_LOAD_STRING("termname", termname);

		XRESOURCE_LOAD_INTEGER("blinktimeout", blinktimeout);
		XRESOURCE_LOAD_INTEGER("bellvolume", bellvolume);
		XRESOURCE_LOAD_INTEGER("disablebold", disablebold);
		XRESOURCE_LOAD_INTEGER("disableitalic", disableitalic);
		XRESOURCE_LOAD_INTEGER("disableroman", disableroman);
		XRESOURCE_LOAD_INTEGER("borderpx", borderpx);
		XRESOURCE_LOAD_INTEGER("borderless", borderless);
		XRESOURCE_LOAD_INTEGER("cursorshape", cursorshape);

		cursorblinkstate = 1; // in case if cursor shape was changed from a blinking one to a non-blinking
		XRESOURCE_LOAD_INTEGER("cursorthickness", cursorthickness);
		XRESOURCE_LOAD_INTEGER("cursorblinkstyle", cursorblinkstyle);
		XRESOURCE_LOAD_INTEGER("cursorblinkontype", cursorblinkontype);

		XRESOURCE_LOAD_INTEGER("scrollrate", scrollrate);

		XRESOURCE_LOAD_FLOAT("cwscale", cwscale);
		XRESOURCE_LOAD_FLOAT("chscale", chscale);
	}
	XFlush(dpy);
}

void
reload(int sig)
{
	xrdb_load();

	/* colors, fonts */
	xloadcols();
	xunloadfonts();
	xloadfonts(font, 0);

	/* pretend the window just got resized */
	cresize(win.w, win.h);

	redraw();

	/* triggers re-render if we're visible. */
	ttywrite("\033[O", 3, 1);

	signal(SIGUSR1, reload);
}

enum motif_wm {
	MWM_HINTS_FUNCTIONS	= (1L << 0),
	MWM_HINTS_DECORATIONS	= (1L << 1),
	MWM_FUNC_ALL		= (1L << 0),
	MWM_FUNC_RESIZE		= (1L << 1),
	MWM_FUNC_MOVE		= (1L << 2),
	MWM_FUNC_MINIMIZE	= (1L << 3),
	MWM_FUNC_MAXIMIZE	= (1L << 4),
	MMW_FUNC_CLOSE		= (1L << 5),
};

/* Motif Window hints */
typedef struct {
	unsigned long	flags;
	unsigned long	functions;
	unsigned long	decorations;
	unsigned long	status;
	long		input_mode;
} Mwm_hints;

void
removewindecorations(void)
{
	Mwm_hints	hints;
	Atom		mwm_hints_property;

	mwm_hints_property = XInternAtom(xw.dpy, "_MOTIF_WM_HINTS", 0);
	hints.flags = MWM_HINTS_DECORATIONS;
	hints.decorations = 0;
	XChangeProperty(xw.dpy, xw.win, mwm_hints_property, mwm_hints_property, 32, PropModeReplace,
			(uchar *)&hints, 5);
}
