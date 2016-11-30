#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "miniDB.h"
/*
    File Structure
    First int: table size
    Second block: table
*/
// Helper functions

address iteraddr;
int bucket;
char file[25];

int writeObj(FILE * f, void * obj, long int addr, int size, int num){
    fseek(f, addr, SEEK_SET);
    fwrite(obj, size, num, f);
    return 0;
}

long int appendObj(FILE * f, void * obj, int size, int num){
    fseek(f, 0, SEEK_END);
    long int pos = ftell(f);
    fwrite(obj, size, num, f);
    return pos;
}

int readObj(FILE * f, void * obj, long int addr, int size, int num){
    fseek(f, addr, SEEK_SET);
    fread(obj, size, num, f);
    return 0;
}

static long int createAddress(FILE * f, long long key, char * filename, address ** addr) {
    //if (entry == NULL || key == NULL) return NULL;
    *addr = malloc(sizeof(address));
    (*addr)->id = key;
    (*addr)->next = 0;
    strcpy((*addr)->filename, filename);
    return appendObj(f, *addr, sizeof(address), 1);
}

/*
 * Helper function that does most of the implemention work
 */
static long int getAddress(long long key, char * filename, int create_if_missing, address **a) {
    FILE *f = fopen(current_db_filename, "rb+");
    int offset = 25*sizeof(char) + sizeof(int);
    fseek(f, offset,SEEK_SET);
    int h = (key % DB_TABLE_SIZE);
    address * curraddr = malloc(sizeof(address));
    long int pos;

    fseek(f, h*sizeof(address),SEEK_CUR);
    pos = ftell(f);

    fread(curraddr, sizeof(address), 1,f);

    if (curraddr->id) {
                /* Search until we find a match or hit the end */
        while (curraddr->next && key != curraddr-> id){
            fseek(f, curraddr->next, SEEK_SET);
            pos = ftell(f);
            fread(curraddr, sizeof(address), 1, f);
        }
        if (key == curraddr->id) {
            *a = curraddr;
            fclose(f);
            return pos;
        } else if (curraddr->next == 0 && create_if_missing) {
            /* If we failed to find the k8ey, we can create an entry in place */
            long int naddr = createAddress(f, key, filename, a);
            curraddr->next = naddr;
            writeObj(f, curraddr, pos, sizeof(address), 1);
            fclose(f);          
            return naddr;
        }
    } else if (create_if_missing) {
        curraddr->id = key;
        curraddr->next = 0;
        strcpy(curraddr->filename, filename);
        writeObj(f, curraddr, pos, sizeof(address), 1);
        *a = curraddr;
        fclose(f);
        return pos;
    }
    fclose(f);
    return 0;
}


//Main functions

int dbinit(char * filename, int hash_size){
    //DB_TABLE_SIZE = hash_size;    
    printf("Initializing Database...\n");
    db = malloc(hash_size*sizeof(address));
    int i;
    for(i = 0; i < hash_size; i++){
        db[i].id = 0;
    }
    char heap[25];
    strcpy(heap, filename);
    strcat(heap, ".heap");

    FILE *f = fopen(filename, "wb");
    fwrite(&hash_size, sizeof(int), 1, f);
    fwrite(heap, sizeof(char), 25,f);
    fwrite(db, sizeof(address), hash_size,f);
    fclose(f);
    f = fopen(heap, "wb");
    fclose(f);
    return 1;
}

int dbload(char * filename){
    current_db_filename = filename;
    FILE *f = fopen(filename, "rb");
    fread(&DB_TABLE_SIZE, sizeof(int), 1, f);
    fread(heap_filename, sizeof(char), 25, f);
    fclose(f);
    return 0;
}

address * tableIter(char * filename){
    int offset = 25*sizeof(char) + sizeof(int);
    if(filename != NULL){
        bucket = 0; 
        iteraddr.id = 0;
        strcpy(file, filename);     
    }
    FILE * f = fopen(file, "rb");
    if(iteraddr.id != 0){
        if(iteraddr.next != 0){
            readObj(f, &iteraddr, iteraddr.next, sizeof(address), 1);
            return &iteraddr;
        }else{
            iteraddr.id = 0;
            bucket++;
        }
    }
    if(bucket < DB_TABLE_SIZE){
        while(!iteraddr.id && bucket < DB_TABLE_SIZE){
            readObj(f, &iteraddr, offset + bucket*sizeof(address), sizeof(address), 1);
            bucket++;
        }
        return &iteraddr;
    }
    return NULL;
}

