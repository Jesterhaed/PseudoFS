#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "extend_functions.h"
#include "enhanced_functions.h"
#include "ntfs.h"

extern int pwd;
extern char ntfs_file[255];

/* Alokuje prvek mft listu */
mft_list *alok_mft_item(mft_item mft_Item) {
    mft_list *ml;
    if ((ml = (mft_list *) malloc(sizeof(mft_list))) == NULL) {
        printf("Out of memory - MFT_LIST\n");
    }

    ml->item = mft_Item;
    ml->next = NULL;

    return ml;
}

/* Prida prvek mft na prislusny index do globalniho mft pole */
void add_mft_item(int uid, mft_item mft_Item) {
	mft_list *pom = alok_mft_item(mft_Item);

    if(mft_seznam[uid] == NULL){
        mft_seznam[uid] = pom;
    }
    else {
        // vlozime prvek nakonec
        mft_list *mpom = mft_seznam[uid];
        while (mpom != NULL) {
            if (mpom->next == NULL) {
                mpom->next = pom;
                break;
            }

            mpom = mpom->next;
        }
    }
}

/*
    Ziska data z jednoho clusteru
    @param adresa Adresa pocatku datoveho bloku
    @return Obsah bloku
*/
char* get_cluster_content(int32_t adresa) {
    FILE *fr;
    char *ret;

    ret = (char*) malloc(CLUSTER_SIZE);

    fr = fopen(ntfs_file, "rb");
    if (fr != NULL) {
        fseek(fr, adresa, SEEK_SET);
        fread(ret, sizeof(char), CLUSTER_SIZE, fr);

        fclose(fr);
    }
    return ret;
}

/*
    Prepise obsah clusteru
    @param adresa Adresa pocatku datoveho bloku
    @param obsah Novy obsah bloku
    @return Zaporne hodnoty chybne
*/

int set_cluster_content(int32_t adresa, char *obsah) {
    FILE *f;

	char *buffer;

    f = fopen(ntfs_file, "r+b");
	int velikost_obsahu = strlen(obsah);

    if (f != NULL) {

		fseek(f, adresa, SEEK_SET);

		if (velikost_obsahu < CLUSTER_SIZE) {

			int pocet_nul = CLUSTER_SIZE - velikost_obsahu;

			buffer = malloc(pocet_nul);

			memset(buffer, 0, pocet_nul);
			fwrite(obsah, 1, velikost_obsahu, f);
			fwrite(buffer, 1, pocet_nul, f);

		}
		else {
			fwrite(obsah, 1, CLUSTER_SIZE, f);
		}

        fclose(f);
        return 1;
    }

    return -1;
}

/*
    Smaze data ve vsech clusterech patricich k danemu souboru a uklidi po danem souboru
    @param file_uid UID souboru ke smazani
*/
void delete_file(int file_uid) {
    mft_list* mft_itemy;
    mft_item mfti;
    mft_fragment mftf;
    int j;

    // vsechny mfti
    if (mft_seznam[file_uid] != NULL){
        mft_itemy = mft_seznam[file_uid];

        // iteruce mfti
        while (mft_itemy != NULL){
            mfti = mft_itemy->item;

            // vsechny mftf
            for (j = 0; j < MFT_FRAGMENTS_COUNT; j++){
                mftf = mfti.fragments[j];

                if (mftf.fragment_start_address != 0 && mftf.fragment_count > 0) {
                    // prepis dat - smazamo clusteru v souboru
                    clear_fragment_content(mftf);

                    // vymaz bitmapy (virtualne i v souboru)
                    clear_bitmap(mftf);
                }
            }

            // prehozeni na dalsi prvek
            mft_itemy = mft_itemy->next;
        }

        // cisteni mft (virtualne i v souboru)
        clear_mft(file_uid);
    }
}

