/*
 * ntfs.h
 *
 *  Created on: 4. 2. 2018
 *      Author: deserteagle
 */

#ifndef NTFS_H_
#define NTFS_H_

#define UID_ITEM_FREE 0
#define MFT_FRAGMENTS_COUNT 32

typedef struct boot_record {
    char signature[9];              //login autora FS
    char volume_descriptor[251];    //popis vygenerovaného FS
    int disk_size;              //celkova velikost VFS
    int cluster_size;           //velikost clusteru
    int cluster_count;          //pocet clusteru
    int mft_start_address;      //adresa pocatku mft
    int bitmap_start_address;   //adresa pocatku bitmapy
    int data_start_address;     //adresa pocatku datovych bloku
    int mft_max_fragment_count; //maximalni pocet fragmentu v jednom zaznamu v mft (pozor, ne souboru)
                                    // stejne jako   MFT_FRAGMENTS_COUNT
}boot_record;

typedef struct mft_fragment {
	int fragment_start_address;     //start adresa
	int fragment_count;             //pocet clusteru ve fragmentu
}mft_fragment;

typedef struct mft_item {
	int uid;                                        //UID polozky, pokud UID = UID_ITEM_FREE, je polozka volna
	int isDirectory;                                   //soubor, nebo adresar
    int item_order;                                  //poradi v MFT pri vice souborech, jinak 1
    int item_order_total;                            //celkovy pocet polozek v MFT
    char item_name[12];                                 //8+3 + /0 C/C++ ukoncovaci string znak
    int item_size;                                  //velikost souboru v bytech
    mft_fragment fragments[MFT_FRAGMENTS_COUNT]; //fragmenty souboru
}mft_item;


void kopiruj();
int nacti_ntfs(char* ntfs);
int kopiruj_soubor();
int presun_soubor();
int smaz_soubor();
int vytvor_adr();
int smaz_adr();
int vypis_adr();
int vypis_soubor();
int zmen_adr();
int vypis_cestu();
int vypis_info();
int nahraj_do_ntfs();
int nahraj_z_ntfs();
int nacti_soubor();
void uloz_ntfs(char* ntfs);
void uvolni_ntfs();
#endif /* NTFS_H_ */
