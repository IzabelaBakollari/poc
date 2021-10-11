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

struct message{
	struct bcm_msg_head b;
	struct canfd_frame f;
};

void
rxsetup_sock(struct message *msg)
{
	memset(&msg, 0, sizeof(msg));

        msg->b.opcode = RX_SETUP;
        msg->b.flags = CAN_FD_FRAME | SETTIMER | STARTTIMER;
        msg->b.count = 0;
        msg->b.ival1.tv_sec = msg->b.ival2.tv_sec = 0;
        msg->b.ival1.tv_usec = msg->b.ival2.tv_usec = 1;
        msg->b.can_id = 0;
        msg->b.nframes = 1;
}

void
txsetup_sock(struct message *msg)
{
	memset(&msg, 0, sizeof(msg));

	msg->b.opcode = TX_SETUP;
        msg->b.flags = CAN_FD_FRAME | SETTIMER | STARTTIMER | TX_COUNTEVT;
        msg->b.count = 2;
        msg->b.ival1.tv_sec = msg->b.ival2.tv_sec = 1;
        msg->b.ival1.tv_usec = msg->b.ival2.tv_usec = 1;
        msg->b.can_id = 0;
        msg->b.nframes = 1;
}

void
receive_and_check(struct message *msg)
{
	int i,s;
	int sock;
	struct sockaddr_can sa;

	sock = socket(AF_CAN, SOCK_DGRAM, CAN_BCM);

	if (sock < 0) {
		perror("sock");
		printf("Errno = %d\n", errno);
		exit(EXIT_FAILURE);
	}

	sa.can_family = AF_CAN;
	sa.can_ifindex = 6;
	sa.can_addr.tp.rx_id = 0;
	sa.can_addr.tp.tx_id = 0;

	s = sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&sa,
			sizeof(sa));

	if (s < 0) {
        	perror("sendto");
		printf("Errno = %d\n", errno);
        	exit (EXIT_FAILURE);
	}

	for (i = 0; i < sizeof(msg); i++)
    		printf("%x ", ((unsigned char*) &msg)[i]);
   	printf("\n");

	socklen_t len = 0;

	s = recvfrom(sock, &msg, sizeof(msg), 0,
			(struct sockaddr *)&sa, &len);

	if (s < sizeof(msg->b)) {
       		perror("Message recevied is null");
		printf("Errno = %d\n", errno);
		exit(EXIT_FAILURE);
	}
    
	for (i = 12; i < 16; i++) {
		char n = ((unsigned char*) &msg)[i];
		if (n !=0) {
			perror("Padding bytes are corrupted");
			printf("%x\n ", n);
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < sizeof(msg); i++)
 		printf("%x ", ((unsigned char*) &msg)[i]);
 	printf("\n");
}

int
main(void)
{
	int sock;
	struct sockaddr_can sa;
	struct {
		struct bcm_msg_head b;
		struct canfd_frame f;
	} msg;

	sock = socket(AF_CAN, SOCK_DGRAM, CAN_BCM);

	if (sock < 0) {
		perror("sock");
		printf("Errno = %d\n", errno);
		exit(EXIT_FAILURE);
	}

	sa.can_family = AF_CAN;
	sa.can_ifindex = 6;
	sa.can_addr.tp.rx_id = 0;
	sa.can_addr.tp.tx_id = 0;

	connect(sock, (struct sockaddr *)&sa, sizeof(sa));

	rxsetup_sock(&msg);

	memset(&sa, 0, sizeof(sa));
	sa.can_family = AF_CAN;
	sa.can_ifindex = 6;

	memset(&msg, 0, sizeof(msg));
	
	receive_and_check(&msg);

	txsetup_sock(&msg);

	receive_and_check(&msg);

	return 0;
}