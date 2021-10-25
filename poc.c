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
#include <sys/ioctl.h>
#include <net/if.h>

struct message {
	struct bcm_msg_head b;
	struct canfd_frame f;
};

static void print_message(struct message *msg, int s)
{
	for (int i=0; i<s; i++)
		printf("%x ", ((unsigned char*) msg)[i]);
	printf("\n");
}

static int receive_and_check(struct message *msg, int sock, struct sockaddr_can *sa)
{
	int s = sendto(sock, msg, sizeof(*msg), 0, (struct sockaddr *)sa,
			sizeof(*sa));

	if (s < 0) {
		perror("Sending incomplete message");
		printf("Errno = %d\n", errno);
		exit (EXIT_FAILURE);
	}

	print_message(msg, s);

	socklen_t len = 0;

	s = recvfrom(sock, msg, sizeof(*msg), 0,
			(struct sockaddr *)sa, &len);

	if (s < sizeof(msg->b)) {
		printf("Recieved message is incomplete\n");
		exit(EXIT_FAILURE);
	}

	print_message(msg, s);

	int b =0;
	for (int i = 12; i < 16; i++) {
		char n = ((unsigned char*) msg)[i];
		if (n != 0) {
			printf("Padding bytes with index number %x are corrupted\n", i);
			printf("%x ", n);
			b = 1;
		}
	}
	return b;
}

static int txsetup(struct message *msg, int sock, struct sockaddr_can *sa)
{
	memset(msg, 0, sizeof(*msg));

	msg->b.opcode = TX_SETUP;
	msg->b.flags = CAN_FD_FRAME | SETTIMER | STARTTIMER | TX_COUNTEVT;
	msg->b.count = 2;
	msg->b.ival1.tv_sec = msg->b.ival2.tv_sec = 1;
	msg->b.ival1.tv_usec = msg->b.ival2.tv_usec = 1;
	msg->b.can_id = 0;
	msg->b.nframes = 1;

	return receive_and_check(msg, sock, sa);
}

static int rxsetup(struct message *msg, int sock, struct sockaddr_can *sa)
{
	memset(msg, 0, sizeof(*msg));

	msg->b.opcode = RX_SETUP;
	msg->b.flags = CAN_FD_FRAME | SETTIMER | STARTTIMER;
	msg->b.count = 0;
	msg->b.ival1.tv_sec = msg->b.ival2.tv_sec = 0;
	msg->b.ival1.tv_usec = msg->b.ival2.tv_usec = 1;
	msg->b.can_id = 0;
	msg->b.nframes = 1;

	return receive_and_check(msg, sock, sa);
}

static int rxchanged(struct message *msg, int sock, struct sockaddr_can *sa)
{
	memset(msg, 0, sizeof(*msg));

	msg->b.opcode = TX_SEND;
	msg->b.flags = CAN_FD_FRAME;
	msg->b.count = 0;
	msg->b.ival1.tv_sec = msg->b.ival2.tv_sec = 0;
	msg->b.ival1.tv_usec = msg->b.ival2.tv_usec = 1;
	msg->b.can_id = 0;
	msg->b.nframes = 1;

	return receive_and_check(msg, sock, sa);
}

int main(int argc, char *argv[])
{
	struct sockaddr_can sa;
	struct message msg;

	int sock = socket(AF_CAN, SOCK_DGRAM, CAN_BCM);

	if (sock < 0) {
		perror("Socket failure");
		printf("Errno = %d\n", errno);
		exit(EXIT_FAILURE);
	}

	if (argc != 2) {
		printf("Please provide one argument\n");
		exit(EXIT_FAILURE);
	}

	char *ifname= argv[1];

	unsigned int getindex = if_nametoindex(ifname);

	if (getindex == 0) {
		perror("Interface is not valid");
		printf("Errno = %d\n", errno);
		exit(EXIT_FAILURE);
	}

	memset(&sa, 0, sizeof(sa));
	sa.can_family = AF_CAN;
	sa.can_ifindex = getindex;
	sa.can_addr.tp.rx_id = 0;
	sa.can_addr.tp.tx_id = 0;

	int con = connect(sock, (struct sockaddr *)&sa, sizeof(sa));

	if (con < 0) {
		perror("Connection failure");
		printf("Errno = %d\n", errno);
		exit(EXIT_FAILURE);
	}

	int r = txsetup(&msg, sock, &sa);
	r |= rxsetup(&msg, sock, &sa);
	r |= rxchanged(&msg, sock, &sa);

	return r;
}