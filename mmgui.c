
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "iup.h"
#include "libcsoup.h"
#include "rename.h"
#include "mmrc_icon_dialog.h"
#include "mmrc_icon_warning.h"

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
	int		magic_width;	/* the width except the list panel */

	/* list panel */
	Ihandle		*list_oldname;
	Ihandle		*list_preview;
	Ihandle		*zbox_extent;
	Ihandle		*status;
	int		fileno;		/* file's number in the list */

	/* list panel extension for search and replace */
	Ihandle		*lable_pattern;
	Ihandle		*entry_pattern;
	Ihandle		*lable_substit;
	Ihandle		*entry_substit;
	Ihandle		*progress;
	int		progbar_min;
	int		progbar_max;
	int		progbar_now;

	/* button box */
	Ihandle		*butt_open;
	Ihandle		*butt_del;
	Ihandle		*butt_run;
	Ihandle		*butt_about;

	/* option box */
	Ihandle		*tick_prefix;
	Ihandle		*entry_prefix;
	Ihandle		*tick_suffix;
	Ihandle		*entry_suffix;
	Ihandle		*tick_lowercase;
	Ihandle		*tick_uppercase;
	Ihandle		*tick_search;

	/* search and replace box */
	Ihandle		*tick_icase;
	Ihandle		*tick_replaced;
	Ihandle		*entry_replaced;
	int		state_replaced;
	Ihandle		*radio_simple_match;
	Ihandle		*radio_back_match;
	Ihandle		*radio_extension;
	Ihandle		*radio_exregex;

	RNOPT		*ropt;
} MMGUI;


static int mmgui_reset(MMGUI *gui);
static int mmgui_event_resize(Ihandle *ih, int width, int height);
static int mmgui_event_timer(Ihandle *ih);
static int mmgui_event_update(Ihandle *ih, ...);
static int mmgui_notify(void *opobj, int msg, int v, void *a1, void *a2);
static Ihandle *mmgui_fnlist_box(MMGUI *gui);
static int mmgui_fnlist_event_dropfiles(Ihandle *, char *, int,int,int);
static int mmgui_fnlist_event_multi_select(Ihandle *ih, char *value);
static int mmgui_fnlist_event_moused(Ihandle *ih, int, int, int, int, char *);
static int mmgui_fnlist_event_dblclick(Ihandle *ih, int item, char *text);
static int mmgui_fnlist_append(MMGUI *gui, char *fname);
static int mmgui_fnlist_remove(MMGUI *gui, int idx);
static int mmgui_fnlist_rename(MMGUI *gui, int idx);
static int mmgui_fnlist_status(MMGUI *gui, char *color, char *fmt, ...);
static int mmgui_fnlist_update_preview(MMGUI *gui, int action);
static Ihandle *mmgui_button_box(MMGUI *gui);
static int mmgui_button_event_load(Ihandle *ih);
static int mmgui_button_event_delete(Ihandle *ih);
static int mmgui_button_event_rename(Ihandle *ih);
static int mmgui_button_event_about(Ihandle *ih);
static int mmgui_button_status_update(MMGUI *gui, int action);
static Ihandle *mmgui_option_box(MMGUI *gui);
static int mmgui_option_reset(MMGUI *gui);
static int mmgui_option_event_tick_prefix(Ihandle* ih, int state);
static int mmgui_option_event_tick_suffix(Ihandle* ih, int state);
static int mmgui_option_event_tick_lowercase(Ihandle* ih, int state);
static int mmgui_option_event_tick_uppercase(Ihandle* ih, int state);
static int mmgui_option_event_tick_search(Ihandle* ih, int state);
static int mmgui_option_collection(MMGUI *gui);
static int mmgui_option_free(MMGUI *gui);
static Ihandle *mmgui_search_box(MMGUI *gui);
static Ihandle *mmgui_search_strip(MMGUI *gui);
static int mmgui_search_reset(MMGUI *gui);
static int mmgui_search_box_show(MMGUI *gui, int state);
static int mmgui_search_strip_show(MMGUI *gui, int state);
static int mmgui_search_event_tick_replaced(Ihandle* ih, int state);

static int mmgui_conflict_popup(MMGUI *gui, char *fname);
static int mmgui_batch_popup(MMGUI *gui);
static int mmgui_rename_exec(MMGUI *gui, int i, char *dstname, char *srcname);
static char *IupTool_FileDlgExtract(char *dfn, char **sp);
static int IupTool_FileDlgCounting(char *value);


void *mmgui_open(RNOPT *ropt, int *argcs, char ***argvs)
{
	MMGUI	*gui;

	IupOpen(argcs, argvs);
	IupImageLibOpen();

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

int mmgui_run(void *guiobj, int argc, char **argv)
{
	MMGUI	*gui = guiobj;
	Ihandle	*vbox_fname, *vbox_panel, *hbox, *timer;
	int	i;

	/* create the controls of left side, the file list panel */
	vbox_fname = mmgui_fnlist_box(gui);

	/* create the controls of right side, the operate panel */
	vbox_panel = IupVbox(NULL);
	IupAppend(vbox_panel, mmgui_button_box(gui));
	IupAppend(vbox_panel, mmgui_option_box(gui));
	IupAppend(vbox_panel, mmgui_search_box(gui));

	/* create the dialog window */
	hbox = IupHbox(vbox_fname, vbox_panel, NULL);
	IupSetAttribute(hbox, "NGAP", "16");
	IupSetAttribute(hbox, "NMARGIN", "16x16");
	
	IupSetHandle("DLG_ICON", IupImageRGBA(128, 128, mmrc_icon_dialog));
	gui->dlg_main = IupDialog(hbox);
	IupSetAttribute(gui->dlg_main, "TITLE", "Rename Extension");
	IupSetAttribute(gui->dlg_main, "ICON", "DLG_ICON");
	IupSetAttribute(gui->dlg_main, "RASTERSIZE", "800");
	IupSetAttribute(gui->dlg_main, RENAME_MAIN, (char*) gui);
	IupSetHandle(gui->inst_id, gui->dlg_main);
	IupSetCallback(gui->dlg_main, "RESIZE_CB", 
			(Icallback) mmgui_event_resize);

	/* create the Open-File dialog initially so it can be popup and hide 
	 * without doing a real destory */
	gui->dlg_open = IupFileDlg();
	IupSetAttribute(gui->dlg_open, "PARENTDIALOG", gui->inst_id);
	IupSetAttribute(gui->dlg_open, "TITLE", "Open Files");
	IupSetAttribute(gui->dlg_open, "MULTIPLEFILES", "YES");

	/* show and run the interface */
	mmgui_reset(gui);
	gui->ropt->notify = mmgui_notify;
	IupShow(gui->dlg_main);

	/* appending filename list from the command line */
	for (i = 0; i < argc; i++) {
		mmgui_fnlist_append(gui, argv[i]);
	}

	/* starting the timer to monitor the progress bar 
	 * Note that the 'timer' object trigger the timeout event, 
	 * not the 'dlg_main' */
	timer = IupTimer();
	IupSetAttribute(timer, "TIME", "50");	/* in ms */
	IupSetAttribute(timer, RENAME_MAIN, (char*) gui);
	IupSetCallback(timer, "ACTION_CB", (Icallback) mmgui_event_timer);
	IupSetAttribute(timer, "RUN", "YES");

	IupMainLoop();
	return 0;
}

static int mmgui_reset(MMGUI *gui)
{
	mmgui_option_reset(gui);
	mmgui_search_reset(gui);
	mmgui_button_status_update(gui, 0);
	return IUP_DEFAULT;
}

static int mmgui_event_resize(Ihandle *ih, int width, int height)
{
	MMGUI	*gui;
	char	buf[32], *value;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	//printf("mmgui_event_resize: %d x %d\n", width, height);
	//printf("mmgui_event_resize: %s\n", IupGetAttribute(ih,"RASTERSIZE"));
	IupSetAttribute(ih, "RASTERSIZE", IupGetAttribute(ih, "RASTERSIZE"));

	/* resize the list panel to make sure it won't growth unexpectly */
	value = IupGetAttribute(ih, "CLIENTSIZE");
	width = (int) strtol(value, NULL, 10);
	if (gui->magic_width == 0) {
		value = IupGetAttribute(gui->list_oldname, "RASTERSIZE");
		height = (int) strtol(value, NULL, 10);
		gui->magic_width = width - height;
		//printf("mmgui_event_resize: %d\n", gui->magic_width);

		/* normalize the buttons against the option box */
		value = IupGetAttribute(gui->tick_prefix, "RASTERSIZE");
		height = (int) strtol(value, NULL, 10);
		value = IupGetAttribute(gui->entry_prefix, "RASTERSIZE");
		height += (int) strtol(value, NULL, 10);
		sprintf(buf, "%d", height);
		//printf("mmgui_event_resize: %d\n", height);
		IupSetAttribute(gui->butt_open, "RASTERSIZE", buf);
		IupSetAttribute(gui->butt_del, "RASTERSIZE", buf);
		IupSetAttribute(gui->butt_run, "RASTERSIZE", buf);
		IupSetAttribute(gui->butt_about, "RASTERSIZE", buf);
	}

	sprintf(buf, "%d", width - gui->magic_width);
	IupSetAttribute(gui->list_oldname, "RASTERSIZE", buf);
	IupSetAttribute(gui->list_preview, "RASTERSIZE", buf);
	return IUP_DEFAULT;
}

static int mmgui_event_timer(Ihandle *ih)
{
	MMGUI	*gui;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}

	if (gui->progbar_min < gui->progbar_max) {
		IupSetInt(gui->progress, "VALUE", gui->progbar_now);
	}
	return IUP_DEFAULT;
}

