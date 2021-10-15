#include <linux/can.h>
#include <linux/can/bcm.h>
#include <net/if.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct message {
	struct bcm_msg_head b;
	struct canfd_frame f;
};

void print_message(struct message *msg, int len)
{	
	int i;

	for (i = 0; i < len; i++)
    		printf("%x ", ((unsigned char*) msg)[i]);
   	printf("\n");
}

void communicate_and_check(struct message *msg, int sock, struct sockaddr_can *sa, __u32 expected_opcode)
{
	int i, r, n;

	printf("Sending:\n");
	print_message(msg, sizeof(*msg));

	r = sendto(sock, msg, sizeof(*msg), 0, (struct sockaddr *)sa, sizeof(*sa));
	if (r < 0) {
		perror("sendto");
		exit (EXIT_FAILURE);
	}

	r = recvfrom(sock, msg, sizeof(*msg), 0, NULL, NULL);
	printf("Received:\n");
	print_message(msg, r);
	if (r < sizeof(msg->b)) {
       		perror("Did not receive a full message");
		exit(EXIT_FAILURE);
	}

	if (msg->b.opcode != expected_opcode) {
		fprintf(stderr, "Unexpected reply opcode received: %u\n", msg->b.opcode);
		exit(EXIT_FAILURE);
	}
    
	for (i = 12; i < 16; i++) {
		n = ((unsigned char*) msg)[i];
		if (n != 0) {
			fprintf(stderr, "Non-zero padding byte in the reply!\n");
			exit(EXIT_FAILURE);
		}
	}
}

int main(int argc, char *argv[])
{
	int sock, r;
	struct sockaddr_can sa;
	struct message msg;
	unsigned ifindex;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <CAN-interface-name>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ifindex = if_nametoindex(argv[1]);
	if (!ifindex) {
		perror("if_nametoindex");
		exit(EXIT_FAILURE);
	}

	memset(&sa, 0, sizeof(sa));
	sa.can_family = AF_CAN;
	sa.can_ifindex = ifindex;
	sa.can_addr.tp.rx_id = 0;
	sa.can_addr.tp.tx_id = 0;

	sock = socket(AF_CAN, SOCK_DGRAM, CAN_BCM);
	if (sock < 0) {
		perror("sock");
		exit(EXIT_FAILURE);
	}

	r = connect(sock, (struct sockaddr *)&sa, sizeof(sa));
	if (r < 0) {
		perror("connect");
		exit(EXIT_FAILURE);
	}

	msg = (struct message) {
		.b.opcode = TX_SETUP,
		.b.flags = CAN_FD_FRAME | SETTIMER | STARTTIMER | TX_COUNTEVT,
		.b.count = 2,
		.b.ival1 = { 0, 1 },
		.b.ival2 = { 0, 1 },
		.b.nframes = 1,
	};
	communicate_and_check(&msg, sock, &sa, TX_EXPIRED);

	msg = (struct message) {
		.b.opcode = RX_SETUP,
		.b.flags = CAN_FD_FRAME | SETTIMER | STARTTIMER,
		.b.ival1 = { 0, 1 },
		.b.ival2 = { 0, 1 },
		.b.nframes = 1,
	};
	communicate_and_check(&msg, sock, &sa, RX_TIMEOUT);

	msg = (struct message) {
		.b.opcode = TX_SEND,
		.b.flags = CAN_FD_FRAME,
		.b.nframes = 1,
		.f.len = 1,
		.f.data = { 0x42, },
	};
	communicate_and_check(&msg, sock, &sa, RX_CHANGED);

	return 0;
}
