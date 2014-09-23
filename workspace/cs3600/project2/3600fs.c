/*
 * CS3600, Fall 2013
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 * This file contains all of the basic functions that you will need 
 * to implement for this project.  Please see the project handout
 * for more details on any particular function, and ask on Piazza if
 * you get stuck.
 */

#define FUSE_USE_VERSION 26

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#define _POSIX_C_SOURCE 199309

#include <time.h>
#include <fuse.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <sys/statfs.h>

#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include "3600fs.h"
#include "disk.h"

vcb * myvcb;
dirent * de[100];
fatent ** fatty = NULL;
char ** dbs = NULL;

//dewrite takes in the index of a block on the dick
//and writes the current information about that block back 
//to the disk. This allows us to modify a directory entry 
//and write the changes to the disk by simply calling
//dewrite.
static int dewrite(int index){
	//i1 - i4 are the 4 directory entries in the de block i.
	int i1 = (index / 4) * 4;
	int i2 = i1 + 1;
	int i3 = i1 + 2;
	int i4 = i1 + 3;
	//tmp is a string representation of the data to be written
	//think of it kind of like a data dump
	char tmp[BLOCKSIZE];

	//use memset to write the information stored in tmp to the disk
	memset(tmp, 0, BLOCKSIZE);
	memcpy(tmp, de[i1], sizeof(dirent));
	memcpy(tmp + 128, de[i2], sizeof(dirent));
	memcpy(tmp + 256, de[i3], sizeof(dirent));
	memcpy(tmp + 128 + 256, de[i4], sizeof(dirent)); //DONT HAVE CALCULATOR ON ME DONT JUDGE ME

	//if dwrite throws an error, print an error message
	if(dwrite(myvcb->de_start + index/4, tmp) < 0){
		perror("Error writing directory entry to disk");
	}
	return 0;
}

//fatwrite allows us to call a method to write information to the disk
//works similarly to the way dewrite does
static int fatwrite(int index){
	int x = (index / 128) * 128;
	char tmp[BLOCKSIZE];
	//write 0s over the temp variable
	memset(tmp,0,BLOCKSIZE);
	//copy the data in the FAT entries to the temp variable
	for(int i= x; i< 128 + x; i++){
		memcpy(tmp + 4 * i, fatty[i], sizeof(fatent));
	}
	//write the data in the tmp to the disk
	//throw error if returns -1
	if(dwrite(myvcb->fat_start + index/128, tmp) < 0) 
		perror("Error writing directory entry to disk");
	return 0;
}
//next_fat_index finds the next free FAT entry space
int next_fat_index(){
	//starting with the first FAT entry, step through the FAT entries
	for(int x = 0; x < myvcb->fat_length * 128; x++){
		//check to see if this fat entry is used
		if(fatty[x]->used == 0){
			//if not, return its index
			return x;
		}
	}
	return -1;
}

/*
 * Initialize filesystem. Read in file system metadata and initialize
 * memory structures. If there are inconsistencies, now would also be
 * a good time to deal with that. 
 *
 * HINT: You don't need to deal with the 'conn' parameter AND you may
 * just return NULL.
 *
 */
