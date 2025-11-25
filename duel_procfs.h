#ifndef _DUEL_PROCFS_H_
#define _DUEL_PROCFS_H_

#define DUEL_PROC_BUF_SIZE 255
#define DUEL_PROC_NAME "duel-params"

extern int duel_init_procfs(void);
extern void duel_exit_procfs(void);

#endif
