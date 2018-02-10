#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <math.h>

#include "ntfs.h"
#include "enhanced_functions.h"
#include "extend_functions.h"

extern int pwd;
extern char ntfs_file[255];
char path1[255];
char path2[255];


/*
    Zkopiruje s1 do s2
*/
void copy_file() {
    char *nazev_zdroj, *nazev_cil, *jen_cesta_zdroj, *jen_cesta_cil;
    int delka, ret_zdroj, ret_cil, zdroj_uid;

    scanf("%255s",path1);
    scanf("%255s",path2);

    // path 1
            nazev_zdroj = strrchr(path1, '/');
            // absolutni adresa
            if (nazev_zdroj != NULL) {
                nazev_zdroj++;

                delka = strlen(path1) - strlen(nazev_zdroj);
                jen_cesta_zdroj = (char *) malloc(delka * sizeof(char *));
                strncpy(jen_cesta_zdroj, path1, delka);
                jen_cesta_zdroj[delka] = '\0';

                ret_zdroj = parcing_path(jen_cesta_zdroj, 1);
            }
            // relativni adresa
            else {
                delka = strlen(path1);

                nazev_zdroj = path1;

                jen_cesta_zdroj = (char *) malloc(1);
                strncpy(jen_cesta_zdroj, "/", 1);
                jen_cesta_zdroj[1] = '\0';

                ret_zdroj = pwd;
            }

            zdroj_uid = get_uid_by_name(nazev_zdroj, ret_zdroj);


    if (ret_zdroj == -1 || zdroj_uid == -1) {
        printf("FILE NOT FOUND\n");
        return;
    }

    // path 2
            nazev_cil = strrchr(path2, '/');
            if (nazev_cil != NULL) {
                nazev_cil++;

                delka = strlen(path2) - strlen(nazev_cil);
                jen_cesta_cil = (char *) malloc(delka);
                strncpy(jen_cesta_cil, path2, delka);
                jen_cesta_cil[delka] = '\0';

                ret_cil = parcing_path(jen_cesta_cil, 1);
            }
            else {
                delka = strlen(path2);

                nazev_cil = path2;

                jen_cesta_cil = (char *) malloc(1);
                strncpy(jen_cesta_cil, "/", 1);
                jen_cesta_cil[1] = '\0';

                ret_cil = pwd;
            }

    if (ret_cil == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    // nacteni obsahu kopirovaneho souboru
    char *content_copy_file = get_file_content(zdroj_uid);

    // novy soubor
    make_file(ret_cil, nazev_cil, content_copy_file, -1, 0, 1);

    printf("OK\n");
}

/*
    Presune s1 do s2
*/
void move_file(){
    int i = 0;
    int ret_zdroj, ret_cil, delka, zdroj_uid;
    char *nazev_zdroj, *nazev_cil, *jen_cesta_zdroj, *jen_cesta_cil;
    char buffer[CLUSTER_SIZE];
    char pom2[100];
    mft_item *mpom;
    int mft_size = sizeof(mft_item);
    FILE *fw;

    scanf("%255s",path1);
    scanf("%255s",path2);

    // path 1
            nazev_zdroj = strrchr(path1, '/');
            if (nazev_zdroj != NULL) {
                nazev_zdroj++;

                delka = strlen(path1) - strlen(nazev_zdroj);
                jen_cesta_zdroj = (char *) malloc(delka);
                strncpy(jen_cesta_zdroj, path1, delka);
                jen_cesta_zdroj[delka] = '\0';

                ret_zdroj = parcing_path(jen_cesta_zdroj, 1);
            }
            else {
                delka = strlen(path1);

                nazev_zdroj = path1;

                jen_cesta_zdroj = (char *) malloc(1);
                strncpy(jen_cesta_zdroj, "/", 1);
                jen_cesta_zdroj[1] = '\0';

                ret_zdroj = pwd;
            }

            zdroj_uid = get_uid_by_name(nazev_zdroj, ret_zdroj);

    // path 2
            nazev_cil = strrchr(path2, '/');
            if (nazev_cil != NULL) {
                nazev_cil++;

                delka = strlen(path2) - strlen(nazev_cil);
                jen_cesta_cil = (char *) malloc(delka);
                strncpy(jen_cesta_cil, path2, delka);
                jen_cesta_cil[delka] = '\0';

                ret_cil = parcing_path(jen_cesta_cil, 1);
            }
            else {
                delka = strlen(path2);

                nazev_cil = path2;

                jen_cesta_cil = (char *) malloc(1);
                strncpy(jen_cesta_cil, "/", 1);
                jen_cesta_cil[1] = '\0';

                ret_cil = pwd;
            }

    // odstrani odkaz z nadrazeneho adresare
    char *soucasny_obsah_zdroj = get_file_content(ret_zdroj);

    char *curLine = soucasny_obsah_zdroj;
    i = 0;
    strcpy(buffer, "");
    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';

        if (atoi(curLine) != zdroj_uid){
            if (i != 0)
                strcat(buffer, "\n");

            strcat(buffer, curLine);
        }

        if (nextLine) *nextLine = '\n';
        curLine = nextLine ? (nextLine + 1) : NULL;
        i++;
    }

    // UID se musi zachovat kvuli linkum
    edit_file_content(ret_zdroj, buffer, mft_seznam[ret_zdroj]->item.item_name, ret_zdroj);


    // zapis odkazu na soubor do noveho nadrazeneho adresare
    sprintf(pom2, "%d", zdroj_uid);
    append_file_content(ret_cil, pom2, 1);

    // prejmenovani
    if (strcmp(nazev_zdroj, nazev_cil) != 0) {

        strcpy(mft_seznam[zdroj_uid]->item.item_name, nazev_cil);

        mpom = malloc(mft_size);

        // zapis mft
        mpom = &mft_seznam[zdroj_uid]->item;
        fw = fopen(ntfs_file, "r+b");
        if (fw != NULL) {
            fseek(fw, boot_rec->mft_start_address + zdroj_uid * mft_size, SEEK_SET);
            fwrite(mpom, mft_size, 1, fw);
        }
        free((void *) mpom);
    }
    printf("OK\n");
}