static void* vfs_mount(struct fuse_conn_info *conn) {
	fprintf(stderr, "vfs_mount called\n");
	//to compile without warnings
	//if(conn){
	//	printf("wup");
	//}
	//  Do not touch or move this code; connects the disk
	fprintf(stderr, "running dconnect()\n");
	dconnect();

	//allocate space for the vcb and a temp variable
	fprintf(stderr, "allocating space for tmp and vcb\n");	
	char * temp = malloc(BLOCKSIZE);
	myvcb = (vcb*)malloc(BLOCKSIZE);

	//  read in vcb 
	fprintf(stderr, "reading in the vcb\n");
	if(dread(0, temp)< 0)
		perror("Error reading disk vcb");

	//copy information read from the vcb to the temp variable	
	fprintf(stderr, "attempting to copy from temp to vcb\n");
	memcpy(myvcb, temp, sizeof(vcb));

	//read from the directory entry blocks	
	fprintf(stderr, "attemping to read from the de blocks\n");
	int direntnum = 0;
	for(int i=0; i < myvcb->de_length; i++){ //4 dirents to a block
		if(dread(i + myvcb->de_start, temp)< 0)
			perror("Error reading disk de");

		//allocate space for the directory entries
		for(int x = 0; x < 4; x++){
			//fprintf(stderr, "attempting to allocate space for de #%d (should be 100)\n", direntnum+1);
			de[i * 4 + x] = malloc(sizeof(dirent));
			memcpy(de[i * 4 + x], temp + x * 128, sizeof(dirent));
			direntnum++;
		}
	}
	//allocate space for the FAT blocks
	fprintf(stderr, "attempting to allocate space for the fat, %d entries at %d size each\n", 
												myvcb->fat_length, sizeof(fatent));
	fatty = malloc(myvcb->fat_length * sizeof(fatent*));
	if(! fatty){
		free(fatty);
		perror("Error allocating memory");
	}
	
	//loop through the FAT blocks	
	fprintf(stderr, "begin looping throught fat\n");
	int fatblocknum = 0;
	int fatentnum = 0;
	for(int i= 0; i < myvcb->fat_length; i++){
		//store data from the FAT into temp
		//throw error if dread returns -1
		if(dread(i + myvcb->fat_start, temp)< 0)
			perror("Error reading disk fat");
		//allocate space for FAT entries	
		fprintf(stderr, "allocate space for FAT block #%d (should be 4)\n", fatblocknum+1);
		fatblocknum++;
		for(int x = 0; x < 128; x++){
			//fprintf(stderr, "allocate space for FAT entry #%d (should be 512)\n", fatentnum+1);
			fatentnum++;
			fatty[i * 128 + x] = malloc(sizeof(fatent));
			//copy data stored in temp to the FAT entries
			memcpy(fatty[i * 128 + x], temp + x * 4, sizeof(fatent));
		}

	}

	//allocate space for data blocks
	fprintf(stderr, "attempting to allocate space for the data blocks, %d entries at %d size each\n",
												myvcb->db_length, BLOCKSIZE);
	dbs = malloc(myvcb->db_length * BLOCKSIZE);
	

	//starting at the first data block, initialize the data blocks
	//throw error if -1 is returned
	fprintf(stderr, "begin initializing data blocks\n");
	int dbnum = 1;
	for(int i=0; i<myvcb->db_start; i++){
		fprintf(stderr, "attempting to read from data block #%d (should be 69)\n", dbnum);
		dbnum++;
		//allocate space for each data block
		dbs[i] = malloc(BLOCKSIZE);	
		if(dread(i + myvcb->db_start, temp) < 0)
			perror("Error with db initialization");
		fprintf(stderr, "attempting to write to dbs #%s\n", dbs[i]);
		memcpy(dbs[i], temp, 512);
	}
	//free temp
	free(temp);
	return NULL;

}



// Called when your file system is unmounted.
static void vfs_unmount (void *private_data) { 
	fprintf(stderr, "vfs_unmount called\n"); 

	/* 3600: YOU SHOULD ADD CODE HERE TO MAKE SURE YOUR ON-DISK STRUCTURES
	   ARE IN-SYNC BEFORE THE DISK IS UNMOUNTED (ONLY NECESSARY IF YOU
	   KEEP DATA CACHED THAT'S NOT ON DISK */

	//Do not touch or move this code; unconnects the disk 
	dunconnect(); 
} 

/*
 *
 * Given an absolute path to a file/directory (i.e., /foo ---all
 * paths will start with the root directory of the CS3600 file
 * system, "/"), you need to return the file attributes that is
 * similar stat system call.
 *
 * HINT: You must implement stbuf->stmode, stbuf->st_size, and
 * stbuf->st_blocks correctly.
 *
 */
