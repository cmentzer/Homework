/*
 * CS3600, Fall 2013
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#ifndef __3600DNS_H__
#define __3600DNS_H__


#endif

typedef struct header{
	unsigned int id: 16;
	unsigned int rd: 1;
	unsigned int tc: 1;
	unsigned int aa: 1;
	unsigned int opcode: 4;
	unsigned int qr: 1;
	unsigned int rc: 4;
	unsigned int z: 3;
	unsigned int ra: 1;
	unsigned int qd: 16;
	unsigned int an: 16;
	unsigned int ns: 16;
	unsigned int ar: 16;	
} header;

typedef struct question{
	unsigned int qtype: 16;
	unsigned int qclass: 16;
} question;

typedef struct answer{
	unsigned int type: 16;
	unsigned int c: 16;
	unsigned int TTL: 32;
	unsigned int rdl: 16;
} answer;