/*
    Smaze soubor s1
*/
void remove_file(){
    int ret, i, delka, kesmazani;
    char buffer[CLUSTER_SIZE];
    char *nazev;
    char *jen_cesta;
    scanf("%255s",path1);

    // priprava cesty a nazev souboru pro smazani
    nazev = strrchr(path1, '/');
    if (nazev != NULL) {
        nazev++;

        delka = strlen(path1) - strlen(nazev);
        jen_cesta = (char *) malloc(delka);
        strncpy(jen_cesta, path1, delka);
        jen_cesta[delka] = '\0';

        ret = parcing_path(jen_cesta, 1);
    }
    else {
        delka = strlen(path1);
        nazev = (char *) malloc(delka);
        jen_cesta = (char *) malloc(delka);

        strncpy(nazev, path1, delka);
        nazev[delka] = '\0';

        strncpy(jen_cesta, "/", 1);
        jen_cesta[1] = '\0';

        ret = pwd;
    }

    kesmazani = parcing_path(path1, 1);

    if (ret == -1 || kesmazani == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    if (mft_seznam[kesmazani]->item.isDirectory == 1){
        printf("NOT A FILE\n");
        return;
    }

    // odstranim odkaz z nadrazeneho adresare
    char *soucasny_obsah = get_file_content(ret);

    char *curLine = soucasny_obsah;

    i = 0;
    strcpy(buffer, "");
    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';

        if (atoi(curLine) != kesmazani){
            if (i != 0)
                strcat(buffer, "\n");

            strcat(buffer, curLine);
        }

        if (nextLine) *nextLine = '\n';
        curLine = nextLine ? (nextLine + 1) : NULL;
        i++;
    }

    edit_file_content(ret, buffer, mft_seznam[ret]->item.item_name, ret);

    delete_file(kesmazani);

    free((void *) jen_cesta);
    printf("OK\n");
}


