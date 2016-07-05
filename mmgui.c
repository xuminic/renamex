
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "iup.h"
#include "libcsoup.h"
#include "rename.h"

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
	Ihandle		*dlg_open;
	char		inst_id[32];

	Ihandle		*list_oldname;
	Ihandle		*list_newname;
	Ihandle		*progress;
	Ihandle		*status;

	Ihandle		*butt_load;
	Ihandle		*butt_run;
	Ihandle		*butt_reset;
	Ihandle		*butt_about;

	Ihandle		*tick_search;
	Ihandle		*entry_pattern;
	Ihandle		*entry_substit;
	Ihandle		*tick_icase;
	Ihandle		*tick_replaced;
	Ihandle		*entry_replaced;
	Ihandle		*radio_simple_match;
	Ihandle		*radio_back_match;
	Ihandle		*radio_suffix;
	Ihandle		*radio_exregex;

	RNOPT		*ropt;
} MMGUI;


static int mmgui_event_resize(Ihandle *ih, int width, int height);
static int mmgui_event_show(Ihandle *ih, int state);
static int mmgui_event_close(Ihandle *ih);
static int mmgui_reset(MMGUI *gui);
static Ihandle *mmgui_search_box(MMGUI *gui);




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

int mmgui_close(void *guiobj)
{
	if (guiobj) {
		IupClose();
		smm_free(guiobj);
	}
	return 0;
}

int mmgui_run(void *guiobj)
{
	MMGUI	*gui = guiobj;
	Ihandle	*vbox_fname, *vbox_panel;
	Ihandle *hbox;

	/* create the controls of left side, the file list panel */
	gui->list_oldname = IupList(NULL);
	IupSetAttribute(gui->list_oldname, "EXPAND", "YES");
	IupSetAttribute(gui->list_oldname, "MULTIPLE", "YES");
	IupSetAttribute(gui->list_oldname, "SCROLLBAR", "YES");
	IupSetAttribute(gui->list_oldname, "DROPFILESTARGET", "YES");
	IupSetAttribute(gui->list_oldname, "ALIGNMENT", "ARIGHT");
	
	gui->list_newname = IupList(NULL);
	IupSetAttribute(gui->list_newname, "EXPAND", "YES");
	IupSetAttribute(gui->list_newname, "MULTIPLE", "YES");
	IupSetAttribute(gui->list_newname, "SCROLLBAR", "YES");
	IupSetAttribute(gui->list_newname, "ALIGNMENT", "ARIGHT");

	gui->progress = IupProgressBar();
	IupSetAttribute(gui->progress, "EXPAND", "HORIZONTAL");
	IupSetAttribute(gui->progress, "DASHED", "YES");
	IupSetAttribute(gui->progress, "SIZE", "x10");
	IupSetAttribute(gui->progress, "VISIBLE", "YES");
	IupSetInt(gui->progress, "MIN", 0);
	IupSetInt(gui->progress, "MAX", 100);
	IupSetInt(gui->progress, "VALUE", 50);

	gui->status = IupLabel("Status is fine");
	IupSetAttribute(gui->status, "EXPAND", "HORIZONTAL");
	
	vbox_fname = IupVbox(gui->list_oldname, gui->list_newname, 
			gui->progress, gui->status, NULL);
	IupSetAttribute(vbox_fname, "NGAP", "8");
	IupSetAttribute(vbox_fname, "NMARGIN", "16x16");

	/* create the controls of right side, the operate panel */
	gui->butt_load = IupButton("Open", NULL);
	IupSetAttribute(gui->butt_load, "SIZE", "60");
	gui->butt_run = IupButton("Run", NULL);
	IupSetAttribute(gui->butt_run, "SIZE", "60");
	gui->butt_reset = IupButton("Clear", NULL);
	IupSetAttribute(gui->butt_reset, "SIZE", "60");
	gui->butt_about = IupButton("About", NULL);
	IupSetAttribute(gui->butt_about, "SIZE", "60");

	vbox_panel = IupVbox(gui->butt_load, gui->butt_run, gui->butt_reset,
			gui->butt_about, NULL);
	IupSetAttribute(vbox_panel, "NGAP", "8");
	IupSetAttribute(vbox_panel, "NMARGIN", "16x16");

	IupAppend(vbox_panel, mmgui_search_box(gui));

	/* create the Open-File dialog initially so it can be popup and hide 
	 * without doing a real destory */
	gui->dlg_open = IupFileDlg();
	IupSetAttribute(gui->dlg_open, "PARENTDIALOG", gui->inst_id);
	IupSetAttribute(gui->dlg_open, "TITLE", "Open Files");

	/* create the dialog window */
	hbox = IupHbox(vbox_fname, vbox_panel, NULL);
	//IupSetHandle("DLG_ICON", IupImageRGBA(128, 128, mmrc_icon_dialog));
	gui->dlg_main = IupDialog(hbox);
	IupSetAttribute(gui->dlg_main, "TITLE", "Rename Extension");
	//IupSetAttribute(gui->dlg_main, "ICON", "DLG_ICON");
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

static int mmgui_notify(RNOPT *ropt, int msg, int cur, void *a, void *opt)
{
	return IUP_DEFAULT;
}

static Ihandle *mmgui_search_box(MMGUI *gui)
{
	Ihandle	*vbox, *vtmp, *title;
	Ihandle	*tt_search, *tt_replace;

	gui->tick_search = IupToggle("Search and Replace", NULL);
	
	tt_search = IupLabel("Search");
	gui->entry_pattern = IupText(NULL);
	IupSetAttribute(gui->entry_pattern, "SIZE", "32");
	tt_replace = IupLabel("Replace");
	gui->entry_substit = IupText(NULL);
	IupSetAttribute(gui->entry_substit, "SIZE", "32");
	vtmp = IupGridBox(tt_search, gui->entry_pattern, tt_replace, gui->entry_substit, NULL);
	IupSetAttribute(vtmp, "ORIENTATION", "HORIZONTAL");
	IupSetAttribute(vtmp, "NUMDIV", "2");

	vbox = IupVbox(vtmp, NULL);
	IupSetAttribute(vbox, "NGAP", "8");
	IupSetAttribute(vbox, "NMARGIN", "16x16");

	gui->tick_icase = IupToggle("Ignore Cases", NULL);
	IupAppend(vbox, gui->tick_icase);

	gui->tick_replaced = IupToggle("Replace All ", NULL);
	gui->entry_replaced = IupText(NULL);
	IupSetAttribute(gui->entry_replaced, "SIZE", "16");
	IupAppend(vbox, IupHbox(gui->tick_replaced, gui->entry_replaced, NULL));

	gui->radio_simple_match = IupToggle("Simple Matching", NULL);
	gui->radio_back_match = IupToggle("Backward Matching", NULL);
	gui->radio_suffix = IupToggle("Suffix Matching", NULL);
	gui->radio_exregex = IupToggle("Regular Expression", NULL);
	title = IupVbox(gui->radio_simple_match, gui->radio_back_match,
			gui->radio_suffix, gui->radio_exregex, NULL);
	IupAppend(vbox, IupRadio(title));

	title = IupVbox(gui->tick_search, vbox, NULL);
	return title;
}

