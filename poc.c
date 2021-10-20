#include <linux/can.h>
#include <linux/can/bcm.h>
#include <net/if.h>
#include <sys/socket.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct message {
	struct bcm_msg_head b;
	struct canfd_frame f;
};

static void print_message(struct message *msg, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (i % 16 == 0)
			printf("%04x  ", i);
		printf("%02x ", ((unsigned char*) msg)[i]);
		if (i % 16 == 15)
			printf("\n");
	}

	if (i % 16)
		printf("\n");
}

static int communicate_and_check(struct message *msg, int sock, const struct sockaddr_can *sa, __u32 expected_opcode)
{
	int i, r;
	char n;

	printf("Sending:\n");
	print_message(msg, sizeof(*msg));

	alarm(1);

	r = sendto(sock, msg, sizeof(*msg), 0, (struct sockaddr *)sa, sizeof(*sa));
	if (r < 0) {
		perror("sendto");
		exit(EXIT_FAILURE);
	}

	r = recvfrom(sock, msg, sizeof(*msg), 0, NULL, NULL);
	if (r < 0) {
		perror("recvfrom");
		exit(EXIT_FAILURE);
	}

	alarm(0);

	printf("Received:\n");
	print_message(msg, r);
	if (r < sizeof(msg->b)) {
		fprintf(stderr, "The received message is too short, only %d bytes.\n", r);
		exit(EXIT_FAILURE);
	}

	if (msg->b.opcode != expected_opcode) {
		fprintf(stderr, "Unexpected opcode in the reply: 0x%08x\n", msg->b.opcode);
		exit(EXIT_FAILURE);
	}

	r = 0;
	for (i = offsetof(struct bcm_msg_head, count) + sizeof(((struct bcm_msg_head){}).count);
	     i < offsetof(struct bcm_msg_head, ival1);
	     i++) {
		n = ((char*) msg)[i];
		if (n != 0 && r == 0) {
			fprintf(stderr, "Non-zero padding byte in the reply!\n");
			r = 1;
		}
	}

	return r;
}

int main(int argc, char *argv[])
{
	int sock, r;
	struct message msg;
	unsigned ifindex;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <CAN-interface-name>\n"
		                "Hint - a virtual CAN device will suffice. It can be set up with:\n"
		                "  ip link add name testcan type vcan && ip link set testcan up\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ifindex = if_nametoindex(argv[1]);
	if (!ifindex) {
		perror("if_nametoindex");
		exit(EXIT_FAILURE);
	}

	const struct sockaddr_can sa = {
		.can_family = AF_CAN,
		.can_ifindex = ifindex,
	};

	sock = socket(AF_CAN, SOCK_DGRAM, CAN_BCM);
	if (sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	r = connect(sock, (const struct sockaddr *)&sa, sizeof(sa));
	if (r < 0) {
		perror("connect");
		exit(EXIT_FAILURE);
	}

	msg = (struct message) {
		.b.opcode = TX_SETUP,
		.b.flags = CAN_FD_FRAME | SETTIMER | STARTTIMER | TX_COUNTEVT,
		.b.count = 2,
		.b.ival1 = { 0, 1 },
		.b.nframes = 1,
	};
	r = communicate_and_check(&msg, sock, &sa, TX_EXPIRED);

	msg = (struct message) {
		.b.opcode = RX_SETUP,
		.b.flags = CAN_FD_FRAME | SETTIMER | STARTTIMER,
		.b.ival1 = { 0, 1 },
		.b.nframes = 1,
	};
	r |= communicate_and_check(&msg, sock, &sa, RX_TIMEOUT);

	msg = (struct message) {
		.b.opcode = TX_SEND,
		.b.flags = CAN_FD_FRAME,
		.b.nframes = 1,
		.f.len = 1,
		.f.data = { 0x42, },
	};
	r |= communicate_and_check(&msg, sock, &sa, RX_CHANGED);

	return r;
}