static int mmgui_event_update(Ihandle *ih, ...)
{
	MMGUI	*gui;
	int	action;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}

	action = mmgui_option_collection(gui);
	mmgui_fnlist_update_preview(gui, action);
	mmgui_button_status_update(gui, action);
	return mmgui_option_free(gui);
}

static int mmgui_notify(void *opobj, int msg, int v, void *a1, void *a2)
{
	RNOPT	*opt = opobj;

	(void) v; (void) a2;

	switch (msg) {
	case RNM_MSG_PROMPT:
		if (!mmgui_conflict_popup(opt->gui, (char*) a1)) {
			return RNM_ERR_SKIP;
		}
		return RNM_ERR_NONE;
	}
	return RNM_ERR_EVENT;	/* pass through */
}

/****************************************************************************
 * file name list box
 ****************************************************************************/
static Ihandle *mmgui_fnlist_box(MMGUI *gui)
{
	Ihandle	*vbox;

	/* create the controls of left side, the file list panel */
	gui->list_oldname = IupList(NULL);
	IupSetAttribute(gui->list_oldname, "EXPAND", "YES");
	IupSetAttribute(gui->list_oldname, "MULTIPLE", "YES");
	IupSetAttribute(gui->list_oldname, "SCROLLBAR", "YES");
	IupSetAttribute(gui->list_oldname, "DROPFILESTARGET", "YES");
	IupSetAttribute(gui->list_oldname, "ALIGNMENT", "ARIGHT");
	IupSetAttribute(gui->list_oldname, "CANFOCUS", "YES");
	IupSetCallback(gui->list_oldname, "DROPFILES_CB",
			(Icallback) mmgui_fnlist_event_dropfiles);
	IupSetCallback(gui->list_oldname, "MULTISELECT_CB",
			(Icallback) mmgui_fnlist_event_multi_select);
	IupSetCallback(gui->list_oldname, "BUTTON_CB",
			(Icallback) mmgui_fnlist_event_moused);
	IupSetCallback(gui->list_oldname, "DBLCLICK_CB",
			(Icallback) mmgui_fnlist_event_dblclick);
	
	gui->list_preview = IupList(NULL);
	IupSetAttribute(gui->list_preview, "EXPAND", "YES");
	IupSetAttribute(gui->list_preview, "SCROLLBAR", "YES");
	IupSetAttribute(gui->list_preview, "ALIGNMENT", "ARIGHT");
	IupSetAttribute(gui->list_preview, "CANFOCUS", "NO");
	IupSetAttribute(gui->list_preview, "FGCOLOR", IUPCOLOR_BLUE);

	vbox = IupVbox(gui->list_oldname, gui->list_preview, NULL);
	IupSetAttribute(vbox, "NGAP", "4");

	gui->progress = IupProgressBar();
	IupSetAttribute(gui->progress, "EXPAND", "HORIZONTAL");
	IupSetAttribute(gui->progress, "DASHED", "YES");
	IupSetAttribute(gui->progress, "SIZE", "x10");
	IupSetAttribute(gui->progress, "VISIBLE", "NO");
	IupSetInt(gui->progress, "MIN", 0);
	IupSetInt(gui->progress, "MAX", 100);
	IupSetInt(gui->progress, "VALUE", 50);
	gui->zbox_extent = IupZbox(mmgui_search_strip(gui), 
			gui->progress, NULL);

	gui->status = IupLabel("");
	IupSetAttribute(gui->status, "EXPAND", "HORIZONTAL");
	
	vbox = IupVbox(vbox, gui->zbox_extent, gui->status, NULL);
	IupSetAttribute(vbox, "NGAP", "8");
	return vbox;
}

static int mmgui_fnlist_event_dropfiles(Ihandle *ih,
		char* filename, int num, int x, int y)
{
	MMGUI	*gui;

	(void) num; (void) x; (void) y;
	//printf("mmgui_fnlist_event_dropfiles: fname=%s number=%d %dx%d\n",
	//		filename, num, x, y);

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	mmgui_fnlist_append(gui, filename);
	return mmgui_event_update(ih);
}

static int mmgui_fnlist_event_multi_select(Ihandle *ih, char *value)
{
	MMGUI	*gui;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	
	/* we don't use the pass-in value */
	/* printf("mmgui_fnlist_event_multi_select: %s\n", value); */

	value = IupGetAttribute(gui->list_oldname, "VALUE");
	//printf("List value=%s\n", value);
	mmgui_fnlist_status(gui, IUPCOLOR_BLACK, "%d File Selected", 
			IupTool_FileDlgCounting(value));

	/* 20160718 Any write to list_oldname will trigger this event
	 * right in the middle of the process so if called mmgui_event_update
	 * here it will read a broken list, which offsets/upsets every items.
	 * It seems not very urgent in this event to update the preview
	 * so I think having it removed should be safe. */
	/* 20160719 Nope, actually the mouse event did the update!!
	 * We'd better do the update seperate to the preview update 
	 * so it won't look broken while doing the keyboard movement. */
	mmgui_button_status_update(gui, mmgui_option_collection(gui));
	return mmgui_option_free(gui);
}

