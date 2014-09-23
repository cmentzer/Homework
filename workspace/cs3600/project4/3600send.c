/*
 * CS3600, Spring 2013
 * Project 4
	 Clayton Mentzer 
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

#include "3600sendrecv.h"
//set the data size
static int DATA_SIZE = 1492;
//sequence starts at 0
unsigned int sequence = 0;

void usage() {
	printf("Usage: 3600send host:port\n");
	exit(1);
}

/**
 * Reads the next block of data from stdin
 */
int get_next_data(char *data, int size) {
	return read(0, data, size);
}

/**
 * Builds and returns the next packet, or NULL
 * if no more data is available.
 */
char *get_next_packet(int sequence, int *len) {
	char *data = malloc(DATA_SIZE);
	int data_len = get_next_data(data, DATA_SIZE);

	if (data_len == 0) {
		free(data);
		return NULL;
	}
	//create the header
	header *myheader = make_header(sequence, data_len, 0, 0);
	//allocate space for the packet
	char *packet = malloc(sizeof(header) + data_len);
	memcpy(packet, myheader, sizeof(header));
	memcpy(((char *) packet) +sizeof(header), data, data_len);

	//free the header and the packet
	free(data);
	free(myheader);

	*len = sizeof(header) + data_len;

	return packet;
}

//the send_packet function in a packet and sends that packet on the given
//socket.
int send_packet(int sock, struct sockaddr_in out, char* packet) {
	//get the header of the passed in packet
	header *tempheader = malloc(sizeof(header));
	tempheader = get_header(packet);
	char* data = get_data(packet);
	int packet_len = 1500;
	//find the size of the passed in packet
	if (packet == NULL){
		free(tempheader);
		return 0;
	}
	//print to mylog
	mylog("[send data] %d (%d)\n", tempheader->sequence, tempheader->length);

	//if sendto doesnt work, return an error
	if (sendto(sock, packet, packet_len, 0, (struct sockaddr *) &out, (socklen_t) sizeof(out)) < 0) {
		perror("sendto");
		exit(1);
	}
  free(tempheader);
	return 1;

}
//send_final_packet sends the EOF packet
void send_final_packet(int sock, struct sockaddr_in out) {
	header *myheader = make_header(sequence-DATA_SIZE+1, 0, 1, 0);
	mylog("[send eof]\n");

	if (sendto(sock, myheader, sizeof(header), 0, (struct sockaddr *) &out, (socklen_t) sizeof(out)) < 0) {
		perror("sendto");
		exit(1);
	}
}

