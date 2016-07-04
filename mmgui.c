
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
	char		inst_id[32];

	Ihandle		*list_oldname;
	Ihandle		*list_newname;

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


static int mmgui_event_resize(Ihandle *ih, int width, int height);
static int mmgui_event_show(Ihandle *ih, int state);
static int mmgui_event_close(Ihandle *ih);
static int mmgui_reset(MMGUI *gui);




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
	Ihandle	*vbox_fname;

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

	vbox_fname = IupVbox(gui->list_oldname, gui->list_newname, NULL);
	IupSetAttribute(vbox_fname, "NGAP", "8");
	IupSetAttribute(vbox_fname, "NMARGIN", "16x16");


	/* create the Saveas-File dialog initially so it can be popup and hide 
	 * without doing a real destory */
	/*gui->dlg_saveas = IupFileDlg();
	IupSetAttribute(gui->dlg_saveas, "PARENTDIALOG", gui->inst_id);
	IupSetAttribute(gui->dlg_saveas, "TITLE", "Save As");
	IupSetAttribute(gui->dlg_saveas, "EXTFILTER", 
			"Image files|*.gif;*.jpg;*.png|All Files|*.*|");*/

	/* create the dialog window */
	//IupSetHandle("DLG_ICON", IupImageRGBA(128, 128, mmrc_icon_dialog));
	gui->dlg_main = IupDialog(vbox_fname);
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

