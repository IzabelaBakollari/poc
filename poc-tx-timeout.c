#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/bcm.h>

void
txsetup_sock(int sock)
{
	int i,s;
	struct sockaddr_can sa;
	struct {
		struct bcm_msg_head b;
		struct canfd_frame f;
	} msg;
	// char buf[sizeof(msg)];

	memset(&msg, 0, sizeof(msg));

	sa.can_family = AF_CAN;
	sa.can_ifindex = 0;
	sa.can_addr.tp.rx_id = 0;
	sa.can_addr.tp.tx_id = 0;

        msg.b.opcode = TX_SEND;
        msg.b.flags = CAN_FD_FRAME | SETTIMER | STARTTIMER;
        msg.b.count = 0;
        msg.b.ival1.tv_sec = msg.b.ival2.tv_sec = 0;
        msg.b.ival1.tv_usec = msg.b.ival2.tv_usec = 1;
        msg.b.can_id = 0;
        msg.b.nframes = 1;

	// memcpy(buf, &msg, sizeof(buf));
 
	for (i = 0; i < sizeof(msg); i++)
    		printf("%x ", ((unsigned char*) &msg)[i]);
   	printf("\n");

	s = sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&sa,
			sizeof(sa));
	
	printf("size of sendto = %d\n", s);

	if (s < 0) {

        perror("sendto");
		printf("Errno = %d\n", errno);
        exit (EXIT_FAILURE);
	}
}

int
main(void)
{
	int i,s;
	int sock;
	struct sockaddr_can sa;
	struct {
		struct bcm_msg_head b;
		struct canfd_frame f;
	} msg;
	//char buf[sizeof(msg)];

	sock = socket(AF_CAN, SOCK_DGRAM, CAN_BCM);

	if (sock < 0) {
		perror("sock");
		printf("Errno = %d\n", errno);

        return 0;
	}

	sa.can_family = AF_CAN;
	sa.can_ifindex = 0;
	sa.can_addr.tp.rx_id = 0;
	sa.can_addr.tp.tx_id = 0;

	connect(sock, (struct sockaddr *)&sa, sizeof(sa));

	txsetup_sock(sock);

	memset(&sa, 0, sizeof(sa));
	sa.can_family = AF_CAN;
	sa.can_ifindex = 0;
	socklen_t len = 0;

	memset(&msg, 0, sizeof(msg));

	s = recvfrom(sock, &msg, sizeof(msg), 0,
			(struct sockaddr *)&sa, &len);
	
	printf("size of recvfrom = %d\n", s);
	
	if (s < 0) {
       		perror("recvfrom");
		printf("Errno = %d\n", errno);

        return 0;
	}
    
	// memcpy(buf, &msg, sizeof(buf));

	for (i = 0; i < sizeof(msg); i++)
 		printf("%x ", ((unsigned char*) &msg)[i]);
 	printf("\n");

	return 0;
}

