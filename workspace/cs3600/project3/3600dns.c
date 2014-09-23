/*
 * CS3600, Fall 2013
 * Project 3 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#include <math.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "3600dns.h"

int packetsize = 0;
//ititialize the packet to be sent
static char* init_packet(char* address){
	
	//allocate space for the packet
	char* packet = (char *) malloc(100);
	char* pstart = packet;

	//initialize header struct
	header h = {htons(1337), 1, 0, 0, 0, 0, 0, 0, 0, htons(1), htons(0), htons(0), htons(0)};

	//initialize question struct
	question q = {htons(1), htons(1)};

	//copy header to packet
	memcpy(packet, &h, 12);
	//move pointer
	packet = packet + 12;
	//copy formatted url to packet
	memcpy(packet, address, strlen(address));
	//move pointer
	packet = packet + strlen(address) + 1;
	//copy question to packet
	memcpy(packet, &q, 4);
	//calculate packetsize
	packetsize = 17 + strlen(address);
	return pstart;
	
}

/**
 * This function will print a hex dump of the provided packet to the screen
 * to help facilitate debugging.  In your milestone and final submission, you 
 * MUST call dump_packet() with your packet right before calling sendto().  
 * You're welcome to use it at other times to help debug, but please comment those
 * out in your submissions.
 *
 * DO NOT MODIFY THIS FUNCTION
 *
 * data - The pointer to your packet buffer
 * size - The length of your packet
 */
static void dump_packet(unsigned char *data, int size) {
    unsigned char *p = data;
    unsigned char c;
    int n;
    char bytestr[4] = {0};
    char addrstr[10] = {0};
    char hexstr[ 16*3 + 5] = {0};
    char charstr[16*1 + 5] = {0};
    for(n=1;n<=size;n++) {
        if (n%16 == 1) {
            /* store address for this line */
            snprintf(addrstr, sizeof(addrstr), "%.4x",
               ((unsigned int)p-(unsigned int)data) );
        }
            
        c = *p;
        if (isprint(c) == 0) {
            c = '.';
        }

        /* store hex str (for left side) */
        snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
        strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

        /* store char str (for right side) */
        snprintf(bytestr, sizeof(bytestr), "%c", c);
        strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);

        if(n%16 == 0) { 
            /* line completed */
            printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
            hexstr[0] = 0;
            charstr[0] = 0;
        } else if(n%8 == 0) {
            /* half line: add whitespaces */
            strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
            strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
        }
        p++; /* next byte */
    }

    if (strlen(hexstr) > 0) {
        /* print rest of buffer if not empty */
        printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
    }
}

int parse_response(char* packet, int size){
  //separate the packet into its parts
  header h;
  question q;
  int i;
  char* packetstart = packet;
  char* temp = packet;

  //copy data from header into header struct
  memcpy(&h, packet, 12);
  //do ntohs on 16 bit fields
  h.id = ntohs(h.id);
  h.qd = ntohs(h.qd);
  h.an = ntohs(h.an);
  h.ns = ntohs(h.ns);
  h.ar = ntohs(h.ar);
 
  //move the pointer to the start of the name
  packet+=12;
 
  i = 0;
  //find the end of the name
  while(1){
	if((int)packet[i] > 10 && packet[i+1] == 0){
		i+=2;
		break;
	} else {
		i++;
	}
  }
  //move the pointer in the packet to the end of the name
  packet+=i;
  //copy data from question into question struct
  memcpy(&q, packet, 4);
  //do ntohs on 16 bit fields
  q.qtype = ntohs(q.qtype);
  q.qclass = ntohs(q.qclass);
  //move pointer to the end of the question 
  packet+=4;

  //there may be multiple answers:
  int x = h.an;
  if(x <= 0){
	//throw an error
 	printf("NOTFOUND");
	return -1;
  }
  while(x>0){
  	//skip over the pointer to qname
  	packet+=2;
  	answer a;
  	char* ip;
 	struct in_addr ip_buffer;	
  	//copy the data from answer into the answer struct
  	memcpy(&a, packet, 10);
  	//do ntohs on answer
  	a.type = ntohs(a.type);
  	a.c = ntohs(a.c);
  	a.TTL = ntohl(a.TTL);
  	a.rdl = ntohs(a.rdl);

  	//skip to the end of the answer
  	packet+=10;
	i=0;
	if(a.type == 1){
  		//read the ip address
		memcpy(&ip_buffer, packet, 4);
	  	ip = inet_ntoa(ip_buffer);
		//check for auth and print accordingly
		if(h.aa == 1){
			printf("IP\t%s\tauth\n", ip);
			packet+=a.rdl;
			x--;
		} else if(h.aa == 0){
			printf("IP\t%s\tnonauth\n", ip);
			packet+=a.rdl;
			x--;
		} else {
			printf("ERROR\t authorative answer invalid\n");
			return -1;
		}
	  	
	} else if(a.type == 5){
		//read the cname
		char* cname = malloc(100);
		char* cstart = cname;
		unsigned int offset = malloc(sizeof(int));
		packet++;
		//get the alias
		while(packet[i] > '\100' && packet[i] < '\177'){
			cname[i] = packet[i];
			i++;
		}
		//move the packet pointer past the string
		packet+=i;
		packet++;
		//get the domains from the pointer offset
		offset = (int) packet[0];
		temp+=offset;
		//and add them to the cname
		strcat(cname, temp);
		int k;
		//remove length indicators and replace with dots
		for(k = 0; k < strlen(cstart); k++){
			if(cname[k] < '\100'){
				cname[k] = '.';
			}
		}
		cname = cstart;
		//check for auth, print accordingly
		if(h.aa == 1){
			printf("CNAME\t%s\tauth\n", cname);
			packet++;
			x--;
		} else if(h.aa == 0){
			printf("CNAME\t%s\tnonauth\n", cname);
			packet++;	
			x--;
		} else {
			printf("ERROR\t authorative answer invalid\n");
			free(cname);
			return -1;
		}
		free(cname);
	//if a.type is not valid	
	} else {
		//throw an error
		printf("NOTFOUND");
	  	return -1;
	  }
  }	
//exit normally
return 0;
}
		
