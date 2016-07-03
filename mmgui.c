
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "iup.h"
#include "libcsoup.h"

#define RENAME_MAIN		"RENAMEGUIOBJ"

#define IUPCOLOR_BLACK		"#000000"
#define IUPCOLOR_SILVER		"#C0C0C0"
#define IUPCOLOR_GRAY		"#808080"
#define IUPCOLOR_WHITE		"#FFFFFF"
#define IUPCOLOR_MAROON		"#800000"
#define IUPCOLOR_RED		"#FF0000"
#define IUPCOLOR_PURPLE		"#800080"
#define IUPCOLOR_FUCHSIA	"#FF00FF"
#define IUPCOLOR_GREEN		"#008000" 
#define IUPCOLOR_LIME		"#00FF00"
#define IUPCOLOR_OLIVE		"#808000"
#define IUPCOLOR_YELLOW		"#FFFF00" 
#define IUPCOLOR_NAVY		"#000080" 
#define IUPCOLOR_BLUE		"#0000FF" 
#define IUPCOLOR_TEAL		"#008080" 
#define IUPCOLOR_AQUA		"#00FFFF" 


typedef	struct	{
	Ihandle		*dlg_main;
	char		inst_id[32];

	Ihandle		*entry_geogcord_a;
	Ihandle		*entry_geogcord_b;

	Ihandle		*combo_zoom;
	int		zoom_now;

	Ihandle		*combo_mhost;
	Ihandle		*combo_mtype;

	Ihandle		*entry_saveas;
	Ihandle		*butt_saveas;
	Ihandle		*dlg_saveas;

	Ihandle		*tick_grid;
	Ihandle		*tick_margin;

	Ihandle		*tick_split;
	Ihandle		*entry_split_width;
	Ihandle		*entry_split_height;
	Ihandle		*entry_split_sep;

	Ihandle		*status;
	Ihandle		*progress;

	Ihandle		*butt_run;
	Ihandle		*butt_reset;
	Ihandle		*butt_about;

	RNOPT		*ropt;
} MMGUI;


void *mmgui_open(RNOPT *ropt, int *argcs, char ***argvs)
{
	MMGUI	*gui;

	IupOpen(argcs, argvs);

	IupSetGlobal("SINGLEINSTANCE", "RENAME");
	if (!IupGetGlobal("SINGLEINSTANCE")) {
		printf("Instance is already running!\n");
		return NULL;
	}

	if ((gui = smm_alloc(sizeof(MMGUI))) == NULL) {
		return NULL;
	}

	/* initialize GUI structure with parameters from command line */
	sprintf(gui->inst_id, "RENAME_%p", gui);
	gui->ropt = ropt;
	return gui;
}

int mmgui_close(void *gui)
{
	if (gui) {
		IupClose();
		smm_free(gui);
	}
	return 0;
}

int mmgui_run(MMGUI *gui)
{
	Ihandle	*vbox;

	/* create an empty vertical box container */
	vbox = IupVbox(NULL);
	IupSetAttribute(vbox, "NGAP", "8");
	IupSetAttribute(vbox, "NMARGIN", "16x16");

	/* appending gui elements */
	gui->entry_geogcord_a = mmgui_create_text(vbox, "Left Top Coordinate");
	gui->entry_geogcord_b = mmgui_create_text(vbox, 
			"Right Bottom Coordinate");
	gui->combo_zoom = mmgui_create_dropdown(vbox, "Zoom");
	gui->combo_mhost = mmgui_create_dropdown(vbox, "Map Server");
	gui->combo_mtype = mmgui_create_dropdown(vbox, "Map Type");
	gui->entry_saveas = mmgui_create_saveas(vbox, "Save As");
	mmgui_create_toggles(vbox, gui);
	gui->status = mmgui_create_progress(vbox, gui);
	IupAppend(vbox, IupFill());
	mmgui_create_button(vbox, gui);

	/* initialize the attribute of the gui elements */


	/* create the Saveas-File dialog initially so it can be popup and hide 
	 * without doing a real destory */
	gui->dlg_saveas = IupFileDlg();
	IupSetAttribute(gui->dlg_saveas, "PARENTDIALOG", gui->inst_id);
	IupSetAttribute(gui->dlg_saveas, "TITLE", "Save As");
	IupSetAttribute(gui->dlg_saveas, "EXTFILTER", 
			"Image files|*.gif;*.jpg;*.png|All Files|*.*|");

	/* create the dialog window */
	IupSetHandle("DLG_ICON", IupImageRGBA(128, 128, mmrc_icon_dialog));
	gui->dlg_main = IupDialog(vbox);
	IupSetAttribute(gui->dlg_main, "TITLE", "Make A Map");
	IupSetAttribute(gui->dlg_main, "ICON", "DLG_ICON");
	IupSetAttribute(gui->dlg_main, "RASTERSIZE", "640");
	IupSetAttribute(gui->dlg_main, RENAME_MAIN, (char*) gui);
	IupSetHandle(gui->inst_id, gui->dlg_main);
	IupSetCallback(gui->dlg_main, "RESIZE_CB", 
			(Icallback) mmgui_event_resize);
	IupSetCallback(gui->dlg_main, "SHOW_CB", 
			(Icallback) mmgui_event_show);
	IupSetCallback(gui->dlg_main, "CLOSE_CB", 
			(Icallback) mmgui_event_close);

	/* show and run the interface */
	IupShow(gui->dlg_main);
	mmgui_reset(gui);
	IupMainLoop();
	return 0;
}