static int vfs_getattr(const char *path, struct stat *stbuf) {
	fprintf(stderr, "vfs_getattr called\n");


	// Do not mess with this code
	stbuf->st_nlink = 1; // hard links
	stbuf->st_rdev  = 0;
	stbuf->st_blksize = BLOCKSIZE;

	char * name;
	int numdir = 0;
	int folderp = strcmp(path, "/");

	//gets name of file
	while(*path){
		if(*path == '/'){
			numdir++;
			if(*(path + 1))
				name = path + 1;
		}
		path++;
	}
	//more than one nested directory
	if(numdir > 1){
		return -1;
	}
	int filenum = -1;
	for(int x = 0; x < NUMFILES; x++){
		if(de[x]->valid == 0)
			if(strcmp(de[x]->name, name) == 0)
				filenum = x;
	}

	if (folderp == 0)
		stbuf->st_mode  = 0777 | S_IFDIR;
	else{
		if(filenum == -1)
			return -ENOENT;
		stbuf->st_mode  = de[filenum]->mode | S_IFREG;
		stbuf->st_uid     = de[filenum]->user;
		stbuf->st_gid     = de[filenum]->group;
	}

	/* stbuf->st_atime   = de[filenum]->access_time; */
	/* stbuf->st_mtime   = de[filenum]->modify_time; */
	/* stbuf->st_ctime   = de[filenum]->create_time; */
	/* stbuf->st_size    = de[filenum]->size; */
	/* stbuf->st_blocks  = 0; //change later */


	return 0;
}

/*
 * Given an absolute path to a directory (which may or may not end in
 * '/'), vfs_mkdir will create a new directory named dirname in that
 * directory, and will create it with the specified initial mode.
 *
 * HINT: Don't forget to create . and .. while creating a
 * directory.
 */
/*
 * NOTE: YOU CAN IGNORE THIS METHOD, UNLESS YOU ARE COMPLETING THE
 *       EXTRA CREDIT PORTION OF THE PROJECT.  IF SO, YOU SHOULD
 *       UN-COMMENT THIS METHOD.
 static int vfs_mkdir(const char *path, mode_t mode) {

 return -1;
 } */

/** Read directory
 *
 * Given an absolute path to a directory, vfs_readdir will return
 * all the files and directories in that directory.
 *
 * HINT:
 * Use the filler parameter to fill in, look at fusexmp.c to see an example
 * Prototype below
 *
 * Function to add an entry in a readdir() operation
 *
 * @param buf the buffer passed to the readdir() operation
 * @param name the file name of the directory entry
 * @param stat file attributes, can be NULL
 * @param off offset of the next entry or zero
 * @return 1 if buffer is full, zero otherwise
 * typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
 *                                 const struct stat *stbuf, off_t off);
 *
 * Your solution should not need to touch fi
 *
 */
static int vfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi)
{
	//because we are only supporting the / directory, all absolute paths must
	//start with /. if a path that does not start with / is given, throw error
	if(strcmp(path, "/") != 0){
		return -1;
	}
	//loop through the directory entries and get each one's names
	for(int x = 0; x < NUMFILES; x++){
		if(de[x]->valid == 0){
			if(filler(buf, de[x]->name, NULL, 0) != 0)
				return 0;
		}
	}
	return 0;
}

/*
 * Given an absolute path to a file (for example /a/b/myFile), vfs_create
 * will create a new file named myFile in the /a/b directory.
 *
 */
static int vfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {

	char * name;
	int numdir = 0;

	//gets name of file
	while(*path){
		if(*path == '/'){
			numdir++;
			if(*(path + 1))
				name = path + 1;
			else
				return -1;
		}
		path++;
	}
	//more than one nested directory
	if(numdir > 1){
		return -1;
	}

	//check if file already exists
	for(int x = 0; x < NUMFILES; x++)
		if(strcmp(de[x]->name, name) == 0)
			return EEXIST; //file already exists


	for(int x = 0; x < NUMFILES; x++){
		if(de[x]->valid){
			de[x]->valid = 0;
			de[x]->first_block = next_fat_index();
			de[x]->user = getuid();
			de[x]->group = getgid();
			de[x]->mode = mode;
			clock_gettime(CLOCK_REALTIME, &(de[x]->access_time));
			clock_gettime(CLOCK_REALTIME, &(de[x]->modify_time));
			clock_gettime(CLOCK_REALTIME, &(de[x]->create_time));
			strcpy(de[x]->name, name);


			fatty[de[x]->first_block]->used = 1;
			fatty[de[x]->first_block]->eof = 1;
			fatwrite(de[x]->first_block);
			dewrite(x);
			return 0;
		}

	}


	return -1;
}

