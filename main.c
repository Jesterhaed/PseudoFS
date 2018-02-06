/*
 * main.c
 *
 *  Created on: 3. 2. 2018
 *      Author: deserteagle
 */
#include <stdio.h>
#include <string.h>
#include "ntfs.h"

void help() {
	printf("Pouziti:.\n");
	printf("	cp s1 s2 	Zkopiruje soubor s1 do umisteni s2.\n");
	printf("	mv s1 s2 	Presune soubor s1 do umisteni s2.\n");
	printf("	rm s1	 	Smaze soubor s1.\n");
	printf("	mkdir a1 	Vytvori adresar a1.\n");
	printf("	rmdir a1	Smaze prazdny adresar a1.\n");
	printf("	ls a1 		Vypise obsah adresare a1.\n");
	printf("	cat s1 		Vypise obsah souboru s1.\n");
	printf("	cd a1		Zmeni aktualni cestu do adresare a1.\n");
	printf("	pwd 		Vypise aktualni cestu.\n");
	printf("	info a1/s1 	Vypise informace o souboru/adresari s1/a1.\n");
	printf("	incp s1 s2 	Nahraje soubor s1 z pevneho disku do umisteni s2 v pseudo NTFS.\n");
	printf("	outcp s1 s2	Nahraje soubour s1 z pseudoNTFS do umisteni na pevnem disku.\n");
	printf("	load s1 	Nacte soubor z pevneho disku, ve kterem budou jednotlive prikazy a zacne je sekvencnae vykonavat.\n");
	printf("	q			Ukonceni a ulozeni NTFS.\n");
	return;
}

int main(int argc, char** argv) {
	char vstup[6];

	if (argc == 3) {
		printf("Vytvori se souborovy system: %s.\n", argv[2]);
		testovaci_ntfs(argv[2]);
		printf("Souborovy system %s vytvoren.\n", argv[2]);
		return 1;
	}

	if (argc != 2) {
		printf("Jako argument zadej souborovy system.");
		return -1;
	}

	nacti_ntfs(argv[1]);

	while (1) {

	printf("Zadej pozadavek: ");
	scanf("%6s",vstup);

	if (!strcmp(vstup,"cp")){
		kopiruj_soubor();
		}
	else if (!strcmp(vstup,"mv")){
		presun_soubor();
		}
	else if (!strcmp(vstup,"rm")){
		smaz_soubor();
		}
	else if (!strcmp(vstup,"mkdir")){
		vytvor_adr();
		}
	else if (!strcmp(vstup,"rmdir")){
		smaz_adr();
		}
	else if (!strcmp(vstup,"ls")){
		vypis_adr();
		}
	else if (!strcmp(vstup,"cat")){
		vypis_soubor();
		}
	else if (!strcmp(vstup,"cd")){
		zmen_adr();
		}
	else if (!strcmp(vstup,"pwd")){
		vypis_cestu();
		}
	else if (!strcmp(vstup,"info")){
		vypis_info();
		}
	else if (!strcmp(vstup,"incp")){
		nahraj_do_ntfs();
		}
	else if (!strcmp(vstup,"outcp")){
		nahraj_z_ntfs();
		}
	else if (!strcmp(vstup,"load")){
		nacti_soubor();
		}
	else if (!strcmp(vstup,"q")){
		printf("Ukonceni aplikace.\n");
		break;
		}
	else {
		help();
		}
	}

	uloz_ntfs(argv[1]);
	uvolni_ntfs();


	return 0;
}
