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
#include <sys/socket.h>
#include "prs/network.h"
#include "database.h"
#include "netprintf.h"

static Database db;

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
	size_t total;
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
		db.msg = "Save failed (database size)";
		return;
	}

	total = fwrite(&db.count, sizeof(size_t), 1, fp);
	if(total != 1) {
		fclose(fp);
		db.err = 3;
		db.msg = "Save failed (database count)";
		return;
	}

	total = fwrite(&db.cur, sizeof(int), 1, fp);
	if(total != 1) {
		fclose(fp);
		db.err = 3;
		db.msg = "Save failed (database cur)";
		return;
	}

	if(db.size != (db.count * MAXDB)) {
		fclose(fp);
		db.err = 3;
		db.msg = "Save failed (database size mismatch)";
		return;
	}

	total = fwrite(db.stat1, sizeof(char), sizeof(db.stat1), fp);
	if(total != sizeof(db.stat1)) {
		fclose(fp);
		db.err = 3;
		db.msg = "Save failed (database stat1)";
		return;
	}

	total = fwrite(db.stat2, sizeof(char), sizeof(db.stat2), fp);
	if(total != sizeof(db.stat2)) {
		fclose(fp);
		db.err = 3;
		db.msg = "Save failed (database stat2)";
		return;
	}

	total = fwrite(db.stat3, sizeof(char), sizeof(db.stat3), fp);
	if(total != sizeof(db.stat3)) {
		fclose(fp);
		db.err = 3;
		db.msg = "Save failed (database stat3)";
		return;
	}

	total = fwrite(db.data, sizeof(struct DatabaseData), db.count * MAXDB, fp);
	fclose(fp);

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
void db_print(SOCKET s, int longest, int id)
{
	size_t i;

	if(id < 0) {
		return;
	}

	for(i = 0; i < db.size && i != id; ++i);

	if(i == id) {
		socket_printf(s, "%-15d %-*s %-15s\r\n", ((struct DatabaseData *)db.data)[i].id, longest+10, ((struct DatabaseData *)db.data)[i].name, ((struct DatabaseData *)db.data)[i].stat);
		return;
	}
	
	socket_printf(s, "ID not found in database.\r\n");
}
/* Replace a database entry using passed ID value.
 */
void db_replace(SOCKET s, int id)
{
	long int nbytes;
	char tmp[MAXBUF];
	size_t i;

	if(id < 0 || id >= db.size) {
		return;
	}

	for(i = 0; i < db.size && i != id; ++i);

	if(i == id) {
		socket_printf(s, "Enter name: ");
		nbytes = recv(s, tmp, MAXBUF-1, s);
		if(nbytes < 0) {
			return;
		}
		tmp[nbytes] = '\0';
		while(tmp[nbytes-1] == '\r' || tmp[nbytes-1] == '\n') {
			--nbytes;
		}
		tmp[nbytes] = '\0';
		if(strncmp(tmp, "", 1) == 0) {
			socket_printf(s, "You need to enter something.\r\n");
		}
		strstr(tmp, "\r\n")[0] = '\0';
		strncpy(((struct DatabaseData *)db.data)[i].name, tmp, sizeof(((struct DatabaseData *)db.data)[i].name));

		socket_printf(s, "Status options available:\r\n1) ALIVE\r\n2) MISSING\r\n3) DEAD\r\nEnter name: ");
		nbytes = recv(s, tmp, MAXBUF-1, s);
		if(nbytes < 0) {
			return;
		}
		tmp[nbytes] = '\0';
		while(tmp[nbytes-1] == '\r' || tmp[nbytes-1] == '\n') {
			--nbytes;
		}
		tmp[nbytes] = '\0';
		if(strncmp(tmp, "", 1) == 0) {
			socket_printf(s, "You need to enter something.\r\n");
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
				socket_printf(s, "Not an option.\r\n");
		}

		return;
	}
	
	socket_printf(s, "ID not found in database.\r\n");
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