/*
 * The function vfs_read provides the ability to read data from
 * an absolute path 'path,' which should specify an existing file.
 * It will attempt to read 'size' bytes starting at the specified
 * offset (offset) from the specified file (path)
 * on your filesystem into the memory address 'buf'. The return
 * value is the amount of bytes actually read; if the file is
 * smaller than size, vfs_read will simply return the most amount
 * of bytes it could read.
 *
 * HINT: You should be able to ignore 'fi'
 *
 */
static int vfs_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi)
{
	char * name;
	int numdir = 0;

	//gets name of file
	while(*path){
		if(*path == '/'){
			numdir++;
			if(*(path + 1))
				name = path + 1;
			else
				return -1;
		}
		path++;
	}

	return 0;
}

/*
 * The function vfs_write will attempt to write 'size' bytes from
 * memory address 'buf' into a file specified by an absolute 'path'.
 * It should do so starting at the specified offset 'offset'.  If
 * offset is beyond the current size of the file, you should pad the
 * file with 0s until you reach the appropriate length.
 *
 * You should return the number of bytes written.
 *
 * HINT: Ignore 'fi'
 */

static int vfs_write(const char *path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi)
{

	/* 3600: NOTE THAT IF THE OFFSET+SIZE GOES OFF THE END OF THE FILE, YOU
	   MAY HAVE TO EXTEND THE FILE (ALLOCATE MORE BLOCKS TO IT). */
/*	char * name;
	int numdir = 0;

	while(*path){
		if(*path == '/'){
			numdir++;
			if(*(path + 1))
				name = path + 1;
			else //is directory
				return -1;
		}
		path++;
	}
	//more than one nested directory
	if(numdir > 1){
		return -1;
	}
	for(int x = 0; x < NUMFILES; x++){
		if(strcmp(de[x]->name,  name) == 0){
			int curblock = de[x]->first_block;
			char * ptr = dbs[curblock];
			int count = 0;
			while(offset > 0){
				if(count < 512){
					ptr++;
					offset--;
				}

			}
			while(buf){
				if(count < 512){
					memcpy(ptr, buf, 1);
					buff++;
					ptr++;
				}else{
					curblock = 

						return 0;
				} 
			}
		}
	}
*/
return 0;
}

/*
 * This function deletes the last component of the path (e.g., /a/b/c you
 * need to remove the file 'c' from the directory /a/b).
 */
static int vfs_delete(const char *path)
{

	/* 3600: NOTE THAT THE BLOCKS CORRESPONDING TO THE FILE SHOULD BE MARKED
	   AS FREE, AND YOU SHOULD MAKE THEM AVAILABLE TO BE USED WITH OTHER FILES */
	char * name;
	int numdir = 0;

	while(*path){
		if(*path == '/'){
			numdir++;
			if(*(path + 1))
				name = path + 1;
			else //is directory
				return -1;
		}
		path++;
	}
	//more than one nested directory
	if(numdir > 1){
		return -1;
	}


	for(int x = 0; x < NUMFILES; x++){
		if(strcmp(de[x]->name,  name) == 0){
			de[x]->valid = 1;

			/* char tmp[BLOCKSIZE]; */
			/* memset(tmp, 0, BLOCKSIZE); */
			/* memcpy(tmp, de[x], sizeof(dirent)); */
			/* if(dwrite(myvcb->de_start + x, tmp) < 0){ */
			/* 	perror("Error deleting file"); */
			/* } */
			dewrite(x);
			return 0;
		}
	}
	return -1;
}

/*
 * The function rename will rename a file or directory named by the
 * string 'oldpath' and rename it to the file name specified by 'newpath'.
 *
 * HINT: Renaming could also be moving in disguise
 *
 */