/*
    Smaze zaznam z mtf
    @param file_uid
*/
void clear_mft(int file_uid) {
    FILE *fw;
    int i, adresa;
    char obsah[sizeof(mft_item)];

    // vynuluje z listu (virtualne)
    mft_seznam[file_uid] = NULL;

    fw = fopen(ntfs_file, "r+b");
    if(fw != NULL) {
        for(i = 0; i < CLUSTER_COUNT; i++) {
            if (i == file_uid) {
                // prepis mfti
                adresa = sizeof(boot_record) + sizeof(mft_item) * file_uid;
                memset(obsah, 0, sizeof(mft_item));
                fseek(fw, adresa, SEEK_SET);
                fwrite(obsah, 1, sizeof(mft_item), fw);
            }
        }
        fclose(fw);
    }
}

/*
    Ziska obsah vsech fragmentu patricich do clusteru
    @param fragment Struktura fragmentu, kterou chceme cist
    @return Obsah celeho fragmentu
*/
char* get_fragment_content(mft_fragment fragment) {
    int adresa, bloku, i;
    char *ret;

    adresa = fragment.fragment_start_address;
    bloku = fragment.fragment_count;
    ret = (char*) malloc(bloku * CLUSTER_SIZE);
    strcpy(ret, "");

    if (adresa != 0) {
        for (i = 0; i < bloku; i++) {
            strcat(ret, get_cluster_content(adresa));

            adresa += CLUSTER_SIZE;
        }
    }
    return ret;
}

/*
    Naplni clustery z daneho fragmentu
    @param fragment Informace o fragmentu
    @param rest_content Obsah pro naplneni
    @return Vratim neulozeny string
*/
char* set_fragment_content(mft_fragment fragment, char *rest_content) {
    int i;
    int adresa = fragment.fragment_start_address;

    for (i = 0; i < fragment.fragment_count; i++) {

        set_cluster_content(adresa, rest_content);

        adresa += CLUSTER_SIZE;
        rest_content += CLUSTER_SIZE;
    }

    return rest_content;
}

/*
    Vynuluje obsah zadaneho fragmentu
    @param fragment Struktura fragmentu, ktery chceme prepsat
*/
void clear_fragment_content(mft_fragment fragment) {
    int adresa, bloku, i;
    char obsah[CLUSTER_SIZE];

    adresa = fragment.fragment_start_address;
    bloku = fragment.fragment_count;

    memset(obsah, 0, CLUSTER_SIZE);

    if (adresa != 0) {
        for (i = 0; i < bloku; i++) {
            set_cluster_content(adresa, obsah);

            adresa += CLUSTER_SIZE;
        }
    }
}

/*
    Vynuluje obsah zadaneho fragmentu
    @param fragment Struktura fragmentu, ktey chceme prepsat
*/
void clear_bitmap(mft_fragment fragment) {
    int index_s, index_e, i;
    FILE *fw;

    // podle adresy pozna ID clusteru
    index_s = (fragment.fragment_start_address - boot_rec->data_start_address) / CLUSTER_SIZE;
    index_e = index_s + fragment.fragment_count;


    // update virtualni bitmapy
    for (i = index_s; i < index_e; i++) {
        ntfs_bitmap[i] = 0;
    }

    // prepis cele bitmapy v souboru
    fw = fopen(ntfs_file, "r+b");
    if(fw != NULL){
        fseek(fw, boot_rec->bitmap_start_address, SEEK_SET);
        fwrite(ntfs_bitmap, 4, CLUSTER_COUNT, fw);

        fclose(fw);
    }
}

