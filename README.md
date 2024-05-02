# Reliable Data Transfer Protocols

This project implements three unidirectional reliable data transfer protocols: Alternating Bit, Go-Back-N, and Selective Repeat. These protocols ensure the reliable delivery of data over an unreliable network in Linux enironment (designed and tested in CentOS).

## Table of Contents

- [Introduction](#introduction)
- [Protocols](#protocols)
- [Data Visualization](#data_visualization)

## Introduction

In this project, I have implemented three reliable data transfer protocols: Alternating Bit, Go-Back-N, and Selective Repeat. These protocols are used in network communication to ensure that data is delivered reliably, even in the presence of corruption and packet loss.

## Protocols

### Alternating Bit Protocol

The Alternating Bit Protocol is a simple protocol that uses a sequence number to ensure reliable data transfer. It works by sending data packets with alternating sequence numbers and waiting for an acknowledgment from the receiver before sending the next packet. This is stop-and-wait protocol.

### Go-Back-N Protocol

The Go-Back-N Protocol is a sliding window protocol that allows the sender to transmit multiple packets without waiting for individual acknowledgments. It uses a window of sequence numbers and retransmits all unacknowledged packets when a timeout occurs.

### Selective Repeat Protocol

The Selective Repeat Protocol is another sliding window protocol that allows the sender to transmit multiple packets without waiting for individual acknowledgments. Unlike the Go-Back-N Protocol, it only retransmits the packets that have been lost or damaged. The presented implementation utilizes multiple logical timers that are based off of a single software timer for tracking en-route packets.

## Data Visualization

One of the files in the tests folder is a python script (data_automation.py) that can be used to generate performance analysis and related graphs by running the compiled tests in the same folder. It will produce 5 graphs with all three protocols in each of them for further analysis.
