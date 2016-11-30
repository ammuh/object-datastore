#ifndef MINIDB_H
#define MINIDB_H
#include <stdio.h>

//Entry

//Collection of all entities
//Entity **db;
char * current_db_filename;
char heap_filename[25];
int DB_TABLE_SIZE;

typedef struct address
{
	long long id;
	int size;
	long int haddr;
	char filename[25];
	long int next;
}address;

address * db;

int dbinit(char* filename, int hash_size);
int dbload(char* path);
int dbwrite(void *ptr, long long id);
void tableDump(char * filename);
int add(void * obj, int size, long long key);
int get(void* obj, long long key);
int replace(void* obj, long long key);
int removeObj(long long key);
address * tableIter(char * filename);

#endif