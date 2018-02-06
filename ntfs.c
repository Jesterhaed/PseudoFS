/*
 * ntfs.c
 *
 *  Created on: 4. 2. 2018
 *      Author: deserteagle
 */
#include <stdio.h>
#include <string.h>
#include "ntfs.h"

char adresa1[255];
char adresa2[255];

int nacti_ntfs(char* ntfs) {
	printf("Nacten FS: %s.\n", ntfs);
	return 1;
}

int kopiruj_soubor() {
	scanf("%255s",adresa1);
	scanf("%255s",adresa2);
	printf("Adr1: %s.\n", adresa1);
	printf("Adr2: %s.\n", adresa2);
	return 1;
}

int presun_soubor() {
	scanf("%255s",adresa1);
	scanf("%255s",adresa2);
	return 1;
}

int smaz_soubor() {
	scanf("%255s",adresa1);
	return 1;
}

int vytvor_adr() {
	scanf("%255s",adresa1);
	return 1;
}

int smaz_adr() {
	scanf("%255s",adresa1);
	return 1;
}

int vypis_adr() {
	scanf("%255s",adresa1);
	return 1;
}

int vypis_soubor() {
	scanf("%255s",adresa1);
	return 1;
}

int zmen_adr() {
	scanf("%255s",adresa1);
	return 1;
}

int vypis_cestu() {
	return 1;
}

int vypis_info() {
	scanf("%255s",adresa1);
	return 1;
}

int nahraj_do_ntfs() {
	scanf("%255s",adresa1);
	scanf("%255s",adresa2);
	return 1;
}

int nahraj_z_ntfs() {
	scanf("%255s",adresa1);
	scanf("%255s",adresa2);
	return 1;
}

int nacti_soubor() {
	scanf("%255s",adresa1);
	return 1;
}

void uloz_ntfs(char* ntfs) {
	printf("%s ulozena.\n", ntfs);
}

void uvolni_ntfs() {

}

int testovaci_ntfs(char* ntfs){
	FILE *fw;
	boot_record *boot_record;
	mft_item *mft_item;

	int i;
	int bitmap[CLUSTER_COUNT];
	int all_fragments_size = CLUSTER_COUNT * sizeof(struct mft_fragment);
	int all_clusters_size = CLUSTER_COUNT * sizeof(struct mft_item);
	int bitmap_size = 4 * CLUSTER_COUNT;
	int bitmap_start = sizeof(struct boot_record) + all_fragments_size + all_clusters_size;
	int data_start = bitmap_start + bitmap_size;


	fw = fopen(ntfs, "wb");
	if (fw != NULL) {
		boot_record = malloc(sizeof(struct boot_record));

		strcpy(boot_record->signature, "Eagle");
		strcpy(boot_record->volume_descriptor, "PseudoNTFS");
		boot_record->disk_size = CLUSTER_SIZE * CLUSTER_COUNT;
		boot_record->cluster_size = CLUSTER_SIZE;
		boot_record->cluster_count = CLUSTER_COUNT;
		boot_record->mft_start_address = sizeof(struct boot_record);
		boot_record->bitmap_start_address = bitmap_start;
		boot_record->data_start_address = data_start;
		boot_record->mft_max_fragment_count = 1;

		fwrite(boot_record, sizeof(struct boot_record), 1, fw);

		fseek(fw, bitmap_start, SEEK_SET);
		bitmap[0] = 1;
		/* bez nulovani bitmapy */
		fwrite(bitmap, 4, CLUSTER_COUNT, fw);

		fseek(fw, sizeof(struct boot_record), SEEK_SET);

		mft_item = malloc(sizeof(struct mft_item));

		mft_item->uid = 0;
		mft_item->isDirectory = 1;
		mft_item->item_order = 1;
		mft_item->item_order_total = 1;
		strcpy(mft_item->item_name, "ROOT_DIR");
		mft_item->item_size = 1;

		mft_item->fragments[0].fragment_start_address = data_start;
		mft_item->fragments[0].fragment_count = 1;

		// nastaveni fragmentu na prazdne
		for (i = 1; i < MFT_FRAGMENTS_COUNT; i++){
			mft_item->fragments[i].fragment_start_address = UID_ITEM_FREE;
			mft_item->fragments[i].fragment_count = UID_ITEM_FREE;
		}

		fwrite(mft_item, sizeof(struct mft_item), 1, fw);

		fseek(fw, data_start, SEEK_SET);
		char odkaz[2];
		odkaz[0] = '0';
		odkaz[1] = '\0';
		fwrite(odkaz, 1, 2, fw);

		free((void *) mft_item);
		free((void *) boot_record);
		fclose(fw);
	}
	return 1;
}
