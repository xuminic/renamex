
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "iup.h"
#include "libcsoup.h"
#include "rename.h"
#include "mmrc_icon_dialog.h"

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

	Ihandle		*list_oldname;
	Ihandle		*list_preview;
	Ihandle		*progress;
	Ihandle		*status;
	Ihandle		*list_vbox;
	int		fileno;		/* file's number in the list */

	/* button box */
	Ihandle		*butt_load;
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
	Ihandle		*lable_pattern;
	Ihandle		*entry_pattern;
	Ihandle		*lable_substit;
	Ihandle		*entry_substit;
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


static int mmgui_event_resize(Ihandle *ih, int width, int height);
static int mmgui_event_show(Ihandle *ih, int state);
static int mmgui_event_close(Ihandle *ih);
static int mmgui_reset(MMGUI *gui);
static Ihandle *mmgui_fnlist_box(MMGUI *gui);
static int mmgui_fnlist_event_dropfiles(Ihandle *, char *, int,int,int);
static int mmgui_fnlist_event_multi_select(Ihandle *ih, char *value);
static int mmgui_fnlist_event_moused(Ihandle *ih, int, int, int, int, char *);
static int mmgui_fnlist_event_run(Ihandle *ih, int item, char *text);
static int mmgui_fnlist_append(MMGUI *gui, char *fname);
static int mmgui_fnlist_remove(MMGUI *gui, int idx);
static int mmgui_fnlist_rename(MMGUI *gui, int idx);
static int mmgui_fnlist_update_preview(MMGUI *gui);
static Ihandle *mmgui_button_box(MMGUI *gui);
static int mmgui_button_event_load(Ihandle *ih);
static int mmgui_button_event_delete(Ihandle *ih);
static int mmgui_button_event_rename(Ihandle *ih);
static int mmgui_button_event_about(Ihandle *ih);
static int mmgui_button_status_update(MMGUI *gui);
static Ihandle *mmgui_option_box(MMGUI *gui);
static int mmgui_option_reset(MMGUI *gui);
static int mmgui_option_event_tick_prefix(Ihandle* ih, int state);
static int mmgui_option_event_entry_prefix(Ihandle* ih);
static int mmgui_option_event_tick_suffix(Ihandle* ih, int state);
static int mmgui_option_event_tick_lowercase(Ihandle* ih, int state);
static int mmgui_option_event_tick_uppercase(Ihandle* ih, int state);
static int mmgui_option_event_tick_search(Ihandle* ih, int state);
static int mmgui_option_collection(MMGUI *gui);
static int mmgui_option_free(MMGUI *gui);
static Ihandle *mmgui_search_box(MMGUI *gui);
static int mmgui_search_reset(MMGUI *gui);
static int mmgui_search_box_show(MMGUI *gui, int state);
static int mmgui_search_event_tick_replaced(Ihandle* ih, int state);
static char *IupTool_FileDlgExtract(char *dfn, char **sp);


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

int mmgui_run(void *guiobj, int argc, char **argv)
{
	MMGUI	*gui = guiobj;
	Ihandle	*vbox_fname, *vbox_panel, *hbox;
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
	IupSetAttribute(gui->dlg_main, "RASTERSIZE", "640");
	IupSetAttribute(gui->dlg_main, RENAME_MAIN, (char*) gui);
	IupSetHandle(gui->inst_id, gui->dlg_main);
	IupSetCallback(gui->dlg_main, "RESIZE_CB", 
			(Icallback) mmgui_event_resize);
	IupSetCallback(gui->dlg_main, "SHOW_CB", 
			(Icallback) mmgui_event_show);
	IupSetCallback(gui->dlg_main, "CLOSE_CB", 
			(Icallback) mmgui_event_close);

	/* create the Open-File dialog initially so it can be popup and hide 
	 * without doing a real destory */
	gui->dlg_open = IupFileDlg();
	IupSetAttribute(gui->dlg_open, "PARENTDIALOG", gui->inst_id);
	IupSetAttribute(gui->dlg_open, "TITLE", "Open Files");
	IupSetAttribute(gui->dlg_open, "MULTIPLEFILES", "YES");

	/* show and run the interface */
	IupShow(gui->dlg_main);
	mmgui_reset(gui);
	for (i = 0; i < argc; i++) {
		mmgui_fnlist_append(gui, argv[i]);
	}
	IupMainLoop();
	return 0;
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
	}
	sprintf(buf, "%d", width - gui->magic_width);
	IupSetAttribute(gui->list_oldname, "RASTERSIZE", buf);
	IupSetAttribute(gui->list_preview, "RASTERSIZE", buf);
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
	mmgui_option_reset(gui);
	mmgui_search_reset(gui);
	mmgui_button_status_update(gui);
	return IUP_DEFAULT;
}


