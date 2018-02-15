#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "enhanced_functions.h"
#include "ntfs.h"
#include "extend_functions.h"

extern int pwd;
extern char ntfs_file[255];

/*
    Vrati volne UID pro pojmenovani souboru
*/
int get_free_uid() {
    int i;

    // prochazi potencionalni list a hleda prvni volny index
    for (i = 0; i < CLUSTER_COUNT; i++) {
        if (mft_seznam[i] == NULL){
            return i;
        }
    }

    return -1;
}

/*
    Prelozi jmeno adresare na UID
     - nacte si obsah clusteru soucasneho adresare a prochazi vsechny tyto adresare a hleda shodnost jmena
    @param dir_name Jmeno adresare pro preklad
    @param uid_pwd Aktualni slozka
*/
int get_uid_by_name(char *dir_name, int uid_pwd){
    mft_item mfti;
    int hledane, i;

    char *curLine = get_file_content(uid_pwd);


    // obsah clusteru daneho adresare po radcich - jeden radek, UID jednoho souboru nebo slozky
    i = 0;
    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';

        if (i != 0){
            // skip prvni radky v clusteru (backlink)
            hledane = atoi(curLine);

            // rozparsovani MFT a shoda nazvu
            if (hledane < CLUSTER_COUNT && mft_seznam[hledane] != NULL){
                mfti = mft_seznam[hledane]->item;


                if (strncmp(mfti.item_name, dir_name, strlen(mfti.item_name)) == 0 && strlen(mfti.item_name) == strlen(dir_name)) {
                    return mfti.uid;
                }
            }
        }
        else {
            // ../../ relativni cesty

            if (strncmp(dir_name, "..", 2) == 0){
                return atoi(curLine);
            }
        }

        if (nextLine) *nextLine = '\n';
        curLine = nextLine ? (nextLine + 1) : NULL;

        i++;
    }

    return -1;
}

/*
    Zkontroluje unikatnost jmena v souboru
    @param newname Nazev souboru
    @param uid_pwd Pracovni adresar, ve kterem hleda
*/
int is_name_unique(char *newname, int uid_pwd){
    if (get_uid_by_name(newname, uid_pwd) == -1) {
        return 1;
    }

    return 0;
}

/*
    Vrati odkaz na rodicovksou slozku
    @param uid_pwd Soucasna slozka
*/
int get_backlink(int uid_pwd) {
    char *curLine = get_file_content(uid_pwd);

    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';

        return atoi(curLine);
    }

    return -1;
}

/*
    Prochazi danou cestu a vrati UID slozky, ktera je posledni nebo -1 pri chybe
    @param patha Cela cesta v FS (muze byt i relativni)
    @param cd Pouziva se jen pro funkci cd...
*/
int parcing_path(char *patha, int cd){
    char *p_c;
    int start_dir;
    char path[100];

    strncpy(path, patha, 100); // bez \0

    // root?
    if (strncmp(path, "/", 1) == 0){
        start_dir = 0;
    }
    else {
        start_dir = pwd;
    }

    if (strcmp(patha, "") != 0) {
        if (strchr(patha, '/') != NULL){
            // parsovani cesty od / do posledniho adr
            p_c = strtok(path, "/");
            while( p_c != NULL ) {
                start_dir = get_uid_by_name(p_c, start_dir); // nazev na UID

                if (start_dir == -1) return -1;
                //

                p_c = strtok(NULL, "/");
            }
        }
        else {
        	// pouze pro prikaz cd
    	   if (cd == 1) {
                start_dir = get_uid_by_name(patha, start_dir);

                if (start_dir == -1) return -1;
            }
        }
    }
    return start_dir;
}