static int vfs_rename(const char *from, const char *to)
{
	//check to see if the string given is longer than 27 characters
	if(strlen(to) > 27){
		perror("Given name longer than 27 characters");
		return -1;
	}
	//find the file given from the data entry blocks
	for(int i = 0; i < NUMFILES; i++){
		if(strcmp(de[i]->name, from) == 0){
			//set the name of the de block to the given name
			strcpy(de[i]->name, to);

			//write the changes back the disk
			/* char tmp[BLOCKSIZE]; */
			/* memset(tmp, 0, BLOCKSIZE); */
			/* memcpy(tmp, de[i], sizeof(dirent)); */
			/* if(dwrite(myvcb->de_start + i, tmp) < 0){ */
			/* 	perror("Error deleting file"); */
			/* } */
			dewrite(i);
			return 0;
		} 
	}
	return -1;
}


/*
 * This function will change the permissions on the file
 * to be mode.  This should only update the file's mode.
 * Only the permission bits of mode should be examined
 * (basically, the last 16 bits).  You should do something like
 *
 * fcb->mode = (mode & 0x0000ffff);
 *
 */
static int vfs_chmod(const char *file, mode_t mode)
{
	//find the file given from the data entry blocks
	for(int i = 0; i < NUMFILES; i++){
		if(strcmp(de[i]->name, file) == 0){
			//set the mode of the de block to the given mode
			de[i]->mode = mode;

			//write the changes back the disk
			/* char tmp[BLOCKSIZE]; */
			/* memset(tmp, 0, BLOCKSIZE); */
			/* memcpy(tmp, de[i], sizeof(dirent)); */
			/* if(dwrite(myvcb->de_start + i, tmp) < 0){ */
			/* 	perror("Error deleting file"); */
			/* } */
			dewrite(i);
			return 0;
		} 
	} 
	return -1;
}

/*
 * This function will change the user and group of the file
 * to be uid and gid.  This should only update the file's owner
 * and group.
 */
static int vfs_chown(const char *file, uid_t uid, gid_t gid)
{ 
	//find the file given from the data entry blocks
	for(int i = 0; i < NUMFILES; i++){
		if(strcmp(de[i]->name, file) == 0){
			strcpy(de[i]->user, uid);
			strcpy(de[i]->group, gid);							   
			dewrite(i);			
			return 0;
		}
	}
	return -1;
}

/*
 * This function will update the file's last accessed time to
 * be ts[0] and will update the file's last modified time to be ts[1].
 */
static int vfs_utimens(const char *file, const struct timespec ts[2])
{
	//find the file given from the data entry blocks
	for(int i = 0; i < NUMFILES; i++){
		if(strcmp(de[i]->name, file) == 0){
			//de[i]->access_time = ts[0];
			//de[i]->modify_time = ts[1];
			dewrite(i);
			return 0;
		}
	}
	return -1;
}

/*
 * This function will truncate the file at the given offset
 * (essentially, it should shorten the file to only be offset
 * bytes long).
 */
static int vfs_truncate(const char *file, off_t offset)
{

	/* 3600: NOTE THAT ANY BLOCKS FREED BY THIS OPERATION SHOULD
	   BE AVAILABLE FOR OTHER FILES TO USE. */

	return 0;
}

/*
 * You shouldn't mess with this; it sets up FUSE
 *
 * NOTE: If you're supporting multiple directories for extra credit,
 * you should add 
 *
 *     .mkdir	 = vfs_mkdir,
 */
static struct fuse_operations vfs_oper = {
	.init    = vfs_mount,
	.destroy = vfs_unmount,
	.getattr = vfs_getattr,
	.readdir = vfs_readdir,
	.create	 = vfs_create,
	.read	 = vfs_read,
	.write	 = vfs_write,
	.unlink	 = vfs_delete,
	.rename	 = vfs_rename,
	.chmod	 = vfs_chmod,
	.chown	 = vfs_chown,
	.utimens	 = vfs_utimens,
	.truncate	 = vfs_truncate,
};

int main(int argc, char *argv[]) {
	/* Do not modify this function */
	umask(0);
	if ((argc < 4) || (strcmp("-s", argv[1])) || (strcmp("-d", argv[2]))) {
		printf("Usage: ./3600fs -s -d <dir>\n");
		exit(-1);
	}
	return fuse_main(argc, argv, &vfs_oper, NULL);
}

