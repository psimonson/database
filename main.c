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
#include "prs/network.h"
#include "prs/readln.h"
#include "database.h"
#include "netprintf.h"

/* Entry point for program.
 */
int main()
{
	short unsigned int port = 8888;
	size_t linecap;
	SOCKET s, c;
	short int ch;

	s = server_socket_open(&port);
	if(s == INVALID_SOCKET) {
		fprintf(stderr, "Error: Cannot open server socket.\n");
		return 1;
	}
	printf("Listening on 0.0.0.0 port %hu...\n", port);

	c = server_socket_accept(s);
	if(c == INVALID_SOCKET) {
		fprintf(stderr, "Error: Cannot accept client.\n");
		socket_close(s);
		return 1;
	}

	db_init();
	socket_printf(c, "List of available commands:\r\na: Append\r\ni: Insert\r\nr: Replace\r\np: Print\r\nl: Load\r\ns: Save\r\nn: New\r\nq: Quit\r\nEnter command: ");
	do {
		char *line;

		(void)readln(&line, &linecap, c);
		ch = line[0];

		if(ch == 'q' || ch == 'Q') {
			break;
		}

		switch(ch) {
			case 'a':
			{
				db_append();
				if(!db_geterrori()) {
					socket_printf(c, "Appended %d more entries to database.\r\n", MAXDB);
				}
			}
			break;
			case 'i':
			{
				Database *db;
				int id;

				db = db_get();
				if(db->data == NULL) {
					socket_printf(c, "No database entries.\r\n");
					break;
				}
				id = db_getcur();

				if(id < 0 || id >= ((struct DatabaseData *)db->data)[db->count*(MAXDB-1)].id) {
					socket_printf(c, "Invalid ID number.\r\n");
					break;
				}

				db_replace(c, id);

				if(!db_geterrori()) {
					socket_printf(c, "You replaced ID number %d!\r\n", id);
					++id;
					db_setcur(id);
				}
			}
			break;
			case 'r':
			{
				Database *db;
				char *tmp;
				int id;

				db = db_get();
				if(db->data == NULL) {
					socket_printf(c, "No database entries.\r\n");
					break;
				}

				socket_printf(c, "Enter ID: ");
				(void)readln(&tmp, &linecap, c);
				id = atoi(tmp);
				free(tmp);

				if(id < 0 || id >= ((struct DatabaseData *)db->data)[db->count*(MAXDB-1)].id) {
					socket_printf(c, "Invalid ID number.\r\n");
					break;
				}

				db_replace(c, id);

				if(!db_geterrori()) {
					socket_printf(c, "You replaced ID number %d!\r\n", id);
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
					socket_printf(c, "No database entries.\r\n");
					break;
				}

				longest = (int)strlen(((struct DatabaseData *)db->data)[i].name);
				for(i = 0; i < ((struct DatabaseData *)db->data)[db->count*(MAXDB-1)].id; ++i) {
					if((int)strlen(((struct DatabaseData *)db->data)[i].name) > longest) {
						longest = (int)strlen(((struct DatabaseData *)db->data)[i].name);
					}
				}

				socket_printf(c, "%-15s %-*s %-15s\r\n", "ID", longest, "NAME", "STATUS");
				for(i = 0; i < ((struct DatabaseData *)db->data)[db->count*(MAXDB-1)].id; ++i) {
					db_print(c, longest, i);
				}
				break;
			}
			case 's':
			{
				char *tmp;

				socket_printf(c, "Enter filename: ");
				(void)readln(&tmp, &linecap, c);
				if(tmp == NULL) {
					printf("Nothing entered as input.\n");
					break;
				}
				db_save(tmp);
				printf("db: %s!\n", db_geterror());
				free(tmp);
				break;
			}
			case 'l':
			{
				char *tmp;

				socket_printf(c, "Enter filename: ");
				(void)readln(&tmp, &linecap, c);
				if(tmp == NULL) {
					socket_printf(c, "Nothing entered as input.\r\n");
					break;
				}
				db_load(tmp);
				socket_printf(c, "db: %s!\n", db_geterror());
				free(tmp);
				break;
			}
			case 'n':
			{
				db_free();
				socket_printf(c, "New database created!\r\n");
			}
			break;
			default:
			break;
		}

		socket_printf(c, "List of available commands:\r\na: Append\r\ni: Insert\r\nr: Replace\r\np: Print\r\nl: Load\r\ns: Save\r\nn: New\r\nq: Quit\r\nEnter command: ");
	} while(1);

	socket_close(c);
	socket_close(s);
	db_free();
	return 0;
}