int main(int argc, char *argv[]) {
  /**
   * I've included some basic code for opening a socket in C, sending
   * a UDP packet, and then receiving a response (or timeout).  You'll 
   * need to fill in many of the details, but this should be enough to
   * get you started.
   */
  unsigned char* packet;
  // process the arguments
  // tokenize the web address
  char* url = strtok(argv[2], ".");
  char* url_buffer = (char *) malloc(1024);
  char* begin = url_buffer;
  //step through the url and replace dots with length indicators
  while(url != NULL){
	*url_buffer = (char *) strlen(url);
	url_buffer++;
	memcpy(url_buffer, url, strlen(url));
	url_buffer = url_buffer+strlen(url);
	url = strtok(NULL, ".");
	}
  //add null byte
  url_buffer = '\0';

  //default portnum
  short portnum = 53;
  char* serverIP = strtok(argv[1], ":@");
  char* IPstart = serverIP;
  int i = 0;

  //check to see if there is a valid alternate port, and for irregular urls.
  while(serverIP != NULL){
	if(i == 1){
		if(strlen(serverIP) > 2){
			printf("ERROR\tInvalid port, check length\n");
			return -1;
		} else {
			portnum = (unsigned short) strtoul(serverIP, NULL, 0);
		}
	}
	if(i >= 2){
		printf("ERROR\tThere are too many colons in the given IP\n");
		return -1;
	}
	i++;
	serverIP=strtok(NULL, ":");
  } 

  
  
  // construct the DNS request
  packet = init_packet(begin);
  free(url_buffer);
  // send the DNS request (and call dump_packet with your request)
 
  // first, open a UDP socket  
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // next, construct the destination address
  struct sockaddr_in out;
  out.sin_family = AF_INET;
  out.sin_port = htons(portnum);
  out.sin_addr.s_addr = inet_addr(IPstart);

  dump_packet(packet, packetsize);
  if (sendto(sock, packet, packetsize, 0, &out, sizeof(out)) < 0) {
    printf("ERROR\tan error occurred sending the packet\n");
	//an error occurred
  }

  // wait for the DNS reply (timeout: 5 seconds)
  struct sockaddr_in in;
  socklen_t in_len;

  // construct the socket set
  fd_set socks;
  FD_ZERO(&socks);
  FD_SET(sock, &socks);

  // construct the timeout
  struct timeval t;
  t.tv_sec = 5;
  t.tv_usec = 0;

  char* in_buffer = malloc(1024);
  // wait to receive, or for a timeout
  if (select(sock + 1, &socks, NULL, NULL, &t)) {
    if (recvfrom(sock, in_buffer, 100, 0, &in, &in_len) < 0) {
      //an error occured
      printf("ERROR\tmessage received but error storing\n");
    }
  } else {
    // a timeout occurred
    printf("NORESPONSE");
    return -1;
  }

  // print out the result
 // dump_packet(in_buffer, 100);
  parse_response(in_buffer, 100);
  free(in_buffer);
  return 0;
}