/*
    Vytvori novou slozku o zadanem nazvu
    @param pwd UID slozky kde ma slozku vytvorit
    @param name Jmeno slozky pro vytvoreni
*/
int make_new_file(int pwd, char *name){
    int i, j, bitmap_free_index, new_cluster_start, volne_uid;
    FILE *fw;
    mft_item mfti;
    mft_fragment mftf;
    mft_item *mpom;
    char pomocnik2[5], pom[5];

    if (is_name_unique(name, pwd) != 1){
    	printf("EXIST\n");
        return -1;
    }

    sprintf(pom, "%d", pwd);

    // najde volne UID
    volne_uid = get_free_uid();

    // najde volnou bitmapu
    bitmap_free_index = -1;
    for (i = 0; i < CLUSTER_COUNT; i++){
        if (ntfs_bitmap[i] == 0){
            bitmap_free_index = i;
            break;
        }
    }

    if (bitmap_free_index != -1){
        new_cluster_start = boot_rec->data_start_address + bitmap_free_index * CLUSTER_SIZE;

        // prvni volny MFT LIST a volne UID pro novou slozku
        for (i = 0; i < CLUSTER_COUNT; i++){
            if (mft_seznam[i] == NULL){
                // volny prcek
                mpom = malloc(sizeof(mft_item));

                mftf.fragment_start_address = new_cluster_start; // start adresa ve FS
                mftf.fragment_count = 1;

                mfti.uid = volne_uid;
                mfti.isDirectory = 1;
                mfti.item_order = 1;
                mfti.item_order_total = 1;
                strcpy(mfti.item_name, name);
                mfti.item_size = strlen(pom);
                mfti.fragments[0] = mftf;

                // dalsi fragmenty prazdne
                mftf.fragment_start_address = UID_ITEM_FREE;
                mftf.fragment_count = UID_ITEM_FREE;

                for (j = 1; j < MFT_FRAGMENTS_COUNT; j++){
                    mfti.fragments[j] = mftf;
                }


                // aktualizace globalni pole a bitmapa
                ntfs_bitmap[bitmap_free_index] = 1;
                add_mft_item(volne_uid, mfti);

                // zapis do souboru
                fw = fopen(ntfs_file, "r+b");
                if(fw != NULL){
                    mpom = &mfti;
                    fseek(fw, sizeof(boot_record) + volne_uid * sizeof(mft_item), SEEK_SET);
                    fwrite(mpom, sizeof(mft_item), 1, fw);

                    // bitmapa
                    fseek(fw, boot_rec->bitmap_start_address, SEEK_SET);
                    fwrite(ntfs_bitmap, 4, CLUSTER_COUNT, fw);

                    // odkaz na slozku do nadrazeneho adresare (zapis do clusteru)
                    sprintf(pomocnik2, "%d", volne_uid);
                    append_file_content(pwd, pomocnik2, 1);

                    // odkaz na nadrazenou slozku do teto slozky - backlink
                    fseek(fw, boot_rec->data_start_address + bitmap_free_index * CLUSTER_SIZE, SEEK_SET);
                    fwrite(pom, 1, strlen(pom), fw);

                    fclose(fw);
                }
                break;
            }
        }
    }
    printf("OK\n");
    return volne_uid;
}

/*
    Ziskani informaci o souborech ve slozce
*/
void ls_printer(int uid) {
    char *p_c;
    int i = 0;
    mft_item mfti;

    // obsah aktualniho adresare
    char *buffer = get_file_content(uid);

    printf("--- NAZEV ----- VELIKOST - UID ---\n");

    // iterace pro kazdou polozku z adresare
    p_c = strtok(buffer, "\n");

    // prvni odkaz je odkaz na nadrazenou slozku

    while((p_c = strtok(NULL, "\n")) != NULL){
        if (mft_seznam[atoi(p_c)] != NULL){
            mfti = mft_seznam[atoi(p_c)]->item;

            printf(" ");
            if (mfti.isDirectory == 1){
                printf("+");
            }
            else{
                printf("-");
            }
            printf(" %-15s %-7d %d\n", mfti.item_name, mfti.item_size, mfti.uid);
        }

        i++;
    }

    printf("-- Celkem souboru: %d --\n", i);
}

/*
    Zkontroluje jeslti je osubor prazdny
    -> vraci pocet radek v souboru
*/
int is_empty_dir(int file_uid) {
    char *curLine = get_file_content(file_uid);
    int i = 0;

    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';

        i++;

        if (nextLine) *nextLine = '\n';
        curLine = nextLine ? (nextLine + 1) : NULL;
    }

    return i;
}

/*
    Nacte cely obsah souboru z pc (externi FS)
*/
char* read_file_from_pc(char *pc_soubor){
    int size;
    FILE *fr;
    char *ret;
    fr = fopen(pc_soubor, "r");
    if (fr != NULL){
        // zjisteni delky souboru
        fseek(fr, 0, SEEK_END);
        size = ftell(fr);

        ret = (char *) malloc(size);

        // precte soubor do promenne
        fseek(fr, 0, SEEK_SET);
        fread(ret, 1, size + 1, fr);
        ret[size] = '\0';

        fclose(fr);
    }
    return ret;
}