static int mmgui_fnlist_event_moused(Ihandle *ih,
		int button, int pressed, int x, int y, char *status)
{
	MMGUI	*gui;

	(void)x; (void)y; (void)status; /* stop compiler complains */

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	if (pressed) {	/* only act when button is released */
		return IUP_DEFAULT;
	}
	/* deselect every thing if the right button was released */
	//printf("Moused: %d\n", button);
	if (button == IUP_BUTTON3) {
		IupSetAttribute(gui->list_oldname, "VALUE", "");
		mmgui_fnlist_status(gui, IUPCOLOR_BLACK, "0 File Selected");
	}
	return mmgui_event_update(ih);
}

static int mmgui_fnlist_event_dblclick(Ihandle *ih, int item, char *text)
{
	MMGUI	*gui;
	Ihandle	*entry, *shadow, *vbox, *hbox;
	Ihandle	*butt_cancel, *butt_yes;
	char	*srcname, *newpath, *tmp;
	int	rc;

	int mmgui_fnlist_event_dblclick_button(Ihandle* ih)
	{
		IupSetAttribute(ih, "FCDLGCLICK", "YES");
		return IUP_CLOSE;
	}

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}

	/* the button box in the bottom of the dialog window */
	butt_cancel = IupButton("Cancel", NULL);
	IupSetAttribute(butt_cancel, "IMAGE", "IUP_ActionCancel");
	IupSetAttribute(butt_cancel, "PADDING", "8");
	IupSetCallback(butt_cancel, "ACTION",
			(Icallback) mmgui_fnlist_event_dblclick_button);
	butt_yes = IupButton("Rename", NULL);
	IupSetAttribute(butt_yes, "IMAGE", "IUP_ActionOk");
	IupSetAttribute(butt_yes, "PADDING", "8");
	IupSetCallback(butt_yes, "ACTION",
			(Icallback) mmgui_fnlist_event_dblclick_button);

	/* pack the buttons first because we want them normalized */
	hbox = IupHbox(butt_cancel, butt_yes, NULL);
	IupSetAttribute(hbox, "NGAP", "4");
	IupSetAttribute(hbox, "NORMALIZESIZE", "HORIZONTAL");

	shadow = IupLabel("");	/* padding the left side of the box */
	IupSetAttribute(shadow, "SIZE", "100");
	IupSetAttribute(shadow, "VISIBLE", "NO");

	hbox = IupHbox(shadow, IupFill(), hbox, NULL); 
	
	/* only display the basename not the full path */
	srcname = csc_path_basename(text, NULL, 0);

	entry = IupText(NULL);
	IupSetAttribute(entry, "EXPAND", "HORIZONTAL");
	IupSetAttribute(entry, "VALUE", srcname);

	/* use this invisible control to keep the text control long enough */
	shadow = IupLabel(srcname);
	IupSetAttribute(shadow, "PADDING", "10");
	IupSetAttribute(shadow, "VISIBLE", "NO");

	vbox = IupVbox(entry, shadow, hbox, NULL);
	IupSetAttribute(vbox, "NGAP", "4x4");
	IupSetAttribute(vbox, "NMARGIN", "16x16");

	hbox = IupDialog(vbox);	/* never mind the old hbox */
	IupSetAttribute(hbox, "TITLE", "Direct Rename");
	IupSetAttribute(hbox, "ICON", "DLG_ICON");
	IupSetAttribute(hbox, "MAXBOX", "NO");
	IupSetAttribute(hbox, "MINBOX", "NO");
	IupSetAttribute(hbox, "HIDETASKBAR", "YES");
	IupSetAttribute(hbox, "PARENTDIALOG", gui->inst_id);
	IupPopup(hbox, IUP_CENTER, IUP_CENTER);

	if (IupGetAttribute(butt_yes, "FCDLGCLICK") != NULL) {
		srcname = IupGetAttribute(entry, "VALUE");
		newpath = csc_strcpy_alloc(text, strlen(srcname));
		tmp = csc_path_basename(newpath, NULL, 0);
		strcpy(tmp, srcname);

		rename_status_clean(gui->ropt);
		rc = mmgui_rename_exec(gui, item, newpath, text);
		if (rc == RNM_ERR_NONE) {
			mmgui_event_update(ih);	/* update the preview */
		}
		smm_free(newpath);
	}
	return IUP_DEFAULT;
}

static int mmgui_fnlist_append(MMGUI *gui, char *fname)
{
	if (fname) {
		IupSetStrAttributeId(gui->list_oldname, "",  
				++(gui->fileno), fname);
		mmgui_fnlist_status(gui, IUPCOLOR_BLACK,
				"%d Files in the Queue", gui->fileno);
	}
	return IUP_DEFAULT;
}

static int mmgui_fnlist_remove(MMGUI *gui, int idx)
{
	char	*fname;

	fname = IupGetAttributeId(gui->list_oldname, "",  idx);
	if (fname) {
		IupSetInt(gui->list_oldname, "REMOVEITEM", idx);
		gui->fileno--;
	}
	fname = IupGetAttributeId(gui->list_preview, "",  idx);
	if (fname) {
		IupSetInt(gui->list_preview, "REMOVEITEM", idx);
	}
	mmgui_fnlist_status(gui, IUPCOLOR_BLACK, 
			"%d Files in the Queue", gui->fileno);
	return IUP_DEFAULT;
}

static int mmgui_fnlist_rename(MMGUI *gui, int idx)
{
	char	*srcname, *dstname;

	srcname = IupGetAttributeId(gui->list_oldname, "", idx);
	if (srcname == NULL) {
		printf("mmgui_fnlist_rename: out of range. [%d]\n", idx);
		return IUP_DEFAULT;
	}

	dstname = IupGetAttributeId(gui->list_preview, "", idx);
	if (dstname == NULL) {
		printf("mmgui_fnlist_rename: lost preview.\n");
		return IUP_DEFAULT;
	}

	return mmgui_rename_exec(gui, idx, dstname, srcname);
	/*if (rename_open_buffer(gui->ropt, srcname) != RNM_ERR_NONE) {
		printf("mmgui_fnlist_rename: can not rename\n");
		return IUP_DEFAULT;
	}
	return mmgui_rename_exec(gui, idx, gui->ropt->buffer, srcname);*/
}

static int mmgui_fnlist_status(MMGUI *gui, char *color, char *fmt, ...)
{
	static	char	value[128];
	va_list	ap;

	if (color) {
		IupSetAttribute(gui->status, "FGCOLOR", color);
	}
	if (fmt) {
		va_start(ap, fmt);
		SMM_VSNPRINT(value, sizeof(value), fmt, ap);
		va_end(ap);
		//puts(value);
		IupSetAttribute(gui->status, "TITLE", value);
	}
	return IUP_DEFAULT;
}