static int mmgui_event_resize(Ihandle *ih, int width, int height)
{
	MMGUI	*gui;

	(void) width; (void) height;
	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	//printf("mmgui_event_resize: %d x %d\n", width, height);
	//printf("mmgui_event_resize: %s\n", IupGetAttribute(ih,"RASTERSIZE"));
	IupSetAttribute(ih, "RASTERSIZE", IupGetAttribute(ih, "RASTERSIZE"));
	return IUP_DEFAULT;
}

static int mmgui_event_show(Ihandle *ih, int state)
{
	MMGUI	*gui;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	switch (state) {
	case IUP_HIDE:
		printf("EVT_SHOW(%d): IUP_HIDE\n", state);
		break;
	case IUP_SHOW:
		printf("EVT_SHOW(%d): IUP_SHOW\n", state);
		break;
	case IUP_RESTORE:
		printf("EVT_SHOW(%d): IUP_RESTORE\n", state);
		break;
	case IUP_MINIMIZE:
		printf("EVT_SHOW(%d): IUP_MINIMIZE\n", state);
		break;
	case IUP_MAXIMIZE:
		printf("EVT_SHOW(%d): IUP_MAXIMIZE\n", state);
		break;
	case IUP_CLOSE:
		printf("EVT_SHOW(%d): IUP_CLOSE\n", state);
		break;
	default:
		printf("EVT_SHOW(%d): unknown\n", state);
		break;
	}
	return IUP_DEFAULT;
}

static int mmgui_event_close(Ihandle *ih)
{
	MMGUI	*gui;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	return IUP_DEFAULT;
}

static int mmgui_reset(MMGUI *gui)
{
	return IUP_DEFAULT;
}

static int mmgui_notify(MAPOPT *mopt, int msg, int cur, int max, void *opt)
{
	return IUP_DEFAULT;
}

