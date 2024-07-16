/*
 * database - Simple database with encryption.
 *
 * Author: Philip R. Simonson
 * Date:   07/05/2024
 *
 */

#define MAXBUF 256
#define MAXDB 32

struct DatabaseData {
	int id;
	char name[64];
	char stat[16];
};

struct Database {
	int fd;
	int err;
	int cur;
	char stat1[16];
	char stat2[16];
	char stat3[16];
	size_t count;
	size_t size;
	char *msg;
	struct DatabaseData **data;
};

typedef struct Database Database;

size_t getstr(char **buffer, const char *prompt);

void db_init(void);
void db_free(void);
void db_append(void);
void db_load(const char *name);
void db_save(const char *name);
void db_crypt(void);
int db_search(int id);
void db_print(int longest, int id);
void db_replace(int id);
int db_geterrori(void);
const char *db_geterror(void);
void db_setcur(int id);
int db_getcur(void);
Database *db_get(void);