/*
    Ziska obsah celeho souboru - precte si vsechny udaje z MFTLISTU (MFTI, MFTF)
    @param file_uid UID cteneho souboru
    @return Obsah celeho souboru
*/
char* get_file_content(int file_uid) {
    int i, j;
    char *ret;
    mft_list* mft_itemy;
    mft_item mfti;
    mft_fragment mftf;

    // existuje takovy item v MFT
    if (mft_seznam[file_uid] != NULL){
        mft_itemy = mft_seznam[file_uid];


        // iterace vsech itemu pro dane UID souboru
        i = 0;
        while (mft_itemy != NULL){
            mfti = mft_itemy->item;
            i++;

            // alokuace cele velikost dle MFT
            if (i == 1) {
                ret = (char*) malloc(((mfti.item_size / CLUSTER_SIZE) + 1) * CLUSTER_SIZE);
                strcpy(ret, "");
            }


            // precte vsechny fragmenty z daneho mft itemu
            for (j = 0; j < MFT_FRAGMENTS_COUNT; j++){
                mftf = mfti.fragments[j];

                if (mftf.fragment_start_address != 0 && mftf.fragment_count > 0) {

                    char *fragc = get_fragment_content(mftf);
                    strncat(ret, fragc, strlen(fragc));
                }
            }
            mft_itemy = mft_itemy->next;
        }
    }

    return ret;
}

/*
    Upravi danemu souboru velikost
    @param file_uid Unikatni cislo souboru
    @param length Delka pro zapis do MFTI souboru
*/
int update_filesize(int file_uid, int length){
    FILE *fw;
    mft_item *mpom;
    int mft_size = sizeof(mft_item);

    fw = fopen(ntfs_file, "r+b");
    if (fw != NULL) {
        mpom = malloc(mft_size);

        // aktualizace virtualni MFT
        mft_seznam[file_uid]->item.item_size = length;

        // zapis mft
        mpom = &mft_seznam[file_uid]->item;
        fseek(fw, boot_rec->mft_start_address + file_uid * mft_size, SEEK_SET);
        fwrite(mpom, mft_size, 1, fw);

        fclose(fw);

        return 0;
    }

    return -1;
}

/*
    Pripoji na konec souboru dalsi data
    @param file_uid UID souboru
    @param append Retezec pro pripojeni nakonec souboru
    @param dir Dir nebo adr
*/
int append_file_content(int file_uid, char *append, int dir){
    int i, j, adresa, delka, mftf_adr;
    char *ret;
    mft_list* mft_itemy;
    FILE *fw;

    ret = (char *) malloc(CLUSTER_SIZE);

    fw = fopen(ntfs_file, "r+b");
    if (fw != NULL) {
        // vypocet adresy pro zapisovat
        adresa = 0;
        if (mft_seznam[file_uid] != NULL){
            mft_itemy = mft_seznam[file_uid];

            // iterace pres vsechny itemy pro dane UID souboru
            i = 0;
            while (mft_itemy != NULL){
                i++;

                // precte vsechny fragmenty z daneho mft itemu
                for (j = 0; j < MFT_FRAGMENTS_COUNT; j++){
                    mftf_adr = mft_itemy->item.fragments[j].fragment_start_address;

                    // nalezeni posledniho fragmentu s adresou
                    if (mftf_adr > 0 && mft_itemy->item.fragments[j].fragment_count > 0) {
                        adresa = mftf_adr;
                    }
                }

                // prehodi se na dalsi prvek
                mft_itemy = mft_itemy->next;
            }
        }

        if (adresa != 0){
            // nacte obsah daneho clusteru
            strcpy(ret, get_cluster_content(adresa));

            // pripoji k nemu potrebne
            if (dir == 1)
                strcat(ret, "\n");

            strcat(ret, append);
            delka = strlen(ret);

            fseek(fw, adresa, SEEK_SET);
            fwrite(ret, 1, delka, fw);

            // aktualizace vsech mft
            update_filesize(file_uid, delka);
        }
        else {
            return -1;
        }

        fclose(fw);
    }

    return 1;
}

/*
    Zmeni obsah souboru - rozsireni i ubrani
    @param file_uid Soubor pro editaci
    @param text Cely (novy) obsah
    @param filename Nazev souboru
    @param puvodni_uid Puvodni UID souboru pokud ma byt zachovano
 */
void edit_file_content(int file_uid, char *text, char *filename, int puvodni_uid){
    delete_file(file_uid);
    make_file(pwd, filename, text, puvodni_uid, 1, 0);
}