/*
    Zapise soubor do FS
*/
void make_file(int target_file, char *filename, char *text, int puvodni_uid, int is_dir, int odkaz){
    int i, j, k, l, size, potreba_clusteru, volne_uid, spoj_len, starter, nasobic;
    FILE *fw;
    char pom[20];
    mft_fragment mftf;

    // delka textu
    size = strlen(text);

    // volne UID pro soubor
    if (puvodni_uid == -1) {
        volne_uid = get_free_uid();
    }
    else {
        volne_uid = puvodni_uid;
    }

    // potreba volnych clusteru
    potreba_clusteru = size / CLUSTER_SIZE + 1;
    int volne_clustery[potreba_clusteru];

    j = 0;
    for (i = 0; i < CLUSTER_COUNT; i++) {
        if (ntfs_bitmap[i] == 0) {
            // volna
            volne_clustery[j] = i;
            j++;
        }

        if (j == potreba_clusteru) {
            break;
        }
    }

    if (j != potreba_clusteru){
        printf("ERROR - NOT ENOUGH CLUSTERS (%d)\n", j);
        return;
    }

    // pomocne pole na fragmenty
    mft_fragment fpom[potreba_clusteru];

    // vynulovani
    for (i = 0; i < potreba_clusteru; i++) {
        fpom[i].fragment_start_address = UID_ITEM_FREE;
        fpom[i].fragment_count = UID_ITEM_FREE;
    }

    // otevreni spojeni s fs
    fw = fopen(ntfs_file, "r+b");
    if (fw != NULL) {
        // reseni spojitosti a nespojitosti bitmapy
        spoj_len = 1;
        starter = 0;
        l = 0;
        k = 0;
        for (j = 0; j < potreba_clusteru; j++) {
            if (volne_clustery[j+1] == volne_clustery[j]+1) {
                spoj_len = spoj_len + 1;

                if (spoj_len == 2){
                    starter = volne_clustery[j];
                }
            }
            else {
                // nasobic spojitosti volnych clusteru
                if (spoj_len != 1) {
                    nasobic = starter;
                }
                else {
                    nasobic = volne_clustery[j];
                }

                mftf.fragment_start_address = boot_rec->data_start_address + nasobic * CLUSTER_SIZE; // adresa do FS do clusteru
                mftf.fragment_count = spoj_len;
                fpom[k] = mftf;

                l++;
                k++;

                spoj_len = 1;
                starter = 0;
            }
        }

        if (spoj_len != 1) {

            mftf.fragment_start_address = boot_rec->data_start_address + starter * CLUSTER_SIZE;
            mftf.fragment_count = spoj_len;
            fpom[k] = mftf;

            l++;
            k++;
        }

        for (j = 0; j < potreba_clusteru; j++) {
            // aktualizace bitmapy v programu
            ntfs_bitmap[volne_clustery[j]] = 1;
        }

        // aktualizace bitmapy v souboru
        fseek(fw, boot_rec->bitmap_start_address, SEEK_SET);
        fwrite(ntfs_bitmap, 4, CLUSTER_COUNT, fw);

        if (make_file_in_mft(fw, volne_uid, filename, text, fpom, sizeof(fpom), is_dir) == -1) {
            printf("ERROR\n");
        }

        // odkaz na slozku do nadrazeneho adresare
        if (odkaz == 1) {
            sprintf(pom, "%d", volne_uid);
            append_file_content(target_file, pom, 1);
        }

        fclose(fw);
    }
}

/*
    Provede zakladni kontrolu konzistence
*/
void *check_consist(void *arg) {
    share_memory *param = (share_memory *) arg;
    int ke_zpracovani;

    while (1) {
        pthread_mutex_lock(param->mutex);
            ke_zpracovani = param->zpracovany_cluster + 1;
            param->zpracovany_cluster = ke_zpracovani;
        pthread_mutex_unlock(param->mutex);

        if (ke_zpracovani >= CLUSTER_COUNT) {
            break;
        }

        // zpracovani dat

        mft_list* mft_itemy;
        mft_item mfti;
        mft_fragment mftf;
        int j;
        int delka = 0;

        // vsechny mfti
        if (mft_seznam[ke_zpracovani] != NULL){
            mft_itemy = mft_seznam[ke_zpracovani];

            // iterace mfti
            while (mft_itemy != NULL){
                mfti = mft_itemy->item;

                // vsechny mftf
                for (j = 0; j < MFT_FRAGMENTS_COUNT; j++){
                    mftf = mfti.fragments[j];

                    if (mftf.fragment_start_address != 0 && mftf.fragment_count > 0) {
                        delka += strlen(get_fragment_content(mftf));
                    }
                }

                // dalsi prvek
                mft_itemy = mft_itemy->next;
            }

            //  vysledek
            printf("Soubor %s ", mft_seznam[ke_zpracovani]->item.item_name);
            if (delka != mft_seznam[ke_zpracovani]->item.item_size) {
                printf("NENI KONZISTENTNI (%d != %d) !!!\n", mft_seznam[ke_zpracovani]->item.item_size, delka);
            }
            else {
                printf("JE V PORADKU (%d == %d)\n", mft_seznam[ke_zpracovani]->item.item_size, delka);
            }
        }
    }

    return NULL;
}
