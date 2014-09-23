/*
 * CS3600, Fall 2013
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 * This program is intended to format your disk file, and should be executed
 * BEFORE any attempt is made to mount your file system.  It will not, however
 * be called before every mount (you will call it manually when you format 
 * your disk file).
 */

#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "3600fs.h"
#include "disk.h"

//initializes vcb
void initvcb(vcb * v, int size){
  v->magic = 420;
  v->blocksize = BLOCKSIZE;
  v->de_start = 1;
  v->de_length = 25;
  v->fat_start = 26;
  v->fat_length = 4;
  v->db_start = 30;
//  v->fat_length = size/100 + 1;
//  v->db_start = 26 + size/100 + 2;
  v->db_length = size - v->db_start;

  v->user = getuid();
  v->group = getgid();
  v->mode = 0777;
  fprintf(stderr, "creating disk with size %d, %d de blocks, %d fat blocks,	and %d data blocks", 
					size, v->de_length, v->fat_length, size-v->db_start);
}

//initialize directory entries
void initde(dirent * d){
  d->valid = 1;
  printf("%d", 1);
}


void myformat(int size) {
  // Do not touch or move this function
  dcreate_connect();

  //setting up vcb
  vcb myvcb;
  initvcb(&myvcb, size);
  
  dirent myde;
  initde(&myde);

  fatent myfat = { 0, 0, 0 }; //like 20% bruh
  char tmp[BLOCKSIZE];
  int block = 0; //current block to write to
  
  //write vcb to file
  memset(tmp, 0, BLOCKSIZE);
  memcpy(tmp, &myvcb, sizeof(vcb));
  dwrite(block,tmp);
  block++;

  //write directory entries to file
  for(int i=0; i<25; i++){
    memset(tmp, 0, BLOCKSIZE);
    memcpy(tmp, &myde, sizeof(dirent));
    memcpy(tmp + 128, &myde, sizeof(dirent));
    memcpy(tmp + 256, &myde, sizeof(dirent));
    memcpy(tmp + 384, &myde, sizeof(dirent));
    if(dwrite(block,tmp)< 0)
      perror("Error while writing to disk");
    block++;
  }

  //write fatents to file
  for (int i=0; i< myvcb.fat_length; i++){ 
    memset(tmp, 0, BLOCKSIZE);
    for(int x=0; x<128; x++){
      memcpy(tmp + (4 * x), &myfat, sizeof(fatent));
    }
    if (dwrite(block, tmp) < 0) 
      perror("Error while writing to disk");
    block++;
  }
  
  //0 out diskspace where there will be data(not really needed..)
  for (int i=0; i<size; i++){ 
    if (dwrite(block, tmp) < 0) 
      perror("Error while writing to disk");
    block++;
  }

  // Do not touch or move this function
  dunconnect();
}

int main(int argc, char** argv) {
  // Do not touch this function
  if (argc != 2) {
    printf("Invalid number of arguments \n");
    printf("usage: %s diskSizeInBlockSize\n", argv[0]);
    return 1;
  }

  unsigned long size = atoi(argv[1]);
  printf("Formatting the disk with size %lu \n", size);
  myformat(size);
}