int main(int argc, char *argv[]) {
	/**
	 * I've included some basic code for opening a UDP socket in C, 
	 * binding to a empheral port, printing out the port number.
	 * 
	 * I've also included a very simple transport protocol that simply
	 * acknowledges every received packet.  It has a header, but does
	 * not do any error handling (i.e., it does not have sequence 
	 * numbers, timeouts, retries, a "window"). You will
	 * need to fill in many of the details, but this should be enough to
	 * get you started.
	 */

	// extract the host IP and port
	if ((argc != 2) || (strstr(argv[1], ":") == NULL)) {
		usage();
	}
	char *tmp = (char *) malloc(strlen(argv[1])+1);
	strcpy(tmp, argv[1]);

	char *ip_s = strtok(tmp, ":");
	char *port_s = strtok(NULL, ":");

	// first, open a UDP socket  
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// next, construct the local port
	struct sockaddr_in out;
	out.sin_family = AF_INET;
	out.sin_port = htons(atoi(port_s));
	out.sin_addr.s_addr = inet_addr(ip_s);

	// socket for received packets
	struct sockaddr_in in;
	socklen_t in_len = sizeof(in);

	// construct the socket set
	fd_set socks;

	int time = 100000;
	// construct the timeout
	struct timeval t;
	t.tv_sec = 0;
	t.tv_usec = time;

	//build an array of packets of size WINDOW_SIZE and set all its avlues to null
	WINDOW_SIZE = 20;
	char* window[WINDOW_SIZE];
	for(int i = 0; i < WINDOW_SIZE; i++){
		window[i] = NULL;
  }
	//reset window size to reflect the optimal size
	WINDOW_SIZE = 3;

	//checks to see if there is any more data to bet gotten
	int outOfData = 0;
	//keeps track of the ack that send has acked up to
	unsigned int expectedAck = 0;
	//keeps track of if the final packet has been sent
	int finalsent =0;
	while (1) {
		//reset the clock
		t.tv_usec = time;
		int dup = 0;
		int packet_len = 1500;
		//fill they array of packets with the first WINDOW_SIZE packets
		//of data from STDIN
		mylog("check to see if any elements in array are less than thing\n");
		for(int i = 0; i < WINDOW_SIZE; i++){
			if(window[i] != NULL){
				header *temp = get_header(window[i]);
				//if any data in the array is outdated, remove it
				if(temp->sequence < expectedAck){
					
					window[i] = NULL;
				}
			} else
			//if there is an empty slot in the array, fill it
			if(window[i] == NULL){
				char* packet = get_next_packet(sequence, &packet_len);
					window[i] = packet;
					sequence+=DATA_SIZE;
			}
		}
		//check to see if the entire array is null
		//if they are, there is no more data to get from stdin
		int allNULL = 1;
		for(int i = 0; i < WINDOW_SIZE; i++){
			if(window[i] != NULL){
				allNULL = 0;
			}
		}
		if(allNULL == 1){
			//done, send final packet
			send_final_packet(sock, out);
			finalsent++;
			if(finalsent >= 7){
				exit(0);
			}
		}
			//send packets
			for(int i = 0; i < WINDOW_SIZE; i++){
				if(window[i] != NULL){
					send_packet(sock, out, window[i]);
				}
			}
			//as long as we haven't received any out of order or bad packets
			while (dup != 1) {
				FD_ZERO(&socks);
				FD_SET(sock, &socks);

				// wait to receive, or for a timeout
				int timedout = select(sock + 1, &socks, NULL, NULL, &t);
				if(timedout == 0){
					if(WINDOW_SIZE > 4){
						WINDOW_SIZE = 3;
					}
					//if we have a timeout, we resend the packet that the recv is waiting for
					for(int i = 0; i < WINDOW_SIZE; i++){
						if(window[i] != NULL){
							header *tempheader = get_header(window[i]);
							if(tempheader->sequence == expectedAck){
								send_packet(sock, out, window[i]);
								//reset the clock
								t.tv_usec = time;
							}
						}
					}
				}
				//if timedout > 0 we have gotten a response
				else if(timedout > 0){
					unsigned char buf[10000];
					int buf_len = sizeof(buf);
					int received;
					//read the packet we have received
					if ((received = recvfrom(sock, &buf, buf_len, 0, (struct sockaddr *) &in, (socklen_t *) &in_len)) < 0) {
						perror("recvfrom");
						exit(1);
					}
					//get the header of that packet.				
					header *myheader = get_header(buf);
					//if it is an EOF, we are done
					if((myheader->magic==MAGIC)&&(myheader->ack==1)&&(myheader->eof==1)){
						mylog("[recv ack] EOF\n");
						exit(0);
					}
					//otherwise we check to see if the sequence number is the one we are looking for
					if ((myheader->magic == MAGIC) && (myheader->ack == 1)) {
						mylog("[recv ack] %d\n", myheader->sequence);
						//if it has sequence -1, we need to re-send the first packet
						if(myheader->sequence >= expectedAck){
							if(myheader->sequence == -1){
								expectedAck = 0;
							//otherwise we modify out expectedAck to reflect the packet we are waiting to be acked
							} else {
								expectedAck = myheader->sequence+=DATA_SIZE;
								mylog("expectedAck is %d\n", expectedAck);
							}
							//we then go back to our array, and remove any elements that are less than that
							for(int i = 0; i < WINDOW_SIZE; i++){
								if(window[i] != NULL){
									//get the header of that packet
									header *thead = get_header(window[i]);
									if(thead->sequence < expectedAck){
										mylog("window[%d] is %d expecteedack is %d\n", i, thead->sequence, expectedAck);
										char* packet = get_next_packet(sequence, &packet_len);
										sequence+=DATA_SIZE;
										window[i] = packet;
										if(packet != NULL){
												send_packet(sock, out, window[i]);
										} else {	
											dup = 1;
										}
										if(WINDOW_SIZE < 10){
						//					WINDOW_SIZE++;
										}
									}
								}
							}
						}
					} else {
						mylog("[recv corrupted ack] %x %d\n", MAGIC, sequence);
						return -1;
					}
				} else {
					mylog("[error] timeout occurred\n");
					return -1;
				}
			}
		
	}
}
