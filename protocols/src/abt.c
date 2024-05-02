#include "../include/simulator.h"
#include <stdlib.h>
#include <stdio.h>

#define IN_TRANSIT 1
#define READY_FOR_TRANSIT 0
#define BUF_LIMIT 1000

struct pkt *msg_buf;
int idx_to_fill;
int idx_to_take;
int idx_to_retransmit;
int seq;
int ack;
int network_indicator;
int mod_counter;
int b_mod_counter;

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
  struct pkt *packet = calloc(1, sizeof(struct pkt));
  packet->seqnum = seq;
  packet->acknum = ack;
  packet->checksum = seq + ack;
  for (int i = 0; i < 20; i++) {
    packet->payload[i] = message.data[i];
    packet->checksum += message.data[i];
  }

  if (idx_to_fill >= BUF_LIMIT) {
    idx_to_fill = 0;
  }
  if (idx_to_take >= BUF_LIMIT) {
    idx_to_take = 0;
  }

  msg_buf[idx_to_fill] = *packet;
  idx_to_fill++;
  free(packet);
  packet = NULL;

  if (network_indicator == READY_FOR_TRANSIT) {
    network_indicator = IN_TRANSIT;
    tolayer3(0, msg_buf[idx_to_take]);
    starttimer(0, 30);
    idx_to_retransmit = idx_to_take;
    idx_to_take++;
  }

  mod_counter++;
  seq = mod_counter % 2;
  ack = mod_counter % 2;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  int checksum = packet.acknum + packet.seqnum;
  if (checksum == packet.checksum && packet.acknum == msg_buf[idx_to_retransmit].acknum) {
    stoptimer(0);
    if (idx_to_take != idx_to_fill) {
      tolayer3(0, msg_buf[idx_to_take]);
      starttimer(0, 30);
      idx_to_retransmit = idx_to_take;
      idx_to_take++;
    } else {
      network_indicator = READY_FOR_TRANSIT;
    }
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  tolayer3(0, msg_buf[idx_to_retransmit]);
  starttimer(0, 30);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. */
void A_init()
{
  msg_buf = calloc(BUF_LIMIT, sizeof(struct pkt));
  idx_to_fill = 0;
  idx_to_take = 0;
  seq = 0;
  ack = 0;
  mod_counter = 0;
  network_indicator = READY_FOR_TRANSIT;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  int checksum = packet.acknum + packet.seqnum;
  for (int i = 0; i < 20; i++) {
    checksum += packet.payload[i];
  }
  if (checksum == packet.checksum && packet.seqnum == b_mod_counter % 2) {
    tolayer5(1, packet.payload);
    b_mod_counter++;
    struct pkt *ack_packet = calloc(1, sizeof(struct pkt));
    ack_packet->seqnum = packet.seqnum;
    ack_packet->acknum = packet.seqnum;
    ack_packet->checksum = packet.seqnum + packet.seqnum;
    tolayer3(1, *ack_packet);
    free(ack_packet);
    ack_packet = NULL;
  } else if (packet.seqnum == (b_mod_counter + 1) % 2) {
    struct pkt *ack_packet = calloc(1, sizeof(struct pkt));
    ack_packet->seqnum = packet.seqnum;
    ack_packet->acknum = packet.seqnum;
    ack_packet->checksum = packet.seqnum + packet.seqnum;
    tolayer3(1, *ack_packet);
    free(ack_packet);
    ack_packet = NULL;
  }
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called.*/
void B_init()
{
  b_mod_counter = 0;
}