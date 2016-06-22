extern  struct  clicmd  fixtoken_cmd;
extern  struct  clicmd  memdump_cmd;
extern  struct  clicmd  slog_cmd;
extern  struct  clicmd  crc_cmd;
extern  struct  clicmd  fontpath_cmd;
extern  struct  clicmd  config_cmd;
extern  struct  clicmd  cdll_cmd;
extern  struct  clicmd  cli_cmd;
extern  struct  clicmd  cli_cmd2;
extern	struct	clicmd	smm_cmd;
extern  struct  clicmd  strings_cmd;

struct	clicmd	*cmdlist[] = {
	&fixtoken_cmd,
	&memdump_cmd,
	&slog_cmd,
	&crc_cmd,
	&fontpath_cmd,
	&config_cmd,
	&cdll_cmd,
	&cli_cmd,
	&cli_cmd2,
	&smm_cmd,
	&strings_cmd,
	NULL
};