void tableDump(char * filename){
    printf("Dumping Database\n");
    FILE *f = fopen(filename, "rb");

    //int *size = malloc(sizeof(int));
    //fread(size, sizeof(int), 1, f);
    int *size = malloc(sizeof(int));
    fread(size, sizeof(int), 1, f);
    printf("Hashtable Size: %d\n", *size);
    char heapfile[25];
    fread(heapfile, sizeof(char), 25, f);
    printf("Heap Filename: %s\n", heapfile);

    fclose(f);
    address * a = tableIter(filename);
    while(a){
        if(a->id){
            printf("Object '%lld' of size %d in bucket %d found in %s at %ld\n", a->id, a->size, bucket, a->filename, a->haddr);
        }
    
        a = tableIter(NULL);
    }
   free(size);
}

int add(void * obj, int size, long long key){
    address * a = malloc(sizeof(address));
    long int addr = getAddress(key, heap_filename, 0, &a);
    if(!addr){
        addr = getAddress(key, heap_filename, 1, &a);
        FILE *f = fopen(heap_filename, "rb+");
        long int haddr = appendObj(f, obj, size, 1);
        fclose(f);
        a->haddr = haddr;
        a->size = size;
        f = fopen(current_db_filename, "rb+");
        writeObj(f, a, addr, sizeof(address), 1);
        fclose(f);
        free(a);
        return 0;
    }else {
        free(a);
        return -1;
    }
}

int get(void* obj, long long key){
    address * a = malloc(sizeof(address));
    long int addr = getAddress(key, heap_filename, 0, &a);
    if(addr){
        FILE *f = fopen(a->filename, "rb");
        readObj(f, obj, a->haddr, a->size, 1);
        fclose(f);
        free(a);
        return 0;
    }
    free(a);
    return -1;
}

int replace(void* obj, long long key){
    address * a = malloc(sizeof(address));
    long int addr = getAddress(key, heap_filename, 0, &a);
    if(addr){
        FILE *f = fopen(a->filename, "rb+");
        writeObj(f, obj, a->haddr, a->size, 1);
        fclose(f);
        free(a);
        return 0;
    }
    free(a);
    return -1;
}

int removeObj(long long key){
    address * a = malloc(sizeof(address));
    long int addr = getAddress(key, heap_filename, 0, &a);
    if(!addr){
        free(a);
        return -1;
    }
    FILE *f = fopen(current_db_filename, "rb+");
    int offset = 25*sizeof(char) + sizeof(int);
    fseek(f, offset,SEEK_SET);
    int h = (key % DB_TABLE_SIZE);
    address * prev = malloc(sizeof(address));
    prev->id = 0;
    address * curraddr = malloc(sizeof(address));

    long int pos;
    long int prevpos = -1;
    fseek(f, h*sizeof(address),SEEK_CUR);
    pos = ftell(f);

    fread(curraddr, sizeof(address), 1,f);
    while(curraddr->id != key){
        free(prev);
        prev = curraddr;
        prevpos = pos;
        fseek(f, curraddr->next,SEEK_SET);
        pos = ftell(f);
        curraddr = malloc(sizeof(address));
        fread(curraddr, sizeof(address), 1,f);
    }
    if(prevpos < 0){
        address a;
        a.id = 0;
        if(curraddr->next){
            address b;
            b.id = 0;
            readObj(f,&a, curraddr->next, sizeof(address), 1);
            writeObj(f, &b, curraddr->next, sizeof(address), 1);
        }
        writeObj(f, &a, pos, sizeof(address), 1);
    }else{
        address b;
        b.id = 0;
        prev->next = curraddr->next;
        writeObj(f, prev, prevpos, sizeof(address), 1);
        writeObj(f, &b, pos, sizeof(address), 1);
    }
    fclose(f);
    return 0;
}