/****************************************************************************
 * file name list box
 ****************************************************************************/
static Ihandle *mmgui_fnlist_box(MMGUI *gui)
{
	/* create the controls of left side, the file list panel */
	gui->list_oldname = IupList(NULL);
	IupSetAttribute(gui->list_oldname, "EXPAND", "YES");
	IupSetAttribute(gui->list_oldname, "MULTIPLE", "YES");
	IupSetAttribute(gui->list_oldname, "SCROLLBAR", "YES");
	IupSetAttribute(gui->list_oldname, "DROPFILESTARGET", "YES");
	IupSetAttribute(gui->list_oldname, "ALIGNMENT", "ARIGHT");
	IupSetCallback(gui->list_oldname, "DROPFILES_CB",
			(Icallback) mmgui_fnlist_event_dropfiles);
	IupSetCallback(gui->list_oldname, "MULTISELECT_CB",
			(Icallback) mmgui_fnlist_event_multi_select);
	IupSetCallback(gui->list_oldname, "BUTTON_CB",
			(Icallback) mmgui_fnlist_event_moused);
	IupSetCallback(gui->list_oldname, "DBLCLICK_CB",
			(Icallback) mmgui_fnlist_event_run);
	
	gui->list_preview = IupList(NULL);
	IupSetAttribute(gui->list_preview, "EXPAND", "YES");
	IupSetAttribute(gui->list_preview, "SCROLLBAR", "YES");
	IupSetAttribute(gui->list_preview, "ALIGNMENT", "ARIGHT");
	IupSetAttribute(gui->list_preview, "CANFOCUS", "NO");
	IupSetAttribute(gui->list_preview, "FGCOLOR", IUPCOLOR_BLUE);

	gui->progress = IupProgressBar();
	IupSetAttribute(gui->progress, "EXPAND", "HORIZONTAL");
	IupSetAttribute(gui->progress, "DASHED", "YES");
	IupSetAttribute(gui->progress, "SIZE", "x10");
	IupSetAttribute(gui->progress, "VISIBLE", "NO");
	IupSetInt(gui->progress, "MIN", 0);
	IupSetInt(gui->progress, "MAX", 100);
	IupSetInt(gui->progress, "VALUE", 50);

	gui->status = IupLabel("Status is fine");
	IupSetAttribute(gui->status, "EXPAND", "HORIZONTAL");
	
	gui->list_vbox = IupVbox(gui->list_oldname, gui->list_preview, 
			gui->progress, gui->status, NULL);
	IupSetAttribute(gui->list_vbox, "NGAP", "4");
	return gui->list_vbox;
}

static int mmgui_fnlist_event_dropfiles(Ihandle *ih,
		char* filename, int num, int x, int y)
{
	MMGUI	*gui;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	printf("mmgui_fnlist_event_dropfiles: fname=%s number=%d %dx%d\n",
			filename, num, x, y);

	mmgui_fnlist_append(gui, filename);
	mmgui_button_status_update(gui);
	mmgui_fnlist_update_preview(gui);
	return IUP_DEFAULT;
}