/*
    Vytvori adresar a1
*/
void make_dir(){
    int ret, delka;
    char *nazev;
    char *jen_cesta;

    scanf("%255s",path1);

    // priprava cesty a nazev souboru pro vytvoreni
    nazev = strrchr(path1, '/');
    if (nazev != NULL) {
        nazev++;

        delka = strlen(path1) - strlen(nazev);
        jen_cesta = (char *) malloc(delka);
        strncpy(jen_cesta, path1, delka);
        jen_cesta[delka] = '\0';

        ret = parcing_path(jen_cesta, 1);
    }
    else {
        delka = strlen(path1);

        nazev = path1;

        jen_cesta = (char *) malloc(1);
        strncpy(jen_cesta, "/", 1);
        jen_cesta[1] = '\0';

        ret = pwd;
    }

    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }
    else {
        // tvorba slozky
        // z bitmapy prvni volny cluster a vypocet adresu, fragment_count 1
        // do prvniho fragmentu polozky mft_seznam[ret]->item - UID noveho adresare
        make_new_file(ret, nazev);
    }


    free((void *) jen_cesta);

    printf("OK\n");
}


/*
    Smaze prazdny adresar
*/
void remove_dir(){
    int ret, i;
    char buffer[CLUSTER_SIZE];
    scanf("%255s",path1);

    ret = parcing_path(path1, 1);


    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    if (mft_seznam[ret]->item.isDirectory == 0) {
        printf("NOT A DIRECTORY\n");
        return;
    }

    if (is_empty_dir(ret) > 1) {
        printf("NOT EMPTY\n");
        return;
    }

    // odstrani odkaz z nadrazeneho adresare
    char *soucasny_obsah = get_file_content(pwd);

    char *curLine = soucasny_obsah;

    i = 0;
    strcpy(buffer, "");
    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';

        if (atoi(curLine) != ret){
            if (i != 0)
                strcat(buffer, "\n");

            strcat(buffer, curLine);
        }

        if (nextLine) *nextLine = '\n';
        curLine = nextLine ? (nextLine + 1) : NULL;
        i++;
    }

    edit_file_content(pwd, buffer, mft_seznam[pwd]->item.item_name, pwd);

    // smaze pozadovany soubor na disku
    delete_file(ret);

    printf("OK\n");
}


/*
    Vypise obsah adresare
*/
void list_dir(){
    int ret;
    scanf("%255s",path1);

    // zkusim si tu cestu projit
    if (!strcmp(path1,".")){
        ret = parcing_path("", 1);
    }
    else {
        ret = parcing_path(path1, 1);
    }

    // cesta neexistuje, nelze splnit pozadavek
    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    ls_printer(ret);
}

/*
    Vypise obsah souboru
*/
void concatenate(){
    int ret;
    scanf("%255s",path1);

    ret = parcing_path(path1, 1);

    // cesta neexistuje, nelze splnit pozadavek
    if (ret == -1){
        printf("FILE NOT FOUND\n");
        return;
    }

    // je posledni uid soubor a ne slozka
    if (mft_seznam[ret]->item.isDirectory == 1){
        printf("FILE NOT FOUND\n");
        return;
    }

    printf("%s\n", get_file_content(ret));
}

/*
    Posun v adresarich
*/
void change_dir(){
    int kam;
    scanf("%255s",path1);

    kam = parcing_path(path1, 1);

    if (kam != -1){
        pwd = kam;
        printf("OK\n");
    }
    else {
        printf("PATH NOT FOUND\n");
        return;
    }
}

/*
    Print working directory
*/
void print_dir(){
    char link[20], full_link[200], pom[200];
    int link_int;

    if (pwd == 0) {
        printf("/\n");
    }
    else if (pwd >= 0) {
        link_int = pwd;

        while (link_int > 0) {
            strcpy(link, "/");
            strcat(link, mft_seznam[link_int]->item.item_name);

            strcpy(pom, full_link);
            strcpy(full_link, link);
            strcat(full_link, pom);

            // backlink dalsiho adresare v poradi
            link_int = get_backlink(link_int);
        }

        printf("%s\n", full_link);
    }
}