/*
    Vytvori MFT info o souboru
*/
int make_file_in_mft(FILE *fw, int volne_uid, char *filename, char *text, mft_fragment fpom[], int fpom_size, int is_dir) {
    int i, j, k, l, adresa_mfti;
    mft_item *mpom, *mff;

    mff = malloc(sizeof(mft_item));

    // realny pocet fragmentu
    int number_frugments = 0;
    int prvku = (fpom_size / sizeof(mft_fragment));
    for (i = 0; i < prvku; i++) {
        if (fpom[i].fragment_start_address != -1) {
            number_frugments++;
        }
    }

    int potreba_mfti = (number_frugments / MFT_FRAGMENTS_COUNT) + (number_frugments % MFT_FRAGMENTS_COUNT);
    int sizeof_mft_item = sizeof(mft_item);

    // mfti pole o spravne velikosti
    k = 0;
    mft_item mfti[potreba_mfti];
    for (i = 0; i < potreba_mfti; i++) {
        mfti[i].uid = volne_uid;
        mfti[i].isDirectory = is_dir;
        mfti[i].item_order = i + 1;
        mfti[i].item_order_total = potreba_mfti;
        strcpy(mfti[i].item_name, filename);
        mfti[i].item_size = strlen(text);

        // kazdemu prvku fragmenty
        for (j = 0; j < MFT_FRAGMENTS_COUNT; j++) {
            mfti[i].fragments[j] = fpom[k];

            // vkladani textu souboru
            text = set_fragment_content(fpom[k], text);
            k++;
        }

        // ulozeni virtualne
        add_mft_item(volne_uid, mfti[i]);

        // pridam ho do souboru
        mpom = malloc(sizeof(mft_item));
        mpom = &mfti[i];

        // nalezeni adresy v MFT bloku v souboru
        for (l = 0; l < CLUSTER_COUNT; l++) {
            adresa_mfti = boot_rec->mft_start_address + (l * sizeof_mft_item);

            fseek(fw, adresa_mfti, SEEK_SET);
            fread(mff, sizeof_mft_item, 1, fw);

            if (mff->uid == UID_ITEM_FREE || strcmp(mff->item_name, "") == 0) {

                fseek(fw, adresa_mfti, SEEK_SET);
                fwrite(mpom, sizeof(mft_item), 1, fw);

                break;
            }
        }
    }

    return 1;
}


/* Nacte NTFS ze souboru */
void load_ntfs(){
    FILE *fr;
    int i;
	int number_mft_blocks;
    mft_item mft_Item;

    fr = fopen(ntfs_file, "rb");
    if (fr == NULL) {
    	 printf("Soubor %s nebyl nalezen.\n", ntfs_file);
    	 exit(1);
    }

	// nactu data ze souboru
	boot_rec = malloc(sizeof(boot_record));

	printf("\tSoubor %s byl uspesne otevren\n", ntfs_file);

	// precteni boot recordu
	fread(boot_rec, sizeof(boot_record), 1, fr);

	printf("BOOT RECORD:\n");
	printf("	descriptor: %s\n", boot_rec->volume_descriptor);
	printf("	Celkova velikost FS: %d\n", boot_rec->disk_size);
	printf("	Velikost clusteru: %d\n", boot_rec->cluster_size);
/*	printf("	Pocet clusteru: %d\n", boot_rec->cluster_count);
	printf("	Adresa pocatku bitmapy: %d\n", boot_rec->bitmap_start_address);*/

	// nacteni bitmapy
	fseek(fr, boot_rec->bitmap_start_address, SEEK_SET);
	fread(ntfs_bitmap, sizeof(int32_t), boot_rec->cluster_count, fr);

/*	printf("	Adresa pocatku mft: %d\n", boot_rec->mft_start_address);*/

	number_mft_blocks = (boot_rec->bitmap_start_address - boot_rec->mft_start_address) / sizeof(mft_item);

	// nacteni neprazdnych mft polozek
	for (i = 0; i < number_mft_blocks; i++) {
		fseek(fr, boot_rec->mft_start_address + i *sizeof(mft_item), SEEK_SET);
		fread(&mft_Item, sizeof(mft_item), 1, fr);

		if (ntfs_bitmap[i] != 0 && mft_Item.uid != UID_ITEM_FREE) {
			add_mft_item(mft_Item.uid, mft_Item);

			if (i == 0) {
				pwd = mft_Item.uid;
			}
		}
	}

/*	printf("	tAdresa pocatku datoveho bloku: %d\n", boot_rec->data_start_address);*/

	fclose(fr);

    printf("Nacten FS: %s.\n", ntfs_file);
}

