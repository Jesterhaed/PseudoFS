#ifndef ENHANCED_FUNCTIONS_H
#define ENHANCED_FUNCTIONS_H

#include <pthread.h>

typedef struct Share_memory{
	pthread_mutex_t * mutex;
	int zpracovany_cluster;
}share_memory;

int get_free_uid();
int parcing_path(char *patha, int cd);
int make_new_file(int pwd, char *name);
int get_uid_by_name(char *dir_name, int uid_pwd);
int is_name_unique(char *newname, int uid_pwd);
void ls_printer(int uid);
int is_empty_dir(int file_uid);
char* read_file_from_pc(char *pc_soubor);
void make_file(int target_file, char *filename, char *text, int puvodni_uid, int is_dir, int odkaz);
int get_backlink(int uid_pwd);
void *check_consist(void *arg);

#endif
