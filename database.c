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
	db_init();
}
/* Add another section of database.
 */
void db_append(void)
{
	unsigned char *tmp;
	size_t i;
	
	tmp = (unsigned char *)realloc(db.data, sizeof(struct DatabaseData) * (db.size+MAXDB));
	if(tmp == NULL) {
		db.err = 4;
		db.msg = "Append failed";
		return;
	}
	db.data = tmp;
	i = db.count * MAXDB;
	db.size += MAXDB;
	++db.count;

	while(i < (db.count * MAXDB)) {
		((struct DatabaseData *)db.data)[i].id = i;
		((struct DatabaseData *)db.data)[i].name[0] = '\0';
		((struct DatabaseData *)db.data)[i].stat[0] = '\0';
		++i;
	}
}
/* Load a database from disk.
 */
void db_load(const char *name)
{
	size_t total;
	FILE *fp;

	db_free();

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
		db.msg = "Load failed (database size)";
		return;
	}

	total = fread(&db.count, sizeof(size_t), 1, fp);
	if(total != 1) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database count)";
		return;
	}

	total = fread(&db.cur, sizeof(int), 1, fp);
	if(total != 1) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database cur)";
		return;
	}

	if(db.size != (db.count * MAXDB)) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database size mismatch)";
		return;
	}

	total = fread(db.stat1, sizeof(char), sizeof(db.stat1), fp);
	if(total != sizeof(db.stat1)) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database stat1)";
		return;
	}

	total = fread(db.stat2, sizeof(char), sizeof(db.stat2), fp);
	if(total != sizeof(db.stat2)) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database stat2)";
		return;
	}

	total = fread(db.stat3, sizeof(char), sizeof(db.stat3), fp);
	if(total != sizeof(db.stat3)) {
		fclose(fp);
		db.err = 3;
		db.msg = "Load failed (database stat3)";
		return;
	}

	db.data = (unsigned char *)malloc(sizeof(struct DatabaseData)*(db.count * MAXDB));
	if(db.data == NULL) {
		db.err = 3;
		db.msg = "Load failed (out of memory)";
		return;
	}

	total = fread(db.data, sizeof(struct DatabaseData), db.count * MAXDB, fp);
	fclose(fp);
	db_crypt(); // Decrypt data

	if(total != db.size) {
		db.err = 3;
		db.msg = "Load failed (unaligned database)";
		return;
	}
}
/* Save database to disk.
 */
void db_save(const char *name)
{
	extern void db_crypt(void);
	size_t total;
	FILE *fp;

	fp = fopen(name, "wb");
	if(fp == NULL) {
		db.err = 2;
		db.msg = "Save failed";
		return;
	}
	db.fd = fileno(fp);
	db_crypt(); // Encrypt data

	total = fwrite(&db.size, sizeof(size_t), 1, fp);
	if(total != 1) {
		db_crypt(); // Decrypt data
		fclose(fp);
		db.err = 3;
		db.msg = "Save failed (database size)";
		return;
	}

	total = fwrite(&db.count, sizeof(size_t), 1, fp);
	if(total != 1) {
		db_crypt(); // Decrypt data
		fclose(fp);
		db.err = 3;
		db.msg = "Save failed (database count)";
		return;
	}

	total = fwrite(&db.cur, sizeof(int), 1, fp);
	if(total != 1) {
		db_crypt(); // Decrypt data
		fclose(fp);
		db.err = 3;
		db.msg = "Save failed (database cur)";
		return;
	}

	if(db.size != (db.count * MAXDB)) {
		db_crypt(); // Decrypt data
		fclose(fp);
		db.err = 3;
		db.msg = "Save failed (database size mismatch)";
		return;
	}

	total = fwrite(db.stat1, sizeof(char), sizeof(db.stat1), fp);
	if(total != sizeof(db.stat1)) {
		db_crypt(); // Decrypt data
		fclose(fp);
		db.err = 3;
		db.msg = "Save failed (database stat1)";
		return;
	}

	total = fwrite(db.stat2, sizeof(char), sizeof(db.stat2), fp);
	if(total != sizeof(db.stat2)) {
		db_crypt(); // Decrypt data
		fclose(fp);
		db.err = 3;
		db.msg = "Save failed (database stat2)";
		return;
	}

	total = fwrite(db.stat3, sizeof(char), sizeof(db.stat3), fp);
	if(total != sizeof(db.stat3)) {
		db_crypt(); // Decrypt data
		fclose(fp);
		db.err = 3;
		db.msg = "Save failed (database stat3)";
		return;
	}

	total = fwrite(db.data, sizeof(struct DatabaseData), db.count * MAXDB, fp);
	fclose(fp);
	db_crypt(); // Decrypt data

	if(total != db.size) {
		db.err = 3;
		db.msg = "Save failed (unaligned database)";
		return;
	}
}
/* Encrypt the database with simple XOR encryption.
 */