static int mmgui_fnlist_update_preview(MMGUI *gui, int action)
{
	RNOPT	*opt = gui->ropt;
	char	*fname;
	int	i;

	if (action == 0) {
		/* action is not ready so wipe out the preview */
		while (IupGetAttributeId(gui->list_preview, "", 1)) {
			IupSetInt(gui->list_preview, "REMOVEITEM", 1);
		}
		return IUP_DEFAULT;
	}
	
	for (i = 1; i <= gui->fileno; i++) {
		fname = IupGetAttributeId(gui->list_oldname, "",  i);
		if (fname == NULL) {
			break;
		}
		smm_codepage_set(65001);   /* set the codepage to utf-8 */
		if (rename_open_buffer(gui->ropt, fname) == RNM_ERR_NONE) {
			IupSetStrAttributeId(gui->list_preview, 
					"",  i, opt->buffer);
		} else {
			IupSetStrAttributeId(gui->list_preview, 
					"",  i, fname);
		}
		smm_codepage_reset();
	}
	return IUP_DEFAULT;
}

/****************************************************************************
 * Button box group
 ****************************************************************************/
static Ihandle *mmgui_button_box(MMGUI *gui)
{
	Ihandle	*vbox;

	gui->butt_open = IupButton("Open", NULL);
	IupSetAttribute(gui->butt_open, "IMAGE", "IUP_FileOpen");
	IupSetAttribute(gui->butt_open, "EXPAND", "HORIZONTALFREE");
	gui->butt_del = IupButton("Delete", NULL);
	IupSetAttribute(gui->butt_del, "IMAGE", "IUP_EditErase");
	IupSetAttribute(gui->butt_del, "EXPAND", "HORIZONTALFREE");
	gui->butt_run = IupButton("Rename", NULL);
	IupSetAttribute(gui->butt_run, "IMAGE", "IUP_ActionOk");
	IupSetAttribute(gui->butt_run, "EXPAND", "HORIZONTALFREE");
	gui->butt_about = IupButton("About", NULL);
	IupSetAttribute(gui->butt_about, "IMAGE", "IUP_MessageInfo");
	IupSetAttribute(gui->butt_about, "EXPAND", "HORIZONTALFREE");

	vbox = IupVbox(gui->butt_open, gui->butt_del, gui->butt_run, 
			gui->butt_about, NULL);
	IupSetAttribute(vbox, "NGAP", "4");

	IupSetCallback(gui->butt_open, "ACTION",
			(Icallback) mmgui_button_event_load);
	IupSetCallback(gui->butt_del, "ACTION",
			(Icallback) mmgui_button_event_delete);
	IupSetCallback(gui->butt_run, "ACTION",
			(Icallback) mmgui_button_event_rename);
	IupSetCallback(gui->butt_about, "ACTION",
			(Icallback) mmgui_button_event_about);
	return vbox;
}

static int mmgui_button_event_load(Ihandle *ih)
{
	MMGUI	*gui;
	char	*fname, *dlgrd, *sp = NULL;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}

	IupPopup(gui->dlg_open, IUP_CENTERPARENT, IUP_CENTERPARENT);
	if (IupGetInt(gui->dlg_open, "STATUS") < 0) {
		return IUP_DEFAULT;	/* cancelled */
	}

	/*printf("Last  DIRECTORY: %s\n", 
			IupGetAttribute(gui->dlg_open, "DIRECTORY"));*/
	dlgrd = IupGetAttribute(gui->dlg_open, "VALUE");
	//printf("Open File VALUE: %s\n", dlgrd);
	while ((fname = IupTool_FileDlgExtract(dlgrd, &sp)) != NULL) {
		mmgui_fnlist_append(gui, fname);
		/* IupList control seems save a copy of its content.
		 * The 'fname' can not be retrieved by IupGetAttribute() */
		smm_free(fname);
	}
	return mmgui_event_update(ih);
}

static int mmgui_button_event_delete(Ihandle *ih)
{
	MMGUI	*gui;
	char	*value;
	int	i;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}

	while (1) {
		value = IupGetAttribute(gui->list_oldname, "VALUE");
		//printf("mmgui_button_event_delete: %s\n", value);
		for (i = 0; value[i]; i++) {
			if (value[i] == '+') {
				mmgui_fnlist_remove(gui, i+1);
				break;
			}
		}
		if (!value[i]) {
			break;
		}
	}
	return mmgui_event_update(ih);
}

static int mmgui_button_event_rename(Ihandle *ih)
{
	MMGUI	*gui;
	char	*value, *vflag;
	int	i;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}

	/* collecting options and parameters */
	if (mmgui_option_collection(gui) == 0) {	/* option not ready */
		return mmgui_option_free(gui);
	}
	//rename_option_dump(gui->ropt);
	rename_status_clean(gui->ropt);

	/* run rename one by one */
	IupSetAttribute(gui->zbox_extent, "VALUEPOS", "1");
	mmgui_search_strip_show(gui, 0);	
	IupSetAttribute(gui->progress, "VISIBLE", "YES");
	IupSetInt(gui->progress, "MAX", gui->fileno);
	IupSetInt(gui->progress, "MIN", 0);

	/* notify the timer the progress is on the way */
	gui->progbar_min = 0;
	gui->progbar_max = gui->fileno;

	value = IupGetAttribute(gui->list_oldname, "VALUE");
	vflag = strchr(value, '+');
	for (i = 0; i < gui->fileno; i++) {
		if (vflag == NULL) {
			mmgui_fnlist_rename(gui, i+1);
		} else if (value[i] == '+') {
			mmgui_fnlist_rename(gui, i+1);
		}
		gui->progbar_now = i + 1; 	/* notify the timer */
	}
	mmgui_batch_popup(gui);

	/* notify the timer the progress is stopped */
	gui->progbar_min = gui->progbar_max = 0;
	IupSetInt(gui->progress, "VALUE", 0);
	IupSetAttribute(gui->progress, "VISIBLE", "NO");

	/* you can't simply turn on the search strip because it depends
	 * on the status of option box */
	value = IupGetAttribute(gui->tick_search, "VALUE");
	if (!strcmp(value, "ON")) {
		mmgui_search_strip_show(gui, 1);
	}
	IupSetAttribute(gui->zbox_extent, "VALUEPOS", "0");

	mmgui_fnlist_status(gui, IUPCOLOR_BLACK,
			"%d Files renamed", gui->ropt->st_success);
	return mmgui_option_free(gui);
}