static Ihandle *mmgui_create_text(Ihandle *vbox, char *label)
{
	Ihandle	*title, *text, *hbox;

	title = IupLabel(label);
	IupSetAttribute(title, "SIZE", "120");
	IupSetAttribute(title, "ALIGNMENT", "ACENTER");

	text = IupText(NULL);
	IupSetAttribute(text, "EXPAND", "HORIZONTAL");

	hbox = IupHbox(title, text, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ABOTTOM");
	IupSetAttribute(hbox, "NGAP", "4");

	IupAppend(vbox, hbox);
	return text;
}

static Ihandle *mmgui_create_dropdown(Ihandle *vbox, char *label)
{
	Ihandle	*title, *list, *hbox;

	title = IupLabel(label);
	IupSetAttribute(title, "SIZE", "120");
	IupSetAttribute(title, "ALIGNMENT", "ACENTER");

	list = IupList(NULL);
	IupSetAttribute(list, "DROPDOWN", "YES");
	IupSetAttribute(list, "EXPAND", "HORIZONTAL");
	IupSetAttribute(list, "VISIBLEITEMS", "10");

	hbox = IupHbox(title, list, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ABOTTOM");
	IupSetAttribute(hbox, "NGAP", "4");

	IupAppend(vbox, hbox);
	return list;
}

static Ihandle *mmgui_create_saveas(Ihandle *vbox, char *label)
{
	Ihandle	*title, *text, *butt, *hbox;

	title = IupLabel(label);
	IupSetAttribute(title, "SIZE", "120");
	IupSetAttribute(title, "ALIGNMENT", "ACENTER");

	text = IupText(NULL);
	IupSetAttribute(text, "EXPAND", "HORIZONTAL");

	butt = IupButton("Open", NULL);
	IupSetAttributeHandle(butt, "IMAGE", 
			IupImageRGBA(16, 16, mmrc_icon_saveas));
	IupSetCallback(butt, "ACTION", (Icallback) mmgui_btn_saveas_event);

	hbox = IupHbox(title, text, butt, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ABOTTOM");
	IupSetAttribute(hbox, "NGAP", "4");

	IupAppend(vbox, hbox);
	return text;
}

static Ihandle *mmgui_create_toggles(Ihandle *vbox, MMGUI *gui)
{
	Ihandle	*title, *grid, *hbox, *hinput;

	title = IupLabel("");
	IupSetAttribute(title, "SIZE", "120");
	IupSetAttribute(title, "ALIGNMENT", "ACENTER");

	gui->tick_grid   = IupToggle("Hide Grid", NULL);
	IupSetAttribute(gui->tick_grid, "EXPAND", "HORIZONTAL");
	gui->tick_margin = IupToggle("No Margin", NULL);
	IupSetAttribute(gui->tick_margin, "EXPAND", "HORIZONTAL");

	gui->tick_split  = IupToggle("Split Map", NULL);
	IupSetAttribute(gui->tick_split, "EXPAND", "HORIZONTAL");

	gui->entry_split_width = IupText(NULL);
	IupSetAttribute(gui->entry_split_width, "SIZE", "32");
	gui->entry_split_height = IupText(NULL);
	IupSetAttribute(gui->entry_split_height, "SIZE", "32");
	gui->entry_split_sep = IupLabel("x");
	hinput = IupHbox(gui->entry_split_width, gui->entry_split_sep,
			gui->entry_split_height, NULL);
	IupSetAttribute(hinput, "ALIGNMENT", "ACENTER");
	IupSetAttribute(hinput, "NGAP", "4");

	grid = IupGridBox(gui->tick_grid, gui->tick_margin, gui->tick_split, 
			hinput, NULL);
	IupSetAttribute(grid, "ORIENTATION", "HORIZONTAL");
	IupSetAttribute(grid, "NUMDIV", "2");
	IupSetAttribute(grid, "EXPAND", "HORIZONTAL");
	IupSetAttribute(grid, "GAPLIN", "4");

	hbox = IupHbox(title, grid, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ABOTTOM");
	IupSetAttribute(hbox, "NGAP", "4");

	IupAppend(vbox, hbox);
	return grid;
}

static int mmgui_map_split_show(MMGUI *gui, int state)
{
	if (state) {	/* 1 if the toggle's state was shifted to on */
		IupSetAttribute(gui->entry_split_width,  "VISIBLE", "YES");
		IupSetAttribute(gui->entry_split_height, "VISIBLE", "YES");
		IupSetAttribute(gui->entry_split_sep,    "VISIBLE", "YES");
	} else {	/* 0 if it was shifted to off */
		IupSetAttribute(gui->entry_split_width,  "VISIBLE", "NO");
		IupSetAttribute(gui->entry_split_height, "VISIBLE", "NO");
		IupSetAttribute(gui->entry_split_sep,    "VISIBLE", "NO");
	}
	return IUP_DEFAULT;
}

static Ihandle *mmgui_create_progress(Ihandle *vbox, MMGUI *gui)
{
	Ihandle	*title, *hbox;

	title = IupLabel("");
	IupSetAttribute(title, "SIZE", "120");
	IupSetAttribute(title, "ALIGNMENT", "ACENTER");

	gui->progress = IupProgressBar();
	IupSetAttribute(gui->progress, "EXPAND", "HORIZONTAL");
	IupSetAttribute(gui->progress, "DASHED", "YES");
	IupSetAttribute(gui->progress, "SIZE", "x10");

	hbox = IupHbox(title, gui->progress, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ABOTTOM");
	IupSetAttribute(hbox, "NGAP", "4");

	IupAppend(vbox, IupLabel(""));
	IupAppend(vbox, hbox);
	return title;
}

static Ihandle *mmgui_create_button(Ihandle *vbox, MMGUI *gui)
{
	Ihandle	*hbox;

	gui->butt_run = IupButton("Run", NULL);
	IupSetAttribute(gui->butt_run, "SIZE", "40");
	gui->butt_reset = IupButton("Clear", NULL);
	IupSetAttribute(gui->butt_reset, "SIZE", "40");
	gui->butt_about = IupButton("About", NULL);
	IupSetAttribute(gui->butt_about, "SIZE", "40");
	hbox = IupHbox(IupFill(), gui->butt_run, gui->butt_reset, 
			gui->butt_about, NULL);
	IupSetAttribute(hbox, "NGAP", "4");

	IupAppend(vbox, hbox);
	return hbox;
}

