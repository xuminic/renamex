
#ifndef	_LIBSMM_H_
#define _LIBSMM_H_

#define	SMM_FSTAT_REGULAR	0
#define SMM_FSTAT_DIR		1
#define SMM_FSTAT_DEVICE	2
#define SMM_FSTAT_LINK		3

#define SMM_PATH_DIR_FIFO	0
#define SMM_PATH_DIR_FIRST	1
#define SMM_PATH_DIR_LAST	2
#define SMM_PATH_DIR_MASK	3

#define SMM_MSG_PATH_ENTER	0
#define SMM_MSG_PATH_LEAVE	1
#define SMM_MSG_PATH_EXEC	2
#define SMM_MSG_PATH_STAT	3
#define SMM_MSG_PATH_BREAK	4


struct	smmdir	{
	int	flags;

	int	stat_dirs;
	int	stat_files;

	int     (*message)(void *option, char *path, int type, void *info);
	void	*option;

	int	(*path_recur)(struct smmdir *sdir, char *path);
};

typedef int (*F_DIR)(void*, char*, int, void*);


extern	int	smm_error_no;
extern	int	smm_sys_cp;

int smm_chdir(char *path);
int smm_codepage(void);
char *smm_cwd_alloc(void);
int smm_cwd_pop(int cwid);
int smm_cwd_push(void);
int smm_errno(void);
int smm_errno_update(int value);
int smm_fstat(char *fname);
int smm_init(void);
int smm_pathtrek(char *path, int flags, F_DIR message, void *option);
int smm_pwuid(char *uname, long *uid, long *gid);
int smm_signal_break(int (*handle)(int));
void *smm_mbstowcs(char *mbs);
char *smm_wcstombs(void *wcs);

#endif