/*
    Vypise informace z FS o danem souboru
*/
void read_info(){
	 scanf("%255s",path1);
    int ret, j, k, adr;
    mft_item mfti;
    mft_list* mft_itemy;
    mft_fragment mftf;

    ret = parcing_path(path1, 1);

    if (ret == -1) {
        printf("FILE NOT FOUND\n");
        return;
    }

    mfti = mft_seznam[ret]->item;

    printf("NAME - UID - SIZE\n");
    printf("%s - %d - %d\n", mfti.item_name, mfti.uid, mfti.item_size);

    printf("FRAGMENTY & CLUSTERY:\n");

    if (mft_seznam[ret] != NULL){
        mft_itemy = mft_seznam[ret];

        // projede vsechny itemy pro dane UID souboru
        k = 0; // celkovy pocet zopracovanych neprazdnych fragmentu
        while (mft_itemy != NULL){
            mfti = mft_itemy->item;

            // precte vsechny fragmenty z daneho mft itemu
            for (j = 0; j < MFT_FRAGMENTS_COUNT; j++){
                mftf = mfti.fragments[j];

                if (mftf.fragment_start_address != 0 && mftf.fragment_count > 0) {
                    k++;
                    adr = (mftf.fragment_start_address - boot_rec->data_start_address) / boot_rec->cluster_size;
                    printf("-- Fragment start=%d, count=%d, clusterID=%d\n", mftf.fragment_start_address, mftf.fragment_count, adr);
                }
            }

            // prehodi se na dalsi prvek
            mft_itemy = mft_itemy->next;
        }
    }

    printf("Pocet fragmentu: %d\n", k);
}

/*
    Nahraje soubor z pevneho disku do FS
*/
void in_copy(){
    int ret, delka;
    FILE *f;
    char pc_file[100];
    char *nazev;
    char *jen_cesta;

    scanf("%255s",path1);
    scanf("%255s",path2);


	// soubor k presunu z pocitace
	strncpy(pc_file, path1, strlen(path1));
	pc_file[strlen(path1)] = '\0';
	f = fopen(pc_file, "r");
	if (f == NULL){
		printf("FILE NOT FOUND\n");
		return;
	}
	// cilove misto, priprava cesty a nazev souboru pro vytvoreni
	nazev = strrchr(path2, '/');
	if (nazev != NULL) {
		nazev++;
		delka = strlen(path2) - strlen(nazev) - 1;

		jen_cesta = (char *) malloc(delka * sizeof(char *));
		strncpy(jen_cesta, path2, delka);
		jen_cesta[delka] = '\0';

		ret = parcing_path(jen_cesta, 1);
	}
	else {
		delka = strlen(path2);
		nazev = path2;

		jen_cesta = (char *) malloc(delka);
		strncpy(jen_cesta, "/", 1);
		jen_cesta[1] = '\0';

		ret = pwd;
	}

	if (ret == -1){
		printf("PATH NOT FOUND\n");
		return;
	}

    make_file(ret, nazev, read_file_from_pc(pc_file), -1, 0, 1);

    free((void *) jen_cesta);
    printf("OK\n");
}

/*
    Nahraje soubor do PC
*/
void out_copy(){
    int ret;
    FILE *fw;
    char *obsah;

    scanf("%255s",path1);
    scanf("%255s",path2);

    // path 1, k presunu z FS
    ret = parcing_path(path1, 1);

    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    obsah = get_file_content(ret);


    // path 2, ulozeni souboru do pc

    fw = fopen(path2, "w");
    if (fw == NULL){
        printf("FILE NOT FOUND\n");
        return;
    }

    fwrite(obsah, 1, strlen(obsah), fw);

    fclose(fw);

    printf("OK\n");
}