/* Zalozi pseudoNFTS obsahujici adresar ROOT_DIR */
void testing_ntfs(char filename[]){
    FILE *fw;
    boot_record *boot_Record;
    mft_item *mft_Item;

    int i;
	int bitmap[CLUSTER_COUNT];
    /* Vypocty celikosti v FS */
    int all_fragments_size = CLUSTER_COUNT * sizeof(mft_fragment);
    int all_clusters_size = CLUSTER_COUNT * sizeof(mft_item);
    int bitmap_size = sizeof(int32_t) * CLUSTER_COUNT;
    int bitmap_start = sizeof(boot_record) + all_fragments_size + all_clusters_size;
    int data_start = bitmap_start + bitmap_size;

    fw = fopen(filename, "wb");
    if (fw != NULL) {
        /* Zapis boot recordu */
        boot_Record = malloc(sizeof(boot_record));

        strcpy(boot_Record->volume_descriptor, "PseudoNTFS");
        boot_Record->disk_size = CLUSTER_SIZE * CLUSTER_COUNT;
        boot_Record->cluster_size = CLUSTER_SIZE;
        boot_Record->cluster_count = CLUSTER_COUNT;
        boot_Record->mft_start_address = sizeof(boot_record);
        boot_Record->bitmap_start_address = bitmap_start;
        boot_Record->data_start_address = data_start;
        boot_Record->mft_max_fragment_count = 1;

        fwrite(boot_Record, sizeof(boot_record), 1, fw);

        fseek(fw, bitmap_start, SEEK_SET);
        bitmap[0] = 1;
        /* nulovani bitmapy */
        for (i = 1; i < CLUSTER_COUNT; i++){
            bitmap[i] = 0;
        }

        fwrite(bitmap, sizeof(int32_t), CLUSTER_COUNT, fw);

        /* Zapis ROOT_DIR do MFT, vydz prvni*/
        /* zacatek oblasti MFT */
        fseek(fw, sizeof(boot_record), SEEK_SET);

        mft_Item = malloc(sizeof(mft_item));

        mft_Item->uid = 0;
        mft_Item->isDirectory = 1;
        mft_Item->item_order = 1;
        mft_Item->item_order_total = 1;
        strcpy(mft_Item->item_name, "ROOT_DIR");
        mft_Item->item_size = 1;

        // zapis fragmentu ROOT_DIR
        mft_Item->fragments[0].fragment_start_address = data_start; // pocatek FS
        mft_Item->fragments[0].fragment_count = 1; // pocet clusteru ve FS od pocatku

        // nastaveni fragmentu na prazdne
        for (i = 1; i < MFT_FRAGMENTS_COUNT; i++){
            mft_Item->fragments[i].fragment_start_address = -1;
            mft_Item->fragments[i].fragment_count = -1;
        }

        fwrite(mft_Item, sizeof(mft_item), 1, fw);

        /* Zapis odkazu na nadrazeny adresar */
        fseek(fw, data_start, SEEK_SET);
        char odkaz[2];
        odkaz[0] = '0';
        odkaz[1] = '\0';
        fwrite(odkaz, 1, 2, fw);

        free((void *) mft_Item);
        free((void *) boot_Record);
        fclose(fw);
    }
}
