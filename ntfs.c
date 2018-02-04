/*
 * ntfs.c
 *
 *  Created on: 4. 2. 2018
 *      Author: deserteagle
 */
#include <stdio.h>
#include <string.h>

char adresa1[255];
char adresa2[255];

int nacti_ntfs(char* ntfs) {
	printf("Nacten FS: %s.\n", ntfs);
	return 1;
}

int kopiruj_soubor() {
	scanf("%255s",adresa1);
	scanf("%255s",adresa2);
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