static int mmgui_button_event_about(Ihandle *ih)
{
	MMGUI	*gui;
	Ihandle	*widget, *vbox;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}

	/* show the icon */
	widget = IupLabel(NULL);
	IupSetAttribute(widget, "IMAGE", "DLG_ICON");

	vbox = IupVbox(widget, NULL);
	IupSetAttribute(vbox, "NGAP", "8");
	IupSetAttribute(vbox, "NMARGIN", "16x16");
	IupSetAttribute(vbox, "ALIGNMENT", "ACENTER");

	/* show name and the version */
	widget = IupLabel(help_version);
	IupSetAttribute(widget, "ALIGNMENT", "ACENTER:ACENTER");
	IupSetAttribute(widget, "FONTSIZE", "20");
	IupSetAttribute(widget, "FONTSTYLE", "Bold");
	IupAppend(vbox, widget);

	/* show the simple description */
	widget = IupLabel(help_descript);
	IupSetAttribute(widget, "ALIGNMENT", "ACENTER:ACENTER");
	IupAppend(vbox, widget);

	/* show the credits */
	widget = IupLabel(help_credits);
	IupSetAttribute(widget, "ALIGNMENT", "ACENTER:ACENTER");
	IupAppend(vbox, widget);

	ih = IupDialog(vbox);
	IupSetAttribute(ih, "TITLE", "About");
	IupSetAttribute(ih, "ICON", "DLG_ICON");
	IupSetAttribute(ih, "RESIZE", "NO");
	IupSetAttribute(ih, "MAXBOX", "NO");
	IupSetAttribute(ih, "MINBOX", "NO");
	IupSetAttribute(ih, "HIDETASKBAR", "YES");
	IupSetAttribute(ih, "PARENTDIALOG", gui->inst_id);
	IupPopup(ih, IUP_CENTER, IUP_CENTER);
	return IUP_DEFAULT;
}

static int mmgui_button_status_update(MMGUI *gui, int action)
{
	char	*value;

	if (gui->fileno == 0) {
		IupSetAttribute(gui->butt_del, "ACTIVE", "NO");
		IupSetAttribute(gui->butt_run, "ACTIVE", "NO");
	} else {
		if (action) {	/* option is ready */
			IupSetAttribute(gui->butt_run, "ACTIVE", "YES");
		} else {
			IupSetAttribute(gui->butt_run, "ACTIVE", "NO");
		}
		value = IupGetAttribute(gui->list_oldname, "VALUE");
		if (strchr(value, '+')) {
			IupSetAttribute(gui->butt_del, "ACTIVE", "YES");
		} else {
			IupSetAttribute(gui->butt_del, "ACTIVE", "NO");
		}
	}
	return IUP_DEFAULT;
}

/****************************************************************************
 * Option box group
 ****************************************************************************/
