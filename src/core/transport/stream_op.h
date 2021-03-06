/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef GRPC_INTERNAL_CORE_TRANSPORT_STREAM_OP_H
#define GRPC_INTERNAL_CORE_TRANSPORT_STREAM_OP_H

#include <grpc/grpc.h>
#include <grpc/support/port_platform.h>
#include <grpc/support/slice.h>
#include <grpc/support/time.h>
#include "src/core/transport/metadata.h"

/* this many stream ops are inlined into a sopb before allocating */
#define GRPC_SOPB_INLINE_ELEMENTS 16

/* Operations that can be performed on a stream.
   Used by grpc_stream_op. */
typedef enum grpc_stream_op_code {
  /* Do nothing code. Useful if rewriting a batch to exclude some operations.
     Must be ignored by receivers */
  GRPC_NO_OP,
  GRPC_OP_METADATA,
  /* Begin a message/metadata element/status - as defined by
     grpc_message_type. */
  GRPC_OP_BEGIN_MESSAGE,
  /* Add a slice of data to the current message/metadata element/status.
     Must not overflow the forward declared length. */
  GRPC_OP_SLICE
} grpc_stream_op_code;

/* Arguments for GRPC_OP_BEGIN */
typedef struct grpc_begin_message {
  /* How many bytes of data will this message contain */
  gpr_uint32 length;
  /* Write flags for the message: see grpc.h GRPC_WRITE_xxx */
  gpr_uint32 flags;
} grpc_begin_message;

typedef struct grpc_linked_mdelem {
  grpc_mdelem *md;
  struct grpc_linked_mdelem *next;
  struct grpc_linked_mdelem *prev;
} grpc_linked_mdelem;

typedef struct grpc_mdelem_list {
  grpc_linked_mdelem *head;
  grpc_linked_mdelem *tail;
} grpc_mdelem_list;

typedef struct grpc_metadata_batch {
  grpc_mdelem_list list;
  grpc_mdelem_list garbage;
  gpr_timespec deadline;
} grpc_metadata_batch;

void grpc_metadata_batch_init(grpc_metadata_batch *comd);
void grpc_metadata_batch_destroy(grpc_metadata_batch *comd);
void grpc_metadata_batch_merge(grpc_metadata_batch *target,
                               grpc_metadata_batch *add);

void grpc_metadata_batch_link_head(grpc_metadata_batch *comd,
                                   grpc_linked_mdelem *storage);
void grpc_metadata_batch_link_tail(grpc_metadata_batch *comd,
                                   grpc_linked_mdelem *storage);

void grpc_metadata_batch_add_head(grpc_metadata_batch *comd,
                                  grpc_linked_mdelem *storage,
                                  grpc_mdelem *elem_to_add);
void grpc_metadata_batch_add_tail(grpc_metadata_batch *comd,
                                  grpc_linked_mdelem *storage,
                                  grpc_mdelem *elem_to_add);

void grpc_metadata_batch_filter(grpc_metadata_batch *comd,
                                grpc_mdelem *(*filter)(void *user_data,
                                                       grpc_mdelem *elem),
                                void *user_data);

#ifndef NDEBUG
void grpc_metadata_batch_assert_ok(grpc_metadata_batch *comd);
#else
#define grpc_metadata_batch_assert_ok(comd) \
  do {                                      \
  } while (0)
#endif

/* Represents a single operation performed on a stream/transport */
typedef struct grpc_stream_op {
  /* the operation to be applied */
  enum grpc_stream_op_code type;
  /* the arguments to this operation. union fields are named according to the
     associated op-code */
  union {
    grpc_begin_message begin_message;
    grpc_metadata_batch metadata;
    gpr_slice slice;
  } data;
} grpc_stream_op;

/* A stream op buffer is a wrapper around stream operations that is dynamically
   extendable.
   TODO(ctiller): inline a few elements into the struct, to avoid common case
                  per-call allocations. */
typedef struct grpc_stream_op_buffer {
  grpc_stream_op *ops;
  size_t nops;
  size_t capacity;
  grpc_stream_op inlined_ops[GRPC_SOPB_INLINE_ELEMENTS];
} grpc_stream_op_buffer;

/* Initialize a stream op buffer */
void grpc_sopb_init(grpc_stream_op_buffer *sopb);
/* Destroy a stream op buffer */
void grpc_sopb_destroy(grpc_stream_op_buffer *sopb);
/* Reset a sopb to no elements */
void grpc_sopb_reset(grpc_stream_op_buffer *sopb);
/* Swap two sopbs */
void grpc_sopb_swap(grpc_stream_op_buffer *a, grpc_stream_op_buffer *b);

void grpc_stream_ops_unref_owned_objects(grpc_stream_op *ops, size_t nops);

/* Append a GRPC_NO_OP to a buffer */
void grpc_sopb_add_no_op(grpc_stream_op_buffer *sopb);
/* Append a GRPC_OP_BEGIN to a buffer */
void grpc_sopb_add_begin_message(grpc_stream_op_buffer *sopb, gpr_uint32 length,
                                 gpr_uint32 flags);
void grpc_sopb_add_metadata(grpc_stream_op_buffer *sopb,
                            grpc_metadata_batch metadata);
/* Append a GRPC_SLICE to a buffer - does not ref/unref the slice */
void grpc_sopb_add_slice(grpc_stream_op_buffer *sopb, gpr_slice slice);
/* Append a buffer to a buffer - does not ref/unref any internal objects */
void grpc_sopb_append(grpc_stream_op_buffer *sopb, grpc_stream_op *ops,
                      size_t nops);

char *grpc_sopb_string(grpc_stream_op_buffer *sopb);

#endif /* GRPC_INTERNAL_CORE_TRANSPORT_STREAM_OP_H */