void db_crypt(void)
{
	for(size_t i = 0; i < db.size; ++i) {
		for(size_t j = 0; j < strlen(((struct DatabaseData *)db.data)[i].name); ++j) {
			if((((struct DatabaseData *)db.data)[i].name[j] >= 'a' && ((struct DatabaseData *)db.data)[i].name[j] <= 'm') ||
				(((struct DatabaseData *)db.data)[i].name[j] >= 'A' && ((struct DatabaseData *)db.data)[i].name[j] <= 'M')) {
				((struct DatabaseData *)db.data)[i].name[j] = ((struct DatabaseData *)db.data)[i].name[j] + 13;
			}
			else if((((struct DatabaseData *)db.data)[i].name[j] >= 'n' && ((struct DatabaseData *)db.data)[i].name[j] <= 'z') ||
				(((struct DatabaseData *)db.data)[i].name[j] >= 'N' && ((struct DatabaseData *)db.data)[i].name[j] <= 'Z')) {
				((struct DatabaseData *)db.data)[i].name[j] = ((struct DatabaseData *)db.data)[i].name[j] - 13;
			}
		}
	}
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
/* Write an HTML table page for database.
 */
void db_writeHTML(const char *name)
{
	size_t i;
	FILE *fp;

	fp = fopen(name, "wt");
	if(fp == NULL) {
		printf("Failed to open '%s' for writing.\n", name);
		db.err = 3;
		db.msg = "Saving failed!";
		return;
	}

	fprintf(fp, "<html>\n\t<head><title>Person Status Report</title></head>\n\n\t<body bgcolor=\"#000000\" text=\"#00FF00\">\n\t\t<center>\n\n\t\t<p><h3>Person Status Report</h3></p>\n\n\t\t<table border=2pt cell-padding=2pt cell-spacing=2pt>\n\t\t\t<tr>\n\t\t\t\t<th>ID</th>\n\t\t\t\t<th>NAME</th>\n\t\t\t\t<th>STATUS</th>\n\t\t\t</tr>");

	db_crypt(); // Decrypt data
	for(i = 0; i < db.size; ++i) {
		fprintf(fp, "\n\n\t\t\t<tr>\n\t\t\t\t<td>%d</td>\n\t\t\t\t<td>%s</td>\n\t\t\t\t<td>%s</td>\n\t\t\t</tr>", ((struct DatabaseData *)db.data)[i].id, ((struct DatabaseData *)db.data)[i].name, ((struct DatabaseData *)db.data)[i].stat);
	}
	db_crypt(); // Encrypt data

	fprintf(fp, "\n\t\t</table>\n\t</body>\n</html>\n");
	fclose(fp);

	db.err = 0;
	db.msg = "OK";
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
		printf("%-15d %-*s %-15s\n", ((struct DatabaseData *)db.data)[i].id, longest+10, ((struct DatabaseData *)db.data)[i].name, ((struct DatabaseData *)db.data)[i].stat);
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
			return;
		}
		strncpy(((struct DatabaseData *)db.data)[i].name, tmp, sizeof(((struct DatabaseData *)db.data)[i].name));

		printf("Status options available:\n1) ALIVE\n2) MISSING\n3) DEAD\n");
		(void)getstr(&tmp, "Enter status: ");
		if(strncmp(tmp, "", 1) == 0) {
			printf("You need to enter something.\n");
			return;
		}

		switch(atoi(tmp)) {
			case 1:
				strncpy(((struct DatabaseData *)db.data)[i].stat, db.stat1, sizeof(((struct DatabaseData *)db.data)[i].stat));
				break;
			case 2:
				strncpy(((struct DatabaseData *)db.data)[i].stat, db.stat2, sizeof(((struct DatabaseData *)db.data)[i].stat));
				break;
			case 3:
				strncpy(((struct DatabaseData *)db.data)[i].stat, db.stat3, sizeof(((struct DatabaseData *)db.data)[i].stat));
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
