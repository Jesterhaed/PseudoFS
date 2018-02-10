#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extend_functions.h"
#include "ntfs.h"

char ntfs_file[255];
extern char path1[255];

void help() {
	printf("\nPouziti:.\n");
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
	printf("	defr		Defragmentace souboroveho systemu.\n");
	printf("	chk			Kontrola konzistence souboroveho systemu.\n");
	printf("	q		Ukonceni a ulozeni NTFS.\n");
	return;
}

int main(int argc, char *argv[]){
    int i;
    FILE *fr;
    char input[6];
    int check = 0;
    int counter = 0;
    int lines = -1;
	int stdin_copy = dup(0);	//kopie stdinu

    // 3 argumenty - jmeno druheho testovaci soubor
    if (argc == 3) {
		printf("Vytvori se souborovy system: %s.\n", argv[2]);
		testing_ntfs(argv[2]);
		printf("Souborovy system %s vytvoren.\n", argv[2]);
		return 1;
	}

	if (argc != 2) {
		printf("Jako argument zadej souborovy system.");
		return -1;
	}

    strcpy(ntfs_file, argv[1]);

    // mft seznam na null
    for (i = 0; i < CLUSTER_COUNT; i++) {
       mft_seznam[i] = NULL;
    }

    load_ntfs();

    while(1){
    		counter++;

            printf("Zadejte prikaz: ");
            scanf("%6s",input);

            if (!strcmp(input,"cp")){
				copy_file();
			}
			else if (!strcmp(input,"mv")){
				move_file();
			}
			else if (!strcmp(input,"rm")){
				remove_file();
			}
			else if (!strcmp(input,"mkdir")){
				make_dir();
			}
			else if (!strcmp(input,"rmdir")){
				remove_dir();
			}
			else if (!strcmp(input,"ls")){
				list_dir();
			}
			else if (!strcmp(input,"cat")){
				concatenate();
			}
			else if (!strcmp(input,"cd")){
				change_dir();
			}
			else if (!strcmp(input,"pwd")){
				print_dir();
			}
			else if (!strcmp(input,"info")){
				read_info();
			}
			else if (!strcmp(input,"incp")){
				in_copy();
			}
			else if (!strcmp(input,"outcp")){
				out_copy();
				}
			else if(!strcmp(input,"load")){
			    scanf("%255s",path1);
				fr = fopen(path1, "r");
				if (fr != NULL) {
					int ch;
					counter = 0;
					lines = 0;
					while(!feof(fr))
					{
					  ch = fgetc(fr);
					  if(ch == '\n')
					  {
					    lines++;
					  }
					}
					fr = freopen(path1, "r", stdin);
				}
				else {
					printf("FILE NOT FOUND\n");
					check = 1;
				}
			}
			else if(!strcmp(input, "defr")){
				defragmentation();
			}
			else if(!strcmp(input, "chk")){
				check_consistency();
			}
			else if (!strcmp(input,"q")){
				printf("Ukonceni aplikace.\n");
				break;
			}
			else {
				help();
			}

            // zavreni nacitani ze souboru (stdinu)
            if (check == 0 &&  counter == lines) {
            	lines = -1;
				close(0);
				dup2(stdin_copy, 0);
			}
        }

    free((void *) boot_rec);

    printf("Zavreni souboroveho systemu.\n");
    return 0;
}
