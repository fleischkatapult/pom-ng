/*
 *  This file is part of pom-ng.
 *  Copyright (C) 2010-2011 Guy Martin <gmsoft@tuxicoman.be>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef __POM_NG_PACKET_H__
#define __POM_NG_PACKET_H__

#include <pom-ng/base.h>
#include <pom-ng/conntrack.h>
#include <pom-ng/timer.h>

#define PACKET_FLAG_FORCE_NO_COPY	0x1
#define PACKET_FLAG_STREAM_BIDIR	0x2

struct proto_process_stack;

struct packet_buffer {

	void *base_buff;
	void *aligned_buff;
	int pool_id;
	struct packet_buffer *next, *prev;

	// The actual data will be after this
	
};

struct packet {

	// Packet description
	uint64_t id; // Unique packet number per input
	size_t len;  // Packet length
	struct timeval ts;
	struct proto *datalink;
	void *buff;
	struct input *input; // Input the packet came from initially
	struct packet_buffer *pkt_buff; // Structure pointing to the buffer information (if any)
	struct packet_multipart *multipart; // Multipart details if the current packet is compose of multiple ones
	unsigned int refcount; // Reference count
	struct packet *prev, *next; // Used internally
};

struct packet_info {
	struct ptype **fields_value;
	struct packet_info *pool_next, *pool_prev;
};

struct packet_info_pool {
	pthread_mutex_t lock;
	struct packet_info *used, *unused;
	unsigned int pool_size, usage;
};

struct packet_multipart_pkt {

	size_t offset, len, pkt_buff_offset;
	struct packet *pkt;
	struct packet_multipart_pkt *prev, *next;

};

struct packet_multipart {

	size_t cur; // Current ammount of data in the buffer
	unsigned int gaps; // Number of gaps
	unsigned int flags;
	struct packet_multipart_pkt *head, *tail;
	struct proto_dependency *proto;
};

struct packet_stream_pkt {

	struct packet *pkt;
	struct proto_process_stack *stack;
	uint32_t seq, ack, plen;
	unsigned int stack_index;
	unsigned int flags;
	struct packet_stream_pkt *prev, *next;

};

struct packet_stream {

	uint32_t cur_seq[CT_DIR_TOT];
	uint32_t cur_ack[CT_DIR_TOT];
	uint32_t cur_buff_size, max_buff_size;
	unsigned int flags;
	pthread_mutex_t lock;
	struct packet_stream_pkt *head[CT_DIR_TOT], *tail[CT_DIR_TOT];
	int (*handler) (void *priv, struct packet *p, struct proto_process_stack *stack, unsigned int stack_index);
	void *priv;
	struct timer *t;
};


struct packet_stream_parser {
	unsigned int max_line_size;
	char *buff;
	unsigned int bufflen;
	unsigned int buffpos;
	char *pload;
	unsigned int plen;
};

int packet_buffer_pool_get(struct packet *pkt, size_t size, size_t align_offset);

struct packet *packet_pool_get();
struct packet *packet_clone(struct packet *src, unsigned int flags);
int packet_pool_release(struct packet *p);

struct packet_multipart *packet_multipart_alloc(struct proto_dependency *proto_dep, unsigned int flags);
int packet_multipart_cleanup(struct packet_multipart *m);
int packet_multipart_add_packet(struct packet_multipart *multipart, struct packet *pkt, size_t offset, size_t len, size_t pkt_buff_offset);
int packet_multipart_process(struct packet_multipart *multipart, struct proto_process_stack *stack, unsigned int stack_index);

struct packet_stream* packet_stream_alloc(uint32_t start_seq, uint32_t start_ack, int direction, uint32_t max_buff_size, unsigned int timeout, unsigned int flags, int (*handler) (void *priv, struct packet *p, struct proto_process_stack *stack, unsigned int stack_index), void *priv);
int packet_stream_cleanup(struct packet_stream *stream);
int packet_stream_process_packet(struct packet_stream *stream, struct packet *pkt, struct proto_process_stack *stack, unsigned int stack_index, uint32_t seq, uint32_t ack);

struct packet_stream_parser *packet_stream_parser_alloc(unsigned int max_line_size);
int packet_stream_parser_add_payload(struct packet_stream_parser *sp, void *pload, unsigned int len);
int packet_stream_parser_get_line(struct packet_stream_parser *sp, char **line, unsigned int *len);
int packet_stream_parser_get_remaining(struct packet_stream_parser *sp, void **pload, unsigned int *len);
int packet_stream_parser_cleanup(struct packet_stream_parser *sp);
#endif