static Ihandle *mmgui_option_box(MMGUI *gui)
{
	Ihandle	*vbox, *hbox;

	vbox = IupVbox(IupLabel(NULL), NULL);
	IupSetAttribute(vbox, "NORMALIZESIZE", "BOTH");

	gui->tick_prefix  = IupToggle("Prefix", NULL);
	IupSetAttribute(gui->tick_prefix, "SIZE", "40");
	IupSetCallback(gui->tick_prefix, "ACTION",
			(Icallback) mmgui_option_event_tick_prefix);
	gui->entry_prefix = IupText(NULL);
	IupSetAttribute(gui->entry_prefix, "SIZE", "60");
	IupSetCallback(gui->entry_prefix, "KILLFOCUS_CB",
			(Icallback) mmgui_event_update);
	hbox = IupHbox(gui->tick_prefix, gui->entry_prefix, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ACENTER");
	IupAppend(vbox, hbox);

	gui->tick_suffix  = IupToggle("Suffix", NULL);
	IupSetAttribute(gui->tick_suffix, "SIZE", "40");
	IupSetCallback(gui->tick_suffix, "ACTION",
			(Icallback) mmgui_option_event_tick_suffix);
	gui->entry_suffix = IupText(NULL);
	IupSetAttribute(gui->entry_suffix, "SIZE", "60");
	IupSetCallback(gui->entry_suffix, "KILLFOCUS_CB", /* use the prefix */
			(Icallback) mmgui_event_update);
	hbox = IupHbox(gui->tick_suffix, gui->entry_suffix, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ACENTER");
	IupAppend(vbox, hbox);

	gui->tick_lowercase = IupToggle("Lowercase", NULL);
	IupSetCallback(gui->tick_lowercase, "ACTION",
			(Icallback) mmgui_option_event_tick_lowercase);
	IupAppend(vbox, gui->tick_lowercase);

	gui->tick_uppercase = IupToggle("Uppercase", NULL);
	IupSetCallback(gui->tick_uppercase, "ACTION",
			(Icallback) mmgui_option_event_tick_uppercase);
	IupAppend(vbox, gui->tick_uppercase);

	gui->tick_search = IupToggle("Search and Replace", NULL);
	IupSetCallback(gui->tick_search, "ACTION",
			(Icallback) mmgui_option_event_tick_search);
	IupAppend(vbox, gui->tick_search);
	return vbox;
}

static int mmgui_option_reset(MMGUI *gui)
{
	RNOPT	*opt = gui->ropt;

	if (opt->oflags & RNM_OFLAG_PREFIX) {
		IupSetAttribute(gui->tick_prefix, "VALUE", "ON");
		IupSetAttribute(gui->entry_prefix, "VALUE", opt->prefix);
		IupSetAttribute(gui->entry_prefix, "VISIBLE", "YES");
	} else {
		IupSetAttribute(gui->tick_prefix, "VALUE", "OFF");
		IupSetAttribute(gui->entry_prefix, "VALUE", "");
		IupSetAttribute(gui->entry_prefix, "VISIBLE", "NO");
	}
	if (opt->oflags & RNM_OFLAG_SUFFIX) {
		IupSetAttribute(gui->tick_suffix, "VALUE", "ON");
		IupSetAttribute(gui->entry_suffix, "VALUE", opt->suffix);
		IupSetAttribute(gui->entry_suffix, "VISIBLE", "YES");
	} else {
		IupSetAttribute(gui->tick_suffix, "VALUE", "OFF");
		IupSetAttribute(gui->entry_suffix, "VALUE", "");
		IupSetAttribute(gui->entry_suffix, "VISIBLE", "NO");
	}
	if ((opt->oflags & RNM_OFLAG_MASKCASE) == RNM_OFLAG_LOWERCASE) {
		IupSetAttribute(gui->tick_lowercase, "VALUE", "ON");
		IupSetAttribute(gui->tick_uppercase, "VALUE", "OFF");
	} else if ((opt->oflags & RNM_OFLAG_MASKCASE) == RNM_OFLAG_UPPERCASE) {
		IupSetAttribute(gui->tick_lowercase, "VALUE", "OFF");
		IupSetAttribute(gui->tick_uppercase, "VALUE", "ON");
	} else {
		IupSetAttribute(gui->tick_lowercase, "VALUE", "OFF");
		IupSetAttribute(gui->tick_uppercase, "VALUE", "OFF");
	}
	return 0;
}

static int mmgui_option_event_tick_prefix(Ihandle* ih, int state)
{
	MMGUI	*gui;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	
	if (state) {
		IupSetAttribute(gui->entry_prefix, "VISIBLE", "YES");
	} else {
		IupSetAttribute(gui->entry_prefix, "VISIBLE", "NO");
	}
	return mmgui_event_update(ih);
}

static int mmgui_option_event_tick_suffix(Ihandle* ih, int state)
{
	MMGUI	*gui;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	
	if (state) {
		IupSetAttribute(gui->entry_suffix, "VISIBLE", "YES");
	} else {
		IupSetAttribute(gui->entry_suffix, "VISIBLE", "NO");
	}
	return mmgui_event_update(ih);
}

static int mmgui_option_event_tick_lowercase(Ihandle* ih, int state)
{
	MMGUI	*gui;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	if (state) {
		IupSetAttribute(gui->tick_uppercase, "VALUE", "OFF");
	}
	return mmgui_event_update(ih);
}

static int mmgui_option_event_tick_uppercase(Ihandle* ih, int state)
{
	MMGUI	*gui;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	if (state) {
		IupSetAttribute(gui->tick_lowercase, "VALUE", "OFF");
	}
	return mmgui_event_update(ih);
}

static int mmgui_option_event_tick_search(Ihandle* ih, int state)
{
	MMGUI	*gui;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	
	mmgui_search_box_show(gui, state);
	return mmgui_event_update(ih);
}

static int mmgui_option_collection(MMGUI *gui)
{
	RNOPT	*opt = gui->ropt;
	char	*value;

	opt->oflags = 0;
	value = IupGetAttribute(gui->tick_lowercase, "VALUE");
	if (!strcmp(value, "ON")) {
		opt->oflags = RNM_OFLAG_LOWERCASE;
	} 
	value = IupGetAttribute(gui->tick_uppercase, "VALUE");
	if (!strcmp(value, "ON")) {
		opt->oflags = RNM_OFLAG_UPPERCASE;
	}
	value = IupGetAttribute(gui->tick_prefix, "VALUE");
	if (!strcmp(value, "ON")) {
		opt->prefix = IupGetAttribute(gui->entry_prefix, "VALUE");
		if ((opt->pre_len = strlen(opt->prefix)) > 0) {
			opt->oflags |= RNM_OFLAG_PREFIX;
		}
	}
	value = IupGetAttribute(gui->tick_suffix, "VALUE");
	if (!strcmp(value, "ON")) {
		opt->suffix = IupGetAttribute(gui->entry_suffix, "VALUE");
		if ((opt->suf_len = strlen(opt->suffix)) > 0) {
			opt->oflags |= RNM_OFLAG_SUFFIX;
		}
	}

	opt->action = 0; 
	value = IupGetAttribute(gui->tick_search, "VALUE");
	if (!strcmp(value, "ON")) {
		opt->patbuf  = NULL;
		opt->regflag = 0;
		opt->substit = IupGetAttribute(gui->entry_substit, "VALUE");
		opt->su_len = strlen(opt->substit);
		value = IupGetAttribute(gui->tick_icase, "VALUE");
		if (!strcmp(value, "OFF")) {
			opt->compare = strncmp;
		} else {
			opt->regflag |= REG_ICASE;
			opt->compare = strncasecmp;
		}
		opt->rpnum = 0;
		value = IupGetAttribute(gui->tick_replaced, "VALUE");
		if (!strcmp(value, "ON")) {
			value = IupGetAttribute(gui->entry_replaced, "VALUE");
			if (value) {
				opt->rpnum = (int) strtol(value, NULL, 0);
			}
		}

		value = IupGetAttribute(gui->radio_simple_match, "VALUE");
		if (!strcmp(value, "ON")) {
			opt->action = RNM_ACT_FORWARD;
		}
		value = IupGetAttribute(gui->radio_back_match, "VALUE");
		if (!strcmp(value, "ON")) {
			opt->action = RNM_ACT_BACKWARD;
		}
		value = IupGetAttribute(gui->radio_extension, "VALUE");
		if (!strcmp(value, "ON")) {
			opt->action = RNM_ACT_EXTENSION;
		}
		value = IupGetAttribute(gui->radio_exregex, "VALUE");
		if (!strcmp(value, "ON")) {
			opt->action = RNM_ACT_REGEX;
			opt->regflag |= REG_EXTENDED;
		}
		opt->pattern = IupGetAttribute(gui->entry_pattern, "VALUE");
		if ((opt->pa_len = strlen(opt->pattern)) == 0) {
			opt->action = 0;	/* empty pattern */
		} else if (rename_compile_regex(opt)) {
			opt->action = 0;	/* wrong regular expression */
		}
	}
	//rename_option_dump(opt);
	if ((opt->action == 0) && (opt->oflags == 0)) {
		return 0;
	}
	return 1;
}

static int mmgui_option_free(MMGUI *gui)
{
	RNOPT	*opt = gui->ropt;

	/* release the context if the regular expression been used */
	if (opt->action == RNM_ACT_REGEX) {
		opt->action = 0;
		regfree(opt->preg);
	}
	return IUP_DEFAULT;
}

/****************************************************************************
 * Search and Replace box group
 ****************************************************************************/
static Ihandle *mmgui_search_box(MMGUI *gui)
{
	Ihandle	*vbox, *item;

	gui->radio_simple_match = IupToggle("Forward Matching", NULL);
	gui->radio_back_match = IupToggle("Backward Matching", NULL);
	gui->radio_extension = IupToggle("Extension Matching", NULL);
	gui->radio_exregex = IupToggle("Regular Expression", NULL);
	IupSetCallback(gui->radio_simple_match, "ACTION",
			(Icallback) mmgui_event_update);
	IupSetCallback(gui->radio_back_match, "ACTION",
			(Icallback) mmgui_event_update);
	IupSetCallback(gui->radio_extension, "ACTION",
			(Icallback) mmgui_event_update);
	IupSetCallback(gui->radio_exregex, "ACTION",
			(Icallback) mmgui_event_update);
	item = IupVbox(gui->radio_simple_match, gui->radio_back_match,
			gui->radio_extension, gui->radio_exregex, NULL);
	vbox = IupVbox(IupRadio(item), NULL);

	gui->tick_icase = IupToggle("Ignore Cases", NULL);
	IupSetCallback(gui->tick_icase, "ACTION",
			(Icallback) mmgui_event_update);
	IupAppend(vbox, gui->tick_icase);

	gui->tick_replaced = IupToggle("Iteration", NULL);
	IupSetCallback(gui->tick_replaced, "ACTION",
			(Icallback) mmgui_search_event_tick_replaced);
	gui->entry_replaced = IupText(NULL);
	IupSetAttribute(gui->entry_replaced, "SIZE", "28");
	IupSetCallback(gui->entry_replaced, "KILLFOCUS_CB",
			(Icallback) mmgui_event_update);
	item = IupHbox(gui->tick_replaced, gui->entry_replaced, NULL);
	IupSetAttribute(item, "ALIGNMENT", "ACENTER");
	IupSetAttribute(item, "NGAP", "8");
	IupAppend(vbox, item);

	item = IupLabel("");	/* make a space */
	IupAppend(vbox, item);

	item = IupLabel("");	/* make an indent */
	IupSetAttribute(item, "SIZE", "10");
	return IupHbox(item, vbox, NULL);
}

static Ihandle *mmgui_search_strip(MMGUI *gui)
{
	Ihandle	*hbox;

	gui->lable_pattern = IupLabel("Search");
	gui->entry_pattern = IupText(NULL);
	IupSetAttribute(gui->entry_pattern, "EXPAND", "HORIZONTAL");
	IupSetCallback(gui->entry_pattern, "KILLFOCUS_CB",
			(Icallback) mmgui_event_update);

	gui->lable_substit = IupLabel("Replace");
	gui->entry_substit = IupText(NULL);
	IupSetAttribute(gui->entry_substit, "EXPAND", "HORIZONTAL");
	IupSetCallback(gui->entry_substit, "KILLFOCUS_CB",
			(Icallback) mmgui_event_update);

	hbox = IupHbox(gui->lable_pattern, gui->entry_pattern,
			gui->lable_substit, gui->entry_substit, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ACENTER");
	IupSetAttribute(hbox, "NGAP", "4");
	return hbox;
}

static int mmgui_search_reset(MMGUI *gui)
{
	RNOPT	*opt = gui->ropt;

	if (opt->action) {
		IupSetAttribute(gui->tick_search, "VALUE", "ON");
		IupSetAttribute(gui->entry_pattern, "VALUE", opt->pattern);
		IupSetAttribute(gui->entry_substit, "VALUE", opt->substit);
		mmgui_search_box_show(gui, 1);
	} else {
		IupSetAttribute(gui->tick_search, "VALUE", "OFF");
		IupSetAttribute(gui->entry_pattern, "VALUE", "");
		IupSetAttribute(gui->entry_substit, "VALUE", "");
		mmgui_search_box_show(gui, 0);
	}
	if (opt->regflag & REG_ICASE) {
		IupSetAttribute(gui->tick_icase, "VALUE", "ON");
	} else {
		IupSetAttribute(gui->tick_icase, "VALUE", "OFF");
	}
	if (opt->rpnum == 0) {
		IupSetAttribute(gui->tick_replaced, "VALUE", "OFF");
		IupSetAttribute(gui->entry_replaced, "VALUE", "");
		IupSetAttribute(gui->entry_replaced, "VISIBLE", "NO");
	} else {
		IupSetAttribute(gui->tick_replaced, "VALUE", "ON");
		IupSetInt(gui->entry_replaced, "VALUE", opt->rpnum);
		IupSetAttribute(gui->entry_replaced, "VISIBLE", "YES");
	}
	switch (opt->action) {
	case RNM_ACT_FORWARD:
		IupSetAttribute(gui->radio_simple_match, "VALUE", "ON");
		break;
	case RNM_ACT_BACKWARD:
		IupSetAttribute(gui->radio_back_match, "VALUE", "ON");
		break;
	case RNM_ACT_EXTENSION:
		IupSetAttribute(gui->radio_extension, "VALUE", "ON");
		break;
	case RNM_ACT_REGEX:
		IupSetAttribute(gui->radio_exregex, "VALUE", "ON");
		break;
	}
	return 0;
}

static int mmgui_search_box_show(MMGUI *gui, int state)
{
	char	*stbuf[2] = { "NO", "YES" };

	state = state ? 1 : 0;
	mmgui_search_strip_show(gui, state);
	IupSetAttribute(gui->tick_icase, "VISIBLE", stbuf[state]);
	IupSetAttribute(gui->tick_replaced, "VISIBLE", stbuf[state]);
	IupSetAttribute(gui->radio_simple_match, "VISIBLE", stbuf[state]);
	IupSetAttribute(gui->radio_back_match, "VISIBLE", stbuf[state]);
	IupSetAttribute(gui->radio_extension, "VISIBLE", stbuf[state]);
	IupSetAttribute(gui->radio_exregex, "VISIBLE", stbuf[state]);

	if (state == 0) {
		IupSetAttribute(gui->entry_replaced, "VISIBLE", stbuf[0]);
	} else {
		IupSetAttribute(gui->entry_replaced, "VISIBLE",
				stbuf[gui->state_replaced]);
	}
	return IUP_DEFAULT;
}

static int mmgui_search_strip_show(MMGUI *gui, int state)
{
	char	*stbuf[2] = { "NO", "YES" };

	state = state ? 1 : 0;
	IupSetAttribute(gui->lable_pattern, "VISIBLE", stbuf[state]);
	IupSetAttribute(gui->entry_pattern, "VISIBLE", stbuf[state]);
	IupSetAttribute(gui->lable_substit, "VISIBLE", stbuf[state]);
	IupSetAttribute(gui->entry_substit, "VISIBLE", stbuf[state]);
	return IUP_DEFAULT;
}

static int mmgui_search_event_tick_replaced(Ihandle* ih, int state)
{
	MMGUI	*gui;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	
	gui->state_replaced = state ? 1 : 0;
	if (state) {
		IupSetAttribute(gui->entry_replaced, "VISIBLE", "YES");
	} else {
		IupSetAttribute(gui->entry_replaced, "VISIBLE", "NO");
	}
	return mmgui_event_update(ih);
}

/****************************************************************************
 * File Conflict Popup
 ****************************************************************************/
static int mmgui_conflict_popup(MMGUI *gui, char *fname)
{
	Ihandle	*widget, *vbox, *hbox;
	Ihandle	*butt_yes, *butt_no, *butt_cancel, *tick_always;
	char	*title, *p;
	int	tlen;
	
	int mmgui_conflict_event(Ihandle* ih)
	{
		IupSetAttribute(ih, "FCDLGCLICK", "YES");
		return IUP_CLOSE;
	}

	tlen = strlen(fname) + 128;
	if ((title = smm_alloc(tlen)) == NULL) {
		return RNM_ERR_LOWMEM;
	}
	strcpy(title, "A file name \"");
	strcat(title, fname);
	strcat(title, "\" is already exists. Do you want to replace it?\n");
	
	/* IupLabel() can not wrap the line so I have to do it manually */
	for (p = title + 48; p < title + strlen(title); p += 48) {
		for ( ; *p; p++) {
			if (csc_isdelim(" -/\\", *p)) {
				csc_strinsert(title, tlen, p+1, 0, "\n");
				break;
			}
		}
	}

	widget = IupLabel(title);
	IupSetAttribute(widget, "FONTSIZE", "14");
	IupSetAttribute(widget, "FONTSTYLE", "Bold");
	IupSetAttribute(widget, "ALIGNMENT", "ALEFT");
	//IupSetAttribute(widget, "WORDWRAP", "YES");
	vbox = IupVbox(widget, NULL);

	widget = IupLabel("Replacing it will overwrite its content.\n\n\n");
	IupAppend(vbox, widget);

	tick_always = IupToggle("Apply this action to all files", NULL);
	IupAppend(vbox, tick_always);

	IupSetHandle("WARN_ICON", IupImageRGBA(64, 64, mmrc_icon_warning));
	widget = IupLabel("");
	IupSetAttribute(widget, "IMAGE", "WARN_ICON");
	hbox = IupHbox(widget, vbox, NULL);
	IupSetAttribute(hbox, "NGAP", "8");

	vbox = IupVbox(hbox, IupFill(), NULL);
	IupSetAttribute(vbox, "NGAP", "8");
	IupSetAttribute(vbox, "NMARGIN", "16x16");

	butt_yes = IupButton("Replace", NULL);
	IupSetAttribute(butt_yes, "SIZE", "50");
	IupSetAttribute(butt_yes, "IMAGE", "IUP_ActionOk");
	IupSetCallback(butt_yes, "ACTION", (Icallback) mmgui_conflict_event);

	butt_no = IupButton("Skip", NULL);
	IupSetAttribute(butt_no, "SIZE", "50");
	IupSetAttribute(butt_no, "IMAGE", "IUP_EditUndo");
	IupSetCallback(butt_no, "ACTION", (Icallback) mmgui_conflict_event);

	butt_cancel = IupButton("Cancel", NULL);
	IupSetAttribute(butt_cancel, "SIZE", "50");
	IupSetAttribute(butt_cancel, "IMAGE", "IUP_ActionCancel");
	IupSetCallback(butt_cancel, "ACTION", (Icallback)mmgui_conflict_event);

	widget = IupHbox(IupFill(), butt_cancel, butt_no, butt_yes, NULL);
	IupSetAttribute(widget, "NGAP", "4");
	IupAppend(vbox, widget);

	widget = IupDialog(vbox);
	IupSetAttribute(widget, "TITLE", "File Conflict");
	IupSetAttribute(widget, "ICON", "DLG_ICON");
	IupSetAttribute(widget, "RESIZE", "NO");
	IupSetAttribute(widget, "MAXBOX", "NO");
	IupSetAttribute(widget, "MINBOX", "NO");
	IupSetAttribute(widget, "HIDETASKBAR", "YES");
	IupSetAttribute(widget, "PARENTDIALOG", gui->inst_id);
	IupPopup(widget, IUP_CENTER, IUP_CENTER);
	smm_free(title);

	if ((p = IupGetAttribute(butt_yes, "FCDLGCLICK")) != NULL) {
		tlen = 1;
	} else if ((p = IupGetAttribute(butt_no, "FCDLGCLICK")) != NULL) {
		tlen = 0;
	} else {
		return 0;	/* cancel and no */
	}
	
	p = IupGetAttribute(tick_always, "VALUE");
	if (p && !strcmp(p, "ON")) {
		gui->ropt->cflags &= ~RNM_CFLAG_PROMPT_MASK;
		if (tlen) {
			gui->ropt->cflags |= RNM_CFLAG_ALWAYS;
		} else {
			gui->ropt->cflags |= RNM_CFLAG_NEVER;
		}
		rename_option_dump(gui->ropt);
	}
	return tlen;
}

static int mmgui_batch_popup(MMGUI *gui)
{
	IupMessagef("Batch Rename", 
			"Total Process Files:		%d	\n"
			"Successfully Renamed:		%d	\n"
			"Failed to be renamed:		%d	\n"
			"Unchanged Files:			%d	\n"
			"Skipped Existed Files:		%d	\n",
			gui->ropt->st_process, gui->ropt->st_success,
			gui->ropt->st_failed, gui->ropt->st_same, 
			gui->ropt->st_skip);
	return IUP_DEFAULT;
}

/****************************************************************************
 * Supportives
 ****************************************************************************/
static int mmgui_rename_exec(MMGUI *gui, int i, char *dstname, char *srcname)
{
	int 	rc;

	//printf("mmgui_rename_exec[%d]: %s -> %s\n", i, srcname, dstname);

	/* set the codepage to utf-8 before calling rename core. 
	 * In Win32 version, the rename uses the default codepage to process
	 * file name. However the GTK converted the file name to UTF-8 so 
	 * the Windows version could not find the file. */
	smm_codepage_set(65001);  /* set the codepage to utf-8 */
	rc = rename_executing(gui->ropt, dstname, srcname);
	smm_codepage_reset();

	switch (rc) {
	case RNM_ERR_NONE:
		/* multi-selection event will be triggered by this statement
		 * because it de-selected the current list by changing its
		 * content */
		IupSetAttributeId(gui->list_oldname, "", i, dstname);
		mmgui_fnlist_status(gui, IUPCOLOR_BLACK, "%d Files successfully renamed", 
				gui->ropt->st_success);
		break;
	case RNM_ERR_IGNORE:
		mmgui_fnlist_status(gui, IUPCOLOR_BLACK, "%d Files are not changed", 
				gui->ropt->st_same);
		break;
	case RNM_ERR_RENAME:
		mmgui_fnlist_status(gui, IUPCOLOR_RED, "%d Files have System Error",
				gui->ropt->st_failed);
		break;
	case RNM_ERR_SKIP:
		mmgui_fnlist_status(gui, IUPCOLOR_BLUE, 
				"%d Files conflict with the existed names",
				gui->ropt->st_skip);
	}
	return rc;
}

/* IUP generate different file list between one file and more files:
 * Open File VALUE: /home/xum1/dwhelper/lan_ke_er.flv
 * Last  DIRECTORY: /home/xum1/dwhelper/
 * Open File VALUE: /home/xum1/dwhelper|file-602303262.flv|
 *                      lan_ke_er.flv|Powered_by_Discuz.flv|
 * Last  DIRECTORY: /home/xum1/dwhelper0 
 *
 * IupPopup(dlg_open, IUP_CENTERPARENT, IUP_CENTERPARENT);
 * if (IupGetInt(dlg_open, "STATUS") >= 0) {
 *     char *sp = NULL;
 *     dfn = IupGetAttribute(dlg_open, "VALUE");
 *     do {
 *         result = IupTool_FileDlgExtract(dfn, &sp);
 *         smm_free(result);
 *     } while (result);
 */
static char *IupTool_FileDlgExtract(char *dfn, char **sp)
{
	char	*p, *rtn;
	int	path_len, sp_len;

	if ((p = strchr(dfn, '|')) != NULL) {	/* multi-file open */
		path_len = (int)(p - dfn);
	} else if (*sp == NULL) {	/* single file open first read */
		*sp = dfn + strlen(dfn);
		return csc_strcpy_alloc(dfn, 0);
	} else {	/* single file open more read */
		return NULL;
	}

	if (*sp == NULL) {	/* multi-file open first read */
		*sp = p + 1;
	}
	if (**sp == 0) {
		return NULL;
	}

	if ((p = strchr(*sp, '|')) != NULL) {
		sp_len = (int)(p - *sp);
	} else {
		sp_len = strlen(*sp);
	}
	
	if ((rtn = smm_alloc(path_len + sp_len + 4)) == NULL) {
		return NULL;
	}
	strncpy(rtn, dfn, path_len);
	rtn[path_len] = 0;
	strcat(rtn, SMM_DEF_DELIM);
	strncat(rtn, *sp, sp_len);

	if (p == NULL) {
		*sp += strlen(*sp);
	} else {
		*sp += sp_len + 1;
	}
	return rtn;
}

#if 0
/* IupTool_FileDlgExtract_Test("/home/xum1/dwhelper/lan_ke_er.flv");
 * IupTool_FileDlgExtract_Test("/home/xum1/dwhelper|lan_ke_er.flv");
 * IupTool_FileDlgExtract_Test("/home/xum1/dwhelper|
 * 		file-62.flv|lan_ke_er.flv|Discuz.flv|");
 * IupTool_FileDlgExtract_Test("/home/xum1/dwhelper|
 * 		file-62.flv|lan_ke_er.flv|Discuz.flv");
 */	
static void IupTool_FileDlgExtract_Test(char *dfn)
{
	char	*sp = NULL, *tmp;

	do {
		tmp = IupTool_FileDlgExtract(dfn, &sp);
		if (tmp) {
			printf("Extracted: %s\n", tmp);
			smm_free(tmp);
		}
	} while (tmp);
}
#endif

static int IupTool_FileDlgCounting(char *value)
{
	int	i, rc;

	for (i = rc = 0; value[i]; i++) {
		rc += (value[i] == '+') ? 1 : 0;
	}
	return rc;
}

