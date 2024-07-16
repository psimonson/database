/*
 * main - Simple database test program.
 *
 * Author: Philip R. Simonson
 * Date:   07/05/2024
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

/* Entry point for program.
 */
int main()
{
	short int c;

	db_init();
	printf("List of available commands:\na: Append\nr: Replace\np: Print\nl: Load\ns: Save\nn: New\nq: Quit\nEnter command: ");
	do {
		c = getc(stdin);
		getc(stdin);

		if(c == 'q' || c == 'Q') {
			break;
		}

		switch(c) {
			case 'a':
			{
				db_append();
				if(!db_geterrori()) {
					printf("Appended 32 more entries to database.\n");
				}
			}
			break;
			case 'r':
			{
				Database *db;
				//char *tmp;
				int id;

				db = db_get();
				if(db->data == NULL) {
					printf("No database entries.\n");
					break;
				}
				id = db_getcur();

				if(id < 0 || id >= db->data[db->count*(MAXDB-1)]->id) {
					printf("Invalid ID number.\n");
					break;
				}
				db_replace(id);
				if(!db_geterrori()) {
					printf("You replaced ID number %d!\n", id);
					++id;
					db_setcur(id);
				}
			}
			break;
			case 'p':
			{
				Database *db;
				int longest;
				int i;

				db = db_get();
				if(db->data == NULL) {
					printf("No database entries.\n");
					break;
				}

				longest = (int)strlen(db->data[i]->name);
				for(i = 0; i < db->data[db->count*(MAXDB-1)]->id; ++i) {
					if((int)strlen(db->data[i]->name) > longest) {
						longest = (int)strlen(db->data[i]->name);
					}
				}

				printf("%-15s %-*s %-15s\n", "ID", longest, "NAME", "STATUS");
				for(i = 0; i < db->data[db->count*(MAXDB-1)]->id; ++i) {
					db_print(longest, i);
				}
				break;
			}
			case 's':
			{
				char *tmp;

				(void)getstr(&tmp, "Enter filename: ");
				if(strlen(tmp) == 0) {
					printf("Nothing entered as input.\n");
					break;
				}
				db_save(tmp);
				printf("db: %s!\n", db_geterror());
				break;
			}
			case 'l':
			{
				char *tmp;

				(void)getstr(&tmp, "Enter filename: ");
				if(strlen(tmp) == 0) {
					printf("Nothing entered as input.\n");
					break;
				}
				db_load(tmp);
				printf("db: %s!\n", db_geterror());
				break;
			}
			case 'n':
			{
				db_free();
				db_init();
				printf("New database created!\n");
			}
			break;
			default:
			break;
		}

		printf("List of available commands:\na: Append\nr: Replace\np: Print\nl: Load\ns: Save\nn: New\nq: Quit\nEnter command: ");
	} while(1);

	db_free();
	return 0;
}
