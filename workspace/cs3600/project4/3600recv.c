/*
 * CS3600, Spring 2013
 * Project 4 Starter Code
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

int main() {
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

  // first, open a UDP socket  
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // next, construct the local port
  struct sockaddr_in out;
  out.sin_family = AF_INET;
  out.sin_port = htons(0);
  out.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sock, (struct sockaddr *) &out, sizeof(out))) {
    perror("bind");
    exit(1);
  }

  struct sockaddr_in tmp;
  int len = sizeof(tmp);
  if (getsockname(sock, (struct sockaddr *) &tmp, (socklen_t *) &len)) {
    perror("getsockname");
    exit(1);
  }

  mylog("[bound] %d\n", ntohs(tmp.sin_port));

  // wait for incoming packets
  struct sockaddr_in in;
  socklen_t in_len = sizeof(in);

  // construct the socket set
  fd_set socks;

  // construct the timeout
  struct timeval t;
  t.tv_sec = 10;
  t.tv_usec = 0;

  // our receive buffer
  int buf_len = 1500;
  void* buf = malloc(buf_len);

	int expectedSeq = 0;
	int lastReceived = -1;
  // wait to receive, or for a timeout
  while (1) {
    FD_ZERO(&socks);
    FD_SET(sock, &socks);
		//check to see if we got a packet
    if (select(sock + 1, &socks, NULL, NULL, &t)) {
      int received;
			//reset the timer
			t.tv_sec = 10;
			//get the packet we received
      if ((received = recvfrom(sock, buf, buf_len, 0, (struct sockaddr *) &in, (socklen_t *) &in_len)) < 0) {
        perror("recvfrom");
        exit(1);
      }	
			//construct its header from the read in buffer
      header *myheader = get_header(buf);
			//and its data..
      char *data = get_data(buf);	
			//check to see if this packet is one of the ones we were waiting for
			//in this case, it isnt
			if(myheader->magic == MAGIC && myheader->sequence > expectedSeq){
				//check to see if it is EOF
        if (myheader->eof) {
          mylog("[recv eof]\n");
					lastReceived++;
				//if it isnt, respond to the sender accordingly
				} else {
					mylog("[recv data] %d (%d) %s\n", myheader->sequence, myheader->length, "received out of order");
					mylog("[send ack] %d\n", lastReceived);
				}
				//build outgoing packet
        header *responseheader = make_header(lastReceived, 0, myheader->eof, 1);
        if (sendto(sock, responseheader, sizeof(header), 0, (struct sockaddr *) &in, (socklen_t) sizeof(in)) < 0) {
          perror("sendto");
          exit(1);
        }
				//if EOF before, exit
        if (myheader->eof) {
          mylog("[completed]\n");
          exit(0);
        }
			//if it is the packet we were looking for
			} else if (myheader->magic == MAGIC && myheader->sequence == expectedSeq) {
				//write its data to stdout
        write(1, data, myheader->length);

        mylog("[recv data] %d (%d) %s\n", myheader->sequence, myheader->length, "ACCEPTED (in-order)");
        mylog("[send ack] %d\n", myheader->sequence);
				lastReceived = myheader->sequence;
				expectedSeq+=1492;
				//respond to sender, build outgoing packet
        header *responseheader = make_header(myheader->sequence, 0, myheader->eof, 1);
        if (sendto(sock, responseheader, sizeof(header), 0, (struct sockaddr *) &in, (socklen_t) sizeof(in)) < 0) {
          perror("sendto");
          exit(1);
        }
				//check again for EOF
        if (myheader->eof) {
          mylog("[recv eof]\n");
          mylog("[completed]\n");
          exit(0);
        }
			//if we have already recieved this packet, inform the sender that we have.
      } else if (myheader->magic == MAGIC && myheader->sequence < expectedSeq) {
				mylog("recv data] %d (%d) %s\n", myheader->sequence, myheader->length, "received already");
				mylog("[send ack] %d\n", lastReceived);
        header *responseheader = make_header(lastReceived, 0, myheader->eof, 1);
        if (sendto(sock, responseheader, sizeof(header), 0, (struct sockaddr *) &in, (socklen_t) sizeof(in)) < 0) {
          perror("sendto");
          exit(1);
        }
			} else {
        mylog("[recv corrupted packet]\n");
      }
    } else {
      mylog("[error] timeout occurred\n");
      exit(1);
    }
  }


  return 0;
}