static int mmgui_fnlist_event_multi_select(Ihandle *ih, char *value)
{
	MMGUI	*gui;

	(void) value;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	//printf("mmgui_fnlist_event_multi_select: %s\n", value);
	printf("List value=%s\n", IupGetAttribute(gui->list_oldname, "VALUE"));
	mmgui_button_status_update(gui);
	return IUP_DEFAULT;
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
	if (button == IUP_BUTTON3) {
		IupSetAttribute(gui->list_oldname, "VALUE", "");
	}
	mmgui_button_status_update(gui);
	return IUP_DEFAULT;
}

static int mmgui_fnlist_event_run(Ihandle *ih, int item, char *text)
{
	MMGUI	*gui;

	(void) text;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	
	mmgui_fnlist_rename(gui, item);
	return IUP_DEFAULT;
}

static int mmgui_fnlist_append(MMGUI *gui, char *fname)
{
	if (fname) {
		IupSetStrAttributeId(gui->list_oldname, "",  
				++(gui->fileno), fname);
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
	return IUP_DEFAULT;
}

static int mmgui_fnlist_rename(MMGUI *gui, int idx)
{
	char	*fname;

	fname = IupGetAttributeId(gui->list_oldname, "",  idx);
	if (fname) {
		printf("RENAME: %s\n", fname);
	}
	return IUP_DEFAULT;
}

static int mmgui_fnlist_update_preview(MMGUI *gui)
{
	char	*fname, *preview;
	int	i, rc;

	if (mmgui_option_collection(gui) == 0) {	/* option not ready */
		while (IupGetAttributeId(gui->list_preview, "", 1)) {
			IupSetInt(gui->list_preview, "REMOVEITEM", 1);
		}
		return mmgui_option_free(gui);
	}

	for (i = 0; i < gui->fileno; i++) {
		fname = IupGetAttributeId(gui->list_oldname, "",  i+1);
		if ((preview = rename_alloc(gui->ropt, fname, &rc)) != NULL) {
			IupSetStrAttributeId(gui->list_preview, 
					"",  i+1, preview);
			smm_free(preview);
		} else if (rc == RNM_ERR_SKIP) { /* same to old name */
			IupSetStrAttributeId(gui->list_preview, 
					"",  i+1, fname);
		}
	}
	return IUP_DEFAULT;
}

/****************************************************************************
 * Button box group
 ****************************************************************************/
static Ihandle *mmgui_button_box(MMGUI *gui)
{
	Ihandle	*vbox;

	gui->butt_load = IupButton("Open", NULL);
	IupSetAttribute(gui->butt_load, "SIZE", "50");
	gui->butt_del = IupButton("Delete", NULL);
	IupSetAttribute(gui->butt_del, "SIZE", "50");
	gui->butt_run = IupButton("Rename", NULL);
	IupSetAttribute(gui->butt_run, "SIZE", "50");
	gui->butt_about = IupButton("About", NULL);
	IupSetAttribute(gui->butt_about, "SIZE", "50");

	vbox = IupGridBox(gui->butt_load, gui->butt_del, gui->butt_run, 
			gui->butt_about, NULL);
	IupSetAttribute(vbox, "ORIENTATION", "HORIZONTAL");
	IupSetAttribute(vbox, "NUMDIV", "2");

	IupSetCallback(gui->butt_load, "ACTION",
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

	/* FIXME: What would the path look like in Win32? */
	printf("Open File VALUE: %s\n", IupGetAttribute(gui->dlg_open, "VALUE"));
	printf("Last  DIRECTORY: %s\n", IupGetAttribute(gui->dlg_open, "DIRECTORY"));
	dlgrd = IupGetAttribute(gui->dlg_open, "VALUE");
	while ((fname = IupTool_FileDlgExtract(dlgrd, &sp)) != NULL) {
		mmgui_fnlist_append(gui, fname);
		/* IupList control seems save a copy of its content.
		 * The 'fname' can not be retrieved by IupGetAttribute() */
		smm_free(fname);
	}
	mmgui_button_status_update(gui);
	mmgui_fnlist_update_preview(gui);
	return IUP_DEFAULT;
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
	mmgui_button_status_update(gui);
	mmgui_fnlist_update_preview(gui);	//FIXME: it should be removed without proper view
	return IUP_DEFAULT;
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

	/* run rename one by one */
	IupSetAttribute(gui->progress, "VISIBLE", "YES");
	value = IupGetAttribute(gui->list_oldname, "VALUE");
	vflag = strchr(value, '+');
	IupSetInt(gui->progress, "MAX", gui->fileno);
	for (i = 0; i < gui->fileno; i++) {
		if (vflag == NULL) {
			mmgui_fnlist_rename(gui, i+1);
			IupSetInt(gui->progress, "VALUE", i+1);
		} else if (value[i] == '+') {
			mmgui_fnlist_rename(gui, i+1);
			IupSetInt(gui->progress, "VALUE", i+1);
		}
		IupFlush();
	}
	IupSetInt(gui->progress, "VALUE",gui->fileno);
	smm_sleep(1, 0);
	IupSetAttribute(gui->progress, "VISIBLE", "NO");
	return mmgui_option_free(gui);
}

static int mmgui_button_event_about(Ihandle *ih)
{
	MMGUI	*gui;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	printf("mmgui_button_event_about: %p\n", gui);
	return IUP_DEFAULT;
}

static int mmgui_button_status_update(MMGUI *gui)
{
	char	*value;

	if (gui->fileno == 0) {
		IupSetAttribute(gui->butt_del, "ACTIVE", "NO");
		IupSetAttribute(gui->butt_run, "ACTIVE", "NO");
	} else {
		IupSetAttribute(gui->butt_run, "ACTIVE", "YES");
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

	gui->tick_prefix  = IupToggle("Prefix", NULL);
	IupSetAttribute(gui->tick_prefix, "SIZE", "40");
	IupSetCallback(gui->tick_prefix, "ACTION",
			(Icallback) mmgui_option_event_tick_prefix);
	gui->entry_prefix = IupText(NULL);
	IupSetAttribute(gui->entry_prefix, "SIZE", "60x10");
	IupSetCallback(gui->entry_prefix, "KILLFOCUS_CB",
			(Icallback) mmgui_option_event_entry_prefix);
	hbox = IupHbox(gui->tick_prefix, gui->entry_prefix, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ACENTER");
	IupAppend(vbox, hbox);

	gui->tick_suffix  = IupToggle("Suffix", NULL);
	IupSetAttribute(gui->tick_suffix, "SIZE", "40");
	IupSetCallback(gui->tick_suffix, "ACTION",
			(Icallback) mmgui_option_event_tick_suffix);
	gui->entry_suffix = IupText(NULL);
	IupSetAttribute(gui->entry_suffix, "SIZE", "60x10");
	IupSetCallback(gui->entry_suffix, "KILLFOCUS_CB", /* use the prefix */
			(Icallback) mmgui_option_event_entry_prefix);
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
	mmgui_fnlist_update_preview(gui);
	return IUP_DEFAULT;
}

static int mmgui_option_event_entry_prefix(Ihandle* ih)
{
	MMGUI	*gui;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	mmgui_fnlist_update_preview(gui);
	return IUP_DEFAULT;
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
	mmgui_fnlist_update_preview(gui);
	return IUP_DEFAULT;
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
	mmgui_fnlist_update_preview(gui);
	return IUP_DEFAULT;
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
	mmgui_fnlist_update_preview(gui);
	return IUP_DEFAULT;
}

static int mmgui_option_event_tick_search(Ihandle* ih, int state)
{
	MMGUI	*gui;

	if ((gui = (MMGUI *) IupGetAttribute(ih, RENAME_MAIN)) == NULL) {
		return IUP_DEFAULT;
	}
	
	mmgui_search_box_show(gui, state);
	mmgui_fnlist_update_preview(gui);
	return IUP_DEFAULT;
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
			if (regcomp(opt->preg, opt->pattern, opt->regflag)) {
				/* wrong regular expression */
				opt->action = 0;
				return 0;	/* invalid options */
			}
		}
		opt->pattern = IupGetAttribute(gui->entry_pattern, "VALUE");
		if ((opt->pa_len = strlen(opt->pattern)) == 0) {
			opt->action = 0;	/* empty pattern */
		} else if ((opt->action == RNM_ACT_REGEX) &&
				regcomp(opt->preg, opt->pattern, opt->regflag)) {
			opt->action = 0;
		}
	}
	rename_option_dump(opt);
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
	Ihandle	*vbox, *hbox;

	gui->lable_pattern = IupLabel("Search");
	IupSetAttribute(gui->lable_pattern, "SIZE", "32");
	gui->entry_pattern = IupText(NULL);
	IupSetAttribute(gui->entry_pattern, "SIZE", "60x10");
	IupSetCallback(gui->entry_pattern, "KILLFOCUS_CB",
			(Icallback) mmgui_option_event_entry_prefix);
	hbox = IupHbox(gui->lable_pattern, gui->entry_pattern, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ACENTER");
	vbox = IupVbox(hbox, NULL);

	gui->lable_substit = IupLabel("Replace");
	IupSetAttribute(gui->lable_substit, "SIZE", "32");
	gui->entry_substit = IupText(NULL);
	IupSetAttribute(gui->entry_substit, "SIZE", "60x10");
	IupSetCallback(gui->entry_substit, "KILLFOCUS_CB",
			(Icallback) mmgui_option_event_entry_prefix);
	hbox = IupHbox(gui->lable_substit, gui->entry_substit, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ACENTER");
	IupAppend(vbox, hbox);

	gui->tick_icase = IupToggle("Ignore Cases", NULL);
	IupAppend(vbox, gui->tick_icase);

	gui->tick_replaced = IupToggle("Repeat No.  ", NULL);
	IupSetCallback(gui->tick_replaced, "ACTION",
			(Icallback) mmgui_search_event_tick_replaced);
	gui->entry_replaced = IupText(NULL);
	IupSetAttribute(gui->entry_replaced, "SIZE", "24x10");
	IupSetCallback(gui->entry_replaced, "KILLFOCUS_CB",
			(Icallback) mmgui_option_event_entry_prefix);
	hbox = IupHbox(gui->tick_replaced, gui->entry_replaced, NULL);
	IupSetAttribute(hbox, "ALIGNMENT", "ACENTER");
	IupAppend(vbox, hbox);

	gui->radio_simple_match = IupToggle("Forward Matching", NULL);
	gui->radio_back_match = IupToggle("Backward Matching", NULL);
	gui->radio_extension = IupToggle("Extension Matching", NULL);
	gui->radio_exregex = IupToggle("Regular Expression", NULL);
	hbox = IupVbox(gui->radio_simple_match, gui->radio_back_match,
			gui->radio_extension, gui->radio_exregex, NULL);
	IupAppend(vbox, IupRadio(hbox));

	hbox = IupLabel("");
	IupSetAttribute(hbox, "SIZE", "10");
	return IupHbox(hbox, vbox, NULL);
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
	IupSetAttribute(gui->lable_pattern, "VISIBLE", stbuf[state]);
	IupSetAttribute(gui->entry_pattern, "VISIBLE", stbuf[state]);
	IupSetAttribute(gui->lable_substit, "VISIBLE", stbuf[state]);
	IupSetAttribute(gui->entry_substit, "VISIBLE", stbuf[state]);
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
	mmgui_fnlist_update_preview(gui);
	return IUP_DEFAULT;
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
 * IupTool_FileDlgExtract_Test("/home/xum1/dwhelper|file-62.flv|lan_ke_er.flv|Discuz.flv|");
 * IupTool_FileDlgExtract_Test("/home/xum1/dwhelper|file-62.flv|lan_ke_er.flv|Discuz.flv");
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


