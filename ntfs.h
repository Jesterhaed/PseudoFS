#ifndef NTFS_H
#define NTFS_H

#define MFT_FRAGMENTS_COUNT 10
#define UID_ITEM_FREE -1
#define CLUSTER_COUNT 200
#define CLUSTER_SIZE 100

// obsahuje informace o NTFS
typedef struct Boot_record {
    char volume_descriptor[251];    //popis vygenerovaného FS
    int32_t disk_size;              //celkova velikost VFS
    int32_t cluster_size;           //velikost clusteru
    int32_t cluster_count;          //pocet clusteru
    int32_t mft_start_address;      //adresa pocatku mft
    int32_t bitmap_start_address;   //adresa pocatku bitmapy
    int32_t data_start_address;     //adresa pocatku datovych bloku
    int32_t mft_max_fragment_count; //maximalni pocet fragmentu v jednom zaznamu v mft (pozor, ne souboru)
                                    // stejne jako   MFT_FRAGMENTS_COUNT
    								// pokud by bylo 1, tak jeden soubor muze mit max 1 mft_item
}boot_record;

typedef struct Mft_fragment {
    int32_t fragment_start_address;     //start adresa
    int32_t fragment_count;             //pocet clusteru ve fragmentu
}mft_fragment;

// jeden soubor muze byt slozen i z vice mft_itemu, ty ale pak maji stejna uid (lisi se item_orderem)
typedef struct Mft_item {
    int uid;                     	                   //UID polozky, pokud UID = UID_ITEM_FREE, je polozka volna
    int isDirectory;                                    //soubor, nebo adresar (1=adresar, 0=soubor)
    int8_t item_order;                                  //poradi v MFT pri vice souborech, jinak 1
    int8_t item_order_total;                            //celkovy pocet polozek v MFT
    char item_name[12];                                 //8+3 + /0 C/C++ ukoncovaci string znak
    int32_t item_size;                                  //velikost souboru v bytech
    mft_fragment fragments[MFT_FRAGMENTS_COUNT]; 		//fragmenty souboru
}mft_item;

// itemy jsou ulozene ve spojovem seznamu, aby se jeden soubor mohl skladat z vice itemu
typedef struct Mft_list {
    mft_item item; // k nested prvkum pristupuji pres tecky
    struct Mft_list *next;
} mft_list;

mft_list *mft_seznam[CLUSTER_COUNT];

void copy_file();
void move_file();
void remove_file();
void make_dir();
void remove_dir();
void list_dir();
void concatenate();
void change_dir();
void print_dir();
void read_info();
void in_copy();
void out_copy();
void defragmentation();
void check_consistency();

#endif
