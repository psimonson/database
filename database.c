/*
 * database - Simple database with encryption.
 *
 * Author: Philip R. Simonson
 * Date:   07/05/2024
 *
 */

#define _DEFAULT_SOURCE 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

static Database db;

/* Get a string from the user, standard input.
 */
size_t getstr(char **buffer, const char *prompt)
{
	static char input[MAXBUF];
	size_t i;

	input[0] = '\0';

	if(prompt != NULL) {
		printf("%s", prompt);
	}

	i = 0;
	while(i < (MAXBUF - 1)) {
		short int c = getc(stdin);

		if(c == '\n' || c == EOF) {
			break;
		}
		else if(c == '\b' && i > 0) {
			i -= 2;
		}
		else {
			input[i] = c;
		}
		++i;
	}
	input[i] = '\0';

	if(buffer != NULL) {
		*buffer = &input[0];
	}

	return i;
}
/* Initialise database with zero data.
 */
void db_init(void)
{
	db.fd = -1;
	db.err = 0;
	db.cur = 0;
	db.count = 0;
	db.size = 0;
	db.msg = "OK";
	db.data = NULL;
	strncpy(db.stat1, "ALIVE", sizeof(db.stat1)-1);
	strncpy(db.stat2, "MISSING", sizeof(db.stat2)-1);
	strncpy(db.stat3, "DEAD", sizeof(db.stat3)-1);
}
/* Free all resources from database.
 */
void db_free(void)
{
	free(db.data);
}
/* Add another section of database.
 */
void db_append(void)
{
	struct DatabaseData *tmp;
	size_t i;
	
	tmp = (struct DatabaseData *)realloc(db.data, sizeof(struct DatabaseData) * (db.size+MAXDB));
	if(tmp == NULL) {
		db.err = 4;
		db.msg = "Append failed";
		return;
	}
	db.size += MAXDB;

	db.data = tmp;
	i = db.count * MAXDB;

	while(i < db.size) {
		db.data[i].id = i;
		db.data[i].name[0] = '\0';
		db.data[i].stat[0] = '\0';
		++i;
	}

	++db.count;
}
/* Load a database from disk.
 */
void db_load(const char *name)
{
	size_t total;
	size_t i;
	FILE *fp;

	fp = fopen(name, "rb");
	if(fp == NULL) {
		db.err = 2;
		db.msg = "Load failed";
		return;
	}
	db.fd = fileno(fp);

	total = fread(&db.size, sizeof(size_t), 1, fp);
	if(total != 1) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database header invalid)";
		return;
	}

	total = fread(&db.count, sizeof(size_t), 1, fp);
	if(total != 1) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database header invalid)";
		return;
	}

	total = fread(db.stat1, sizeof(char), sizeof(db.stat1), fp);
	if(total != sizeof(db.stat1)) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database header invalid)";
		return;
	}

	total = fread(db.stat2, sizeof(char), sizeof(db.stat2), fp);
	if(total != sizeof(db.stat2)) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database header invalid)";
		return;
	}

	total = fread(db.stat3, sizeof(char), sizeof(db.stat3), fp);
	if(total != sizeof(db.stat3)) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database header invalid)";
		return;
	}

	i = 0;
	while(i < db.count) {
		db_append();
		++i;
	}

	printf("Database Count: %lu\n", i);

	i = 0;
	total = 0;
	while(i < db.count) {
		db_append();
		total += fread(db.data, sizeof(struct DatabaseData), MAXDB, fp);
		++i;
	}
	fclose(fp);

	if(i != db.count || total != db.size) {
		db.err = 3;
		db.msg = "Load failed (unaligned database)";
		printf("Got here!\n");
		return;
	}
}
/* Save database to disk.
 */
void db_save(const char *name)
{
	size_t total;
	size_t i;
	FILE *fp;

	fp = fopen(name, "wb");
	if(fp == NULL) {
		db.err = 2;
		db.msg = "Save failed";
		return;
	}
	db.fd = fileno(fp);

	total = fwrite(&db.size, sizeof(size_t), 1, fp);
	if(total != 1) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database header invalid)";
		return;
	}

	total = fwrite(&db.count, sizeof(size_t), 1, fp);
	if(total != 1) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database header invalid)";
		return;
	}

	total = fwrite(db.stat1, sizeof(char), sizeof(db.stat1), fp);
	if(total != sizeof(db.stat1)) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database header invalid)";
		return;
	}

	total = fwrite(db.stat2, sizeof(char), sizeof(db.stat2), fp);
	if(total != sizeof(db.stat2)) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database header invalid)";
		return;
	}

	total = fwrite(db.stat3, sizeof(char), sizeof(db.stat3), fp);
	if(total != sizeof(db.stat3)) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database header invalid)";
		return;
	}

	i = 0;
	total = 0;
	while(i < db.count) {
		total += fwrite(db.data, sizeof(struct DatabaseData), MAXDB, fp);
		++i;
	}
	fclose(fp);

	if(i != db.count || total != db.size) {
		db.err = 3;
		db.msg = "Save failed (unaligned database)";
		return;
	}
}
/* Encrypt the database with simple XOR encryption.
 */
void db_crypt(void)
{
}
/* Search the database for an ID.
 */
int db_search(int id)
{
	size_t i;

	for(i = 0; i < db.size && i != id; ++i);

	if(i == id) {
		return id;
	}
	
	return -1;
}
/* Print a database entry.
 */
void db_print(int longest, int id)
{
	size_t i;

	if(id < 0) {
		return;
	}

	for(i = 0; i < db.size && i != id; ++i);

	if(i == id) {
		printf("%-15d %-*s %-15s\n", db.data[i].id, longest+10, db.data[i].name, db.data[i].stat);
		return;
	}
	
	printf("ID not found in database.\n");
}
/* Replace a database entry using passed ID value.
 */
void db_replace(int id)
{
	char *tmp;
	size_t i;

	if(id < 0 || id >= db.size) {
		return;
	}

	for(i = 0; i < db.size && i != id; ++i);

	if(i == id) {
		(void)getstr(&tmp, "Enter name: ");
		if(strncmp(tmp, "", 1) == 0) {
			printf("You need to enter something.\n");
		}
		strncpy(db.data[i].name, tmp, sizeof(db.data[i].name));

		printf("Status options available:\n1) ALIVE\n2) MISSING\n3) DEAD\n");
		(void)getstr(&tmp, "Enter status: ");
		if(strncmp(tmp, "", 1) == 0) {
			printf("You need to enter something.\n");
		}

		switch(atoi(tmp)) {
			case 1:
				strncpy(db.data[i].stat, db.stat1, sizeof(db.data[i].stat));
				break;
			case 2:
				strncpy(db.data[i].stat, db.stat2, sizeof(db.data[i].stat));
				break;
			case 3:
				strncpy(db.data[i].stat, db.stat3, sizeof(db.data[i].stat));
				break;
			default:
				printf("Not an option.\n");
		}
		return;
	}
	
	printf("ID not found in database.\n");
}
/* Get the error code from the database.
 */
int db_geterrori(void)
{
	return db.err;
}
/* Get the error message from the database.
 */
const char *db_geterror(void)
{
	return db.msg;
}
/* Set the database current id number.
 */
void db_setcur(int id)
{
	if(id >= 0 && id < db.size) {
		db.cur = id;
	}
}
/* Get the database current id number.
 */
int db_getcur(void)
{
	return db.cur;
}
/* Get the database from here.
 */
Database *db_get(void)
{
	return &db;
}
