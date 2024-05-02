#include "../include/simulator.h"
#include <stdlib.h>
#include <stdio.h>

#define BUF_LIMIT 2000
#define CLEARING_BUFFER 1
#define READY_TO_SEND 0


struct pkt *msg_buf;
int idx_to_fill;
int base;
int nextseq;
int seq;
int ack;
int b_seq;
int window_size;
int buf_comparison;
int buffer_state;

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

  msg_buf[idx_to_fill] = *packet;
  idx_to_fill++;
  buf_comparison++;
  free(packet);
  packet = NULL;

  if (buffer_state == READY_TO_SEND) {
    if (nextseq < base + window_size) {
      tolayer3(0, msg_buf[nextseq]);
      if (base == nextseq) {
        starttimer(0, 40);
      }
      nextseq++;
    }
  }
  seq++;
  ack++;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  int checksum = packet.acknum + packet.seqnum;
  if (packet.checksum == checksum) {
    base = packet.acknum + 1;
    if (base == nextseq) {
      stoptimer(0);
    } else {
      stoptimer(0);
      starttimer(0, 40);
    }
    if (base == nextseq && buf_comparison > nextseq) {
      buffer_state = CLEARING_BUFFER;
      int loop_int;
      if (nextseq + window_size > buf_comparison) {
        loop_int = buf_comparison;
      } else {
        loop_int = nextseq + window_size;
      }
      starttimer(0, 40);
      for (int i = nextseq; i < loop_int; i++) {
        tolayer3(0, msg_buf[i]);
        nextseq++;
      }
      buffer_state = READY_TO_SEND;
    }
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  starttimer(0, 40);
  for (int i = base; i < nextseq; i++) {
    tolayer3(0, msg_buf[i]);
  }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called.*/
void A_init()
{
  nextseq = 0;
  seq = 0;
  ack = 0;
  base = 0;
  buf_comparison = 0;
  window_size = getwinsize();
  msg_buf = calloc(BUF_LIMIT, sizeof(struct pkt));
  idx_to_fill = 0;
  buffer_state = READY_TO_SEND;
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
  if (checksum == packet.checksum && packet.seqnum == b_seq) {
    tolayer5(1, packet.payload);
    b_seq++;
    struct pkt *ack_packet = calloc(1, sizeof(struct pkt));
    ack_packet->seqnum = packet.seqnum;
    ack_packet->acknum = packet.seqnum;
    ack_packet->checksum = packet.seqnum + packet.seqnum;
    tolayer3(1, *ack_packet);
    free(ack_packet);
    ack_packet = NULL;
  } else if (packet.seqnum != b_seq) {
    struct pkt *ack_packet = calloc(1, sizeof(struct pkt));
    ack_packet->seqnum = b_seq - 1;
    ack_packet->acknum = b_seq - 1;
    ack_packet->checksum = ack_packet->seqnum + ack_packet->acknum;
    tolayer3(1, *ack_packet);
    free(ack_packet);
    ack_packet = NULL;
  }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called.*/
void B_init()
{
  b_seq = 0;
}