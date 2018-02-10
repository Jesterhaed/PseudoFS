#ifndef EXTEND_FUNCTIONS_H
#define EXTEND_FUNCTIONS_H

#include "ntfs.h"

int ntfs_bitmap[CLUSTER_COUNT];
int32_t pwd;
boot_record *boot_rec;

mft_list *alok_mft_item(mft_item mfti);
void add_mft_item(int uid, mft_item mfti);
char* get_cluster_content(int32_t adresa);
int set_cluster_content(int32_t adresa, char *obsah);
void clear_bitmap(mft_fragment fragment);
void clear_mft(int file_uid);
void clear_fragment_content(mft_fragment fragment);
char* get_fragment_content(mft_fragment fragment);
char* set_fragment_content(mft_fragment fragment, char *zbyvajici_obsah);
char* get_file_content(int file_uid);
void delete_file(int file_uid);
int update_filesize(int file_uid, int length);
int append_file_content(int file_uid, char *append, int dir);
void edit_file_content(int file_uid, char *text, char *filename, int puvodni_uid);
int make_file_in_mft(FILE *fw, int volne_uid, char *filename, char *text, mft_fragment fpom[], int fpom_size, int is_dir);
void load_ntfs();
void testing_ntfs(char filename[]);

#endif