/*
    Defragmentace
    Soubory se budou skladat pouze z jednoho fragmentu
*/
void defragmentation(){
    int i, j, k, clusteru, p_clusteru, zpracovany, adresa;
    int nova_bitmapa[CLUSTER_COUNT];
    FILE *fw;

    // zaloh ntfs
    char *old = (char *) malloc(100);
    char *new = (char *) malloc(100);
    strcpy(new, ntfs_file);
    strcat(new, ".b");
    strcat(new, "\0");
    strcpy(old, ntfs_file);
    strcat(old, "\0");

    if (rename(ntfs_file, new) == 0) {
        printf("VYTVARIM ZALOZNI SOUBOR .bak\n");
    }

    // provede prislusne operace defragmentace
    fw = fopen(ntfs_file, "wb");
    if (fw != NULL) {
        zpracovany = 0; // zpracovany cluster
        for (i = 0; i < CLUSTER_COUNT; i++) {
            if (mft_seznam[i] != NULL){
                // soubor stoji za zpracovani
                printf("Zpracovavam soubor %s\n", mft_seznam[i]->item.item_name);

                // nactu si obsah souboru
                strcpy(ntfs_file, new);
                char *cely_soubor = get_file_content(i);
                strcpy(ntfs_file, old);

//               clusteru = ceil((double) strlen(cely_soubor) / CLUSTER_SIZE);

                // zapis do bitmapy
                for (j = zpracovany; j < zpracovany + clusteru; j++) {
                    nova_bitmapa[j] = 1;
                }

                // aktualizace mfti
                mft_seznam[i]->item.item_order = 1;
                mft_seznam[i]->item.item_order_total = 1;
                mft_seznam[i]->item.item_size = strlen(cely_soubor);

                // priprava fragmentu
                adresa = boot_rec->data_start_address + zpracovany * CLUSTER_SIZE;
                for (k = 0; k < MFT_FRAGMENTS_COUNT; k++) {
                    p_clusteru = clusteru;

                    // prvni fragment
                    if (k == 1) {
                        adresa = -1;
                        p_clusteru = -1;
                    }
                    else {
                            fseek(fw, adresa, SEEK_SET);
                            fwrite(cely_soubor, strlen(cely_soubor), 1, fw);
                    }

                    mft_seznam[i]->item.fragments[k].fragment_start_address = adresa;
                    mft_seznam[i]->item.fragments[k].fragment_count = p_clusteru;
                }

                // zrusi odkaz na dalsi prvek pameti
                mft_seznam[i]->next = NULL;

                zpracovany += clusteru;
            }
        }
        fclose(fw);
    }

    // novy soubor
    fw = fopen(ntfs_file, "r+b");
    if (fw != NULL) {
        /* Zapis boot record */
        fseek(fw, 0, SEEK_SET);
        fwrite(boot_rec, sizeof(boot_record), 1, fw);

        /* Zapis startovaci bitmapy */
        fseek(fw, boot_rec->bitmap_start_address, SEEK_SET);
        fwrite(nova_bitmapa, 4, CLUSTER_COUNT, fw);

        /* Zapis MFT */
        adresa = boot_rec->mft_start_address;
        for (i = 0; i < CLUSTER_COUNT; i++) {
            if (mft_seznam[i] != NULL){
                fseek(fw, adresa, SEEK_SET);
                fwrite(&mft_seznam[i]->item, sizeof(mft_item), 1, fw);

                adresa += sizeof(mft_item);
            }
        }

        fclose(fw);
    }

    free((void *) new);
    free((void *) old);

    printf("OK\n");
}

/*
    Kontrola konzistence
    -> kontrola celistvosti souboru (velikost souboru odpovida poctu alokovanych datovych bloku)
 */
void check_consistency(){
    int i, rc;
    int pocet_vlaken = 2;

    pthread_t pt[pocet_vlaken];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    share_memory memory;

    memory.mutex = &mutex;
    memory.zpracovany_cluster = -1;

    // start vlaken
    for (i = 0; i < pocet_vlaken; i++) {
        rc = pthread_create(&pt[i], NULL, check_consist, (void *) &memory);
        assert(0 == rc);
    }

    // Cekani na dokonceni vsech vlaken
    for (i = 0; i < pocet_vlaken; i++) {
        rc = pthread_join(pt[i], NULL);
        assert(0 == rc);
    }

    printf("OK\n");
}
