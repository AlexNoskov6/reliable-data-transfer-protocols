#include "../include/simulator.h"
#include <stdlib.h>
#include <stdio.h>

#define BUF_LIMIT 2000
#define CLEARING_BUFFER 1
#define READY_TO_SEND 0
#define ACKED 1
#define NOT_ACKED 0

struct sent_packet {
  struct pkt *packet;
  float timeout;
  int acked;
  struct sent_packet *next;
};

struct sent_packet *sent_packets_list;
struct pkt *msg_buf;
struct pkt *b_msg_buf;
int idx_to_fill;
int b_buf_idx;
int base;
int b_base;
int nextseq;
int seq;
int ack;
int b_seq;
int window_size;
int b_window_size;
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

  if (buffer_state == READY_TO_SEND) {
    if (nextseq < base + window_size) {
      struct sent_packet *sent_packet = calloc(1, sizeof(struct sent_packet));
      sent_packet->packet = &msg_buf[nextseq];
      sent_packet->timeout = get_sim_time() + 30;
      sent_packet->acked = NOT_ACKED;
      sent_packet->next = NULL;
      if (sent_packets_list == NULL) {
        sent_packets_list = sent_packet;
      } else {
        struct sent_packet *current = sent_packets_list;
        while (current->next != NULL) {
          current = current->next;
        }
        current->next = sent_packet;
      }
      tolayer3(0, msg_buf[nextseq]);
      if (base == nextseq) {
        starttimer(0, 10);
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
    struct sent_packet *current = sent_packets_list;
    while (current != NULL) {
      if (current->packet->acknum == packet.acknum) {
        current->acked = ACKED;
        break;
      }
      current = current->next;
    }
    if (packet.acknum == base) {
      while (current != NULL && current->acked == ACKED) {
        base++;
        current = current->next;
      }
    }
    if (base == nextseq) {
      stoptimer(0);
    }
    if (base == nextseq && buf_comparison > nextseq) {
      buffer_state = CLEARING_BUFFER;
      int loop_int;
      if (nextseq + window_size > buf_comparison) {
        loop_int = buf_comparison;
      } else {
        loop_int = nextseq + window_size;
      }
      starttimer(0, 10);
      for (int i = nextseq; i < loop_int; i++) {
        struct sent_packet *sent_packet = calloc(1, sizeof(struct sent_packet));
        sent_packet->packet = &msg_buf[i];
        sent_packet->timeout = get_sim_time() + 30;
        sent_packet->acked = NOT_ACKED;
        sent_packet->next = NULL;
        struct sent_packet *current = sent_packets_list;
        while (current->next != NULL) {
          current = current->next;
        }
        current->next = sent_packet;
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
  struct sent_packet *current = sent_packets_list;
  while (current != NULL) {
    if (get_sim_time() > current->timeout && current->acked == NOT_ACKED){
      tolayer3(0, *current->packet);
      current->timeout = get_sim_time() + 30;
    }
    current = current->next;
  }
  starttimer(0, 10);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. */
void A_init()
{
  sent_packets_list = NULL;
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
  if (checksum == packet.checksum && packet.seqnum < b_base + b_window_size) {
    b_buf_idx = b_base + (packet.seqnum - b_base);
    if (b_buf_idx >= BUF_LIMIT) {
      b_buf_idx -= BUF_LIMIT - 1;
    }
    if (packet.seqnum >= b_base) {
      b_msg_buf[b_buf_idx] = packet;
    }
    if (packet.seqnum == b_base) {
      while (b_msg_buf[b_buf_idx].seqnum != -1) {
        tolayer5(1, b_msg_buf[b_buf_idx].payload);
        b_base++;
        b_buf_idx++;
      }
    }
    struct pkt *ack_packet = calloc(1, sizeof(struct pkt));
    ack_packet->seqnum = packet.seqnum;
    ack_packet->acknum = packet.acknum;
    ack_packet->checksum = packet.seqnum + packet.acknum;
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
  b_base = 0;
  b_buf_idx = 0;
  b_window_size = getwinsize();
  b_msg_buf = calloc(BUF_LIMIT, sizeof(struct pkt));
  for (int i = 0; i < BUF_LIMIT; i++) {
    b_msg_buf[i].seqnum = -1;
  }
}