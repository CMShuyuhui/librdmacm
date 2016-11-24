/*
 * Copyright (c) 2010-2014 Intel Corporation.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#if !defined(RDMA_VERBS_H)
#define RDMA_VERBS_H

#include <assert.h>
#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int rdma_seterrno(int ret)
{
	if (ret) {
		errno = ret;
		ret = -1;
	}
	return ret;
}

/*
 * Shared receive queues.
 */
/**
 * [rdma_create_srq allocates a shared request queue associated with the rdma_cm_id, id. The id
 * 	must be bound to a local RMDA device before calling this routine. If the protection domain, pd, is
 * 	provided, it must be for that same device. After being allocated, the SRQ will be ready to handle
 * 	posting of receives. If a pd is NULL, then the rdma_cm_id will be created using a default protection domain. 
 * 	One default protection domain is allocated per RDMA device. The initial SRQ attributes are specified by the attr parameter.
 * 	If a completion queue, CQ, is not specified for the XRC SRQ, then a CQ will be allocated by the
 * 	rdma_cm for the SRQ, along with corresponding completion channels. Completion channels and
 * 	CQ data created by the rdma_cm are exposed to the user through the rdma_cm_id structure. The
 * 	actual capabilities and properties of the created SRQ will be returned to the user through the attr parameter.
 * 	An rdma_cm_id may only be associated with a single SRQ.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:28:41+0800
 * @param     id                       [The RDMA communication identifier]
 * @param     pd                       [Optional protection domain for the shared request queue(SRQ)]
 * @param     attr                     [Initial SRQ attributes]
 * @return                             [0 on success, -1 on error. If the call fails, errno will be set to indicate the reason for the failure.]
 */
int rdma_create_srq(struct rdma_cm_id *id, struct ibv_pd *pd,
		    struct ibv_srq_init_attr *attr);
int rdma_create_srq_ex(struct rdma_cm_id *id, struct ibv_srq_init_attr_ex *attr);

/**
 * [rdma_destroy_srq destroys an SRQ allocated on the rdma_cm_id, id. Any SRQ associated with
 * 	an rdma_cm_id must be destroyed before destroying the rdma_cm_id, id.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:30:22+0800
 * @param     id                       [The RDMA communication identifier whose associated SRQ we wish to destroy.]
 */
void rdma_destroy_srq(struct rdma_cm_id *id);


/*
 * Memory registration helpers.
 */

/**
 * [rdma_reg_msgs rdma_reg_msgs registers an array of memory buffers for sending or receiving messages or for
 *	RDMA operations. The registered memory buffers may then be posted to an rdma_cm_id using
 *	rdma_post_send or rdma_post_recv. They may also be specified as the target of an RDMA read
 *	operation or the source of an RDMA write request.
 *	The memory buffers are registered with the protection domain associated with the rdma_cm_id.
 *	The start of the data buffer array is specified through the addr parameter and the total size of the
 *	array is given by the length.
 *	All data buffers must be registered before being posted as a work request. They must be deregistered by calling rdma_dereg_mr.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:12:50+0800
 * @param     id                       [a reference to the communication identifier where the message buffer(s) will be used]
 * @param     addr                     [the address of the memory buffer(s) to register]
 * @param     length                   [the total length of the memory to register]
 * @return                             [A reference to the registered memory region on success or NULL on failure]
 */
static inline struct ibv_mr* rdma_reg_msgs(struct rdma_cm_id *id, void *addr, size_t length)
{
	return ibv_reg_mr(id->pd, addr, length, IBV_ACCESS_LOCAL_WRITE);
}
/**
 * [rdma_reg_read  Registers a memory buffer that will be accessed by a remote RDMA read operation.
 *  Memory buffers registered using rdma_reg_read may be targeted in an RDMA read request,
 *  allowing the buffer to be specified on the remote side of an RDMA connection as the remote_addr
 *  of rdma_post_read, or similar call.
 *  rdma_reg_read is used to register a data buffer that will be the target of an RDMA read operation
 *  on a queue pair associated with an rdma_cm_id. The memory buffer is registered with the protection 
 *  domain associated with the identifier. The start of the data buffer is specified through the addr
 *  parameter, and the total size of the buffer is given by length.
 *  All data buffers must be registered before being posted as work requests. Users must deregister all
 *  registered memory by calling the rdma_dereg_mr.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:13:13+0800
 * @param     id                       [a referenct to the communication identifier where the message buffer(s) will be used]
 * @param     addr                     [the address of the memory buffer(s) to register]
 * @param     length                   [the total length of the memory to register]
 * @return                             [A reference to the registered memory region on success or NULL on failure. If an
 *                                        error occurs, errno will be set to indicate the failure reason.]
 */
static inline struct ibv_mr* rdma_reg_read(struct rdma_cm_id *id, void *addr, size_t length)
{
	return ibv_reg_mr(id->pd, addr, length, IBV_ACCESS_LOCAL_WRITE |
						IBV_ACCESS_REMOTE_READ);
}

/**
 * [rdma_reg_write registers a memory buffer which will be accessed by a remote RDMA write
 * 	operation. Memory buffers registered using this routine may be targeted in an RDMA write
 *  request, allowing the buffer to be specified on the remote side of an RDMA connection as the
 *  remote_addr of an rdma_post_write or similar call.
 *  The memory buffer is registered with the protection domain associated with the rdma_cm_id. 
 *  Thestart of the data buffer is specified through the addr parameter, and the total size of the buffer is
 * 	given by the length.
 *  All data buffers must be registered before being posted as work requests. Users must deregister all
 * 	registered memory by calling the rdma_dereg_mr.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:23:42+0800
 * @param     id                       [A reference to the communication identifier where the
 *                                     	message buffer(s) will be used]
 * @param     addr                     [The address of the memory buffer(s) to register]
 * @param     length                   [The total length of the memory to register]
 * @return                             [A reference to the registered memory region on success or NULL on failure. If an
 *                                        error occurs, errno will be set to indicate the failure reason.]
 */
static inline struct ibv_mr* rdma_reg_write(struct rdma_cm_id *id, void *addr, size_t length)
{
	return ibv_reg_mr(id->pd, addr, length, IBV_ACCESS_LOCAL_WRITE |
						IBV_ACCESS_REMOTE_WRITE);
}

/**
 * [rdma_dereg_mr deregisters a memory buffer which has been registered for RDMA or message 
 * 	operations. This routine must be called for all registered memory associated with a given
 *  rdma_cm_id before destroying the rdma_cm_id.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:27:24+0800
 * @param     mr                       [A reference to a registered memory buffer]
 * @return                             [0 on success, -1 on error. If the call fails, errno will be set to indicate the reason for the failure.]
 */
static inline int rdma_dereg_mr(struct ibv_mr *mr)
{
	return rdma_seterrno(ibv_dereg_mr(mr));
}


/*
 * Vectored send, receive, and RDMA operations.
 * Support multiple scatter-gather entries.
 */
/**
 * [rdma_post_recvv posts a single work request to the receive queue of the queue pair associated
 * 	with the rdma_cm_id, id. The posted buffers will be queued to receive an incoming message sent by the remote peer.
 * 	Please note that this routine supports multiple scatter-gather entries. The user is responsible for
 * 	ensuring that the receive is posted, and the total buffer space is large enough to contain all sent data
 * 	before the peer posts the corresponding send message. 
 * 	The message buffers must have been registered before being posted, and the buffers must remain registered until the receive completes.
 * 	Messages may be posted to an rdma_cm_id only after a queue pair has been associated with it. 
 * 	A queue pair is bound to an rdma_cm_id after calling rdma_create_ep or rdma_create_qp, 
 * 	if the rdma_cm_id is allocated using rdma_create_id.
 * 	The user-defined context associated with the receive request will be returned to the user 
 * 	through the work completion work request identifier (wr_id) field.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:31:30+0800
 * @param     id                       [A reference to the communication identifier where the message buffer(s) will be posted]
 * @param     context                  [A user-defined context associated with the request]
 * @param     sgl                      [A scatter-gather list of memory buffers posted as a single request]
 * @param     nsge                     [The number of scatter-gather entries in the sgl array]
 * @return                             [0 on success, -1 on error. If the call fails, errno will be set to indicate the reason for the failure.]
 */
static inline int rdma_post_recvv(struct rdma_cm_id *id, void *context, struct ibv_sge *sgl,
		int nsge)
{
	struct ibv_recv_wr wr, *bad;

	wr.wr_id = (uintptr_t) context;
	wr.next = NULL;
	wr.sg_list = sgl;
	wr.num_sge = nsge;

	if (id->srq)
		return rdma_seterrno(ibv_post_srq_recv(id->srq, &wr, &bad));
	else
		return rdma_seterrno(ibv_post_recv(id->qp, &wr, &bad));
}

/**
 * [rdma_post_sendv posts a work request to the send queue of the queue pair associated with the rdma_cm_id, id.
 *  The contents of the posted buffers will be sent to the remote peer of the connection.
 *  The user is responsible for ensuring that the remote peer has queued a receive request before issuing the send operations. 
 *  Also, unless the send request is using inline data, the message buffers 
 *  must already have been registered before being posted. The buffers must remain registered until the send completes.
 *  This routine supports multiple scatter-gather entries.
 *  Send operations may not be posted to an rdma_cm_id or the corresponding queue pair until a connection has been established.
 *  The user-defined context associated with the send request will be returned to the user through the
 *  work completion work request identifier (wr_id) field.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:33:39+0800
 * @param     id                       [A reference to the communication identifier where the message buffer will be posted]
 * @param     context                  [A user-defined context associated with the request]
 * @param     sgl                      [A scatter-gather list of memory buffers posted as a single request]
 * @param     nsge                     [The number of scatter-gather entries in the sgl array]
 * @param     flags                    [Optional flags used to control the send operation]
 * @return                             [0 on success, -1 on error. If the call fails, errno will be set to indicate the reason for the failure.]
 */
static inline int rdma_post_sendv(struct rdma_cm_id *id, void *context, struct ibv_sge *sgl,
		int nsge, int flags)
{
	struct ibv_send_wr wr, *bad;

	wr.wr_id = (uintptr_t) context;
	wr.next = NULL;
	wr.sg_list = sgl;
	wr.num_sge = nsge;
	wr.opcode = IBV_WR_SEND;
	wr.send_flags = flags;

	return rdma_seterrno(ibv_post_send(id->qp, &wr, &bad));
}

/**
 * [rdma_post_readv posts a work request to the send queue of the queue pair associated with the rdma_cm_id, id.
 *  The contents of the remote memory region at remote_addr will be read into the local data buffers given in the sgl array.
 *  The user must ensure that both the remote and local data buffers have been registered before the read is issued.
 *  The buffers must remain registered until the read completes.
 *  Read operations may not be posted to an rdma_cm_id or the corresponding queue pair until a connection has been established.
 *  The user-defined context associated with the read request will be returned to the user 
 *  through the work completion work request identifier (wr_id) field.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:35:31+0800
 * @param     id                       [A reference to the communication identifier where the request will be posted]
 * @param     context                  [A user-defined context associated with the request]
 * @param     sgl                      [A scatter-gather list of the destination buffers of the read]
 * @param     nsge                     [The number of scatter-gather entries in the sgl array]
 * @param     flags                    [Optional flags used to control the read operation]
 * @param     remote_addr              [The address of the remote registered memory to read from]
 * @param     rkey                     [The registered memory key associated with the remote address]
 * @return                             [0 on success, -1 on error. If the call fails, errno will be set to indicate the reason for the failure.]
 */
static inline int rdma_post_readv(struct rdma_cm_id *id, void *context, struct ibv_sge *sgl,
		int nsge, int flags, uint64_t remote_addr, uint32_t rkey)
{
	struct ibv_send_wr wr, *bad;

	wr.wr_id = (uintptr_t) context;
	wr.next = NULL;
	wr.sg_list = sgl;
	wr.num_sge = nsge;
	wr.opcode = IBV_WR_RDMA_READ;
	wr.send_flags = flags;
	wr.wr.rdma.remote_addr = remote_addr;
	wr.wr.rdma.rkey = rkey;

	return rdma_seterrno(ibv_post_send(id->qp, &wr, &bad));
}

/**
 * [rdma_post_writev posts a work request to the send queue of the queue pair associated with the rdma_cm_id, id.
 *  The contents of the local data buffers in the sgl array will be written to the remote memory region at remote_addr.
 *  Unless inline data is specified, the local data buffers must have been registered before the write is issued,
 *  and the buffers must remain registered until the write completes. The remote buffers must always be registered.
 *  Write operations may not be posted to an rdma_cm_id or the corresponding queue pair until a connection has been established.
 *  The user-defined context associated with the write request will be returned to the user 
 *  through the work completion work request identifier (wr_id) field.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:37:54+0800
 * @param     id                       [A reference to the communication identifier where the request will be posted]
 * @param     context                  [A user-defined context associated with the request]
 * @param     sgl                      [A scatter-gather list of the source buffers of the write]
 * @param     nsge                     [The number of scatter-gather entries in the sgl array]
 * @param     flags                    [Optional flags used to control the write operation]
 * @param     remote_addr              [The address of the remote registered memory to write into]
 * @param     rkey                     [The registered memory key associated with the remote address]
 * @return                             [0 on success, -1 on error. If the call fails, errno will be set to indicate the reason for the failure.]
 */
static inline int rdma_post_writev(struct rdma_cm_id *id, void *context, struct ibv_sge *sgl,
		 int nsge, int flags, uint64_t remote_addr, uint32_t rkey)
{
	struct ibv_send_wr wr, *bad;

	wr.wr_id = (uintptr_t) context;
	wr.next = NULL;
	wr.sg_list = sgl;
	wr.num_sge = nsge;
	wr.opcode = IBV_WR_RDMA_WRITE;
	wr.send_flags = flags;
	wr.wr.rdma.remote_addr = remote_addr;
	wr.wr.rdma.rkey = rkey;

	return rdma_seterrno(ibv_post_send(id->qp, &wr, &bad));
}

/*
 * Simple send, receive, and RDMA calls.
 */
/**
 * [rdma_post_recv  posts a work request to the receive queue of the queue pair associated with the rdma_cm_id, id. 
 * 	The posted buffer will be queued to receive an incoming message sent by the remote peer. 
 * 	The user is responsible for ensuring that receive buffer is posted and is large enough to contain 
 * 	all sent data before the peer posts the corresponding send message. 
 * 	The buffer must have already been registered before being posted, with the mr parameter referencing the registration.
 * 	The buffer must remain registered until the receive completes.
 * 	Messages may be posted to an rdma_cm_id only after a queue pair has been associated with it. 
 * 	A queue pair is bound to an rdma_cm_id after calling rdma_create_ep or rdma_create_qp, 
 * 	if the rdma_cm_id is allocated using rdma_create_id.
 * 	The user-defined context associated with the receive request will be returned to the user 
 * 	through the work completion request identifier (wr_id) field.
 * 	
 * 	Please note that this is a simple receive call. There are no scatter-gather lists involved here.]
 * 	
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:40:32+0800
 * @param     id                       [A reference to the communication identifier where the message buffer will be posted]
 * @param     context                  [A user-defined context associated with the request]
 * @param     addr                     [The address of the memory buffer to post]
 * @param     length                   [The length of the memory buffer]
 * @param     mr                       [A registered memory region associated with the posted buffer]
 * @return                             [0 on success, -1 on error. If the call fails, errno will be set to indicate the reason for the failure.]
 */
static inline int rdma_post_recv(struct rdma_cm_id *id, void *context, void *addr,
	       size_t length, struct ibv_mr *mr)
{
	struct ibv_sge sge;

	assert((addr >= mr->addr) &&
		(((uint8_t *) addr + length) <= ((uint8_t *) mr->addr + mr->length)));
	sge.addr = (uint64_t) (uintptr_t) addr;
	sge.length = (uint32_t) length;
	sge.lkey = mr->lkey;

	return rdma_post_recvv(id, context, &sge, 1);
}

/**
 * [rdma_post_send posts a work request to the send queue of the queue pair associated with the rdma_cm_id, id. 
 * 	The contents of the posted buffer will be sent to the remote peer of the connection. 
 * 	The user is responsible for ensuring that the remote peer has queued a receive request before issuing the send operations. 
 * 	Also, unless the send request is using inline data, the message buffer must already have been registered before being posted,
 * 	with the mr parameter referencing the registration. The buffer must remain registered until the send completes.
 * 	Send operations may not be posted to an rdma_cm_id or the corresponding queue pair until a connection has been established.
 * 	The user-defined context associated with the send request will be returned to the user 
 * 	through the work completion work request identifier (wr_id) field.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:43:14+0800
 * @param     id                       [A reference to the communication identifier where the message buffer will be posted]
 * @param     context                  [A user-defined context associated with the request]
 * @param     addr                     [The address of the memory buffer to post]
 * @param     length                   [The length of the memory buffer]
 * @param     mr                       [Optional registered memory region associated with the posted buffer]
 * @param     flags                    [Optional flags used to control the send operation]
 * @return                             [0 on success, -1 on error. If the call fails, errno will be set to indicate the reason for the failure.]
 */
static inline int rdma_post_send(struct rdma_cm_id *id, void *context, void *addr,
	       size_t length, struct ibv_mr *mr, int flags)
{
	struct ibv_sge sge;

	sge.addr = (uint64_t) (uintptr_t) addr;
	sge.length = (uint32_t) length;
	sge.lkey = mr ? mr->lkey : 0;

	return rdma_post_sendv(id, context, &sge, 1, flags);
}

/**
 * [rdma_post_read posts a work request to the send queue of the queue pair associated with the rdma_cm_id. 
 * 	The contents of the remote memory region will be read into the local data buffer.
 * 	For a list of supported flags, see ibv_post_send. The user must ensure that both the remote and 
 * 	local data buffers must have been registered before the read is issued, and the buffers must remain 
 * 	registered until the read completes.
 * 	Read operations may not be posted to an rdma_cm_id or the corresponding queue pair until it has been connected.
 * 	The user-defined context associated with the read request will be returned to the user through the
 * 	work completion wr_id, work request identifier, field.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:45:16+0800
 * @param     id                       [A reference to the communication identifier where the request will be posted]
 * @param     context                  [A user-defined context associated with the request]
 * @param     addr                     [The address of the local destination of the read request]
 * @param     length                   [The length of the read operation]
 * @param     mr                       [Registered memory region associated with the local buffer]
 * @param     flags                    [Optional flags used to control the read operation]
 * @param     remote_addr              [The address of the remote registered memory to read from]
 * @param     rkey                     [The registered memory key associated with the remote address]
 * @return                             [0 on success, -1 on error. If the call fails, errno will be set to indicate the reason for the failure.]
 */
static inline int rdma_post_read(struct rdma_cm_id *id, void *context, void *addr,
	       size_t length, struct ibv_mr *mr, int flags,
	       uint64_t remote_addr, uint32_t rkey)
{
	struct ibv_sge sge;

	sge.addr = (uint64_t) (uintptr_t) addr;
	sge.length = (uint32_t) length;
	sge.lkey = mr->lkey;

	return rdma_post_readv(id, context, &sge, 1, flags, remote_addr, rkey);
}

/**
 * [rdma_post_write posts a work request to the send queue of the queue pair associated with the rdma_cm_id, id. 
 * 	The contents of the local data buffer will be written into the remote memory region.
 *  Unless inline data is specified, the local data buffer must have been registered before the write is issued, 
 *  and the buffer must remain registered until the write completes. The remote buffer must always be registered.
 *  Write operations may not be posted to an rdma_cm_id or the corresponding queue pair until a connection has been established.
 *  The user-defined context associated with the write request will be returned to the user through the
 *  work completion work request identifier (wr_id) field.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:46:59+0800
 * @param     id                       [A reference to the communication identifier where the request will be posted]
 * @param     context                  [A user-defined context associated with the request]
 * @param     addr                     [The local address of the source of the write request]
 * @param     length                   [The length of the write operation]
 * @param     mr                       [Optional registered memory region associated with the local buffer]
 * @param     flags                    [Optional flags used to control the write operation]
 * @param     remote_addr              [The address of the remote registered memory to write into]
 * @param     rkey                     [The registered memory key associated with the remote address]
 * @return                             [0 on success, -1 on error. If the call fails, errno will be set to indicate the reason for the failure.]
 */
static inline int rdma_post_write(struct rdma_cm_id *id, void *context, void *addr,
		size_t length, struct ibv_mr *mr, int flags,
		uint64_t remote_addr, uint32_t rkey)
{
	struct ibv_sge sge;

	sge.addr = (uint64_t) (uintptr_t) addr;
	sge.length = (uint32_t) length;
	sge.lkey = mr ? mr->lkey : 0;

	return rdma_post_writev(id, context, &sge, 1, flags, remote_addr, rkey);
}

/**
 * [rdma_post_ud_send posts a work request to the send queue of the queue pair associated with the rdma_cm_id, id. 
 * 	The contents of the posted buffer will be sent to the specified destination queue pair, remote_qpn.
 *  The user is responsible for ensuring that the destination queue pair has queued a receive request before issuing the send operations. 
 *  Unless the send request is using inline data, the message buffer must have been registered before being posted, 
 *  with the mr parameter referencing the registration. 
 *  The buffer must remain registered until the send completes.
 *  The user-defined context associated with the send request will be returned to the user 
 *  through the work completion work request identifier (wr_id) field.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:48:50+0800
 * @param     id                       [A reference to the communication identifier where the request will be posted]
 * @param     context                  [A user-defined context associated with the request]
 * @param     addr                     [The address of the memory buffer to post]
 * @param     length                   [The length of the memory buffer]
 * @param     mr                       [Optional registered memory region associated with the posted buffer]
 * @param     flags                    [Optional flags used to control the send operation]
 * @param     ah                       [An address handle describing the address of the remote node]
 * @param     remote_qpn               [The destination node's queue pair number]
 * @return                             [0 on success, -1 on error. If the call fails, errno will be set to indicate the reason for the failure.]
 */
static inline int rdma_post_ud_send(struct rdma_cm_id *id, void *context, void *addr,
		  size_t length, struct ibv_mr *mr, int flags,
		  struct ibv_ah *ah, uint32_t remote_qpn)
{
	struct ibv_send_wr wr, *bad;
	struct ibv_sge sge;

	sge.addr = (uint64_t) (uintptr_t) addr;
	sge.length = (uint32_t) length;
	sge.lkey = mr ? mr->lkey : 0;

	wr.wr_id = (uintptr_t) context;
	wr.next = NULL;
	wr.sg_list = &sge;
	wr.num_sge = 1;
	wr.opcode = IBV_WR_SEND;
	wr.send_flags = flags;
	wr.wr.ud.ah = ah;
	wr.wr.ud.remote_qpn = remote_qpn;
	wr.wr.ud.remote_qkey = RDMA_UDP_QKEY;

	return rdma_seterrno(ibv_post_send(id->qp, &wr, &bad));
}
/**
 * [rdma_get_send_comp retrieves a completed work request for a send, RDMA read or RDMA write operation. 
 * 	Information about the completed request is returned through the ibv_wc, wc parameter, 
 * 	with the wr_id set to the context of the request. 
 * 	Please see ibv_poll_cq for details on the work completion structure, ibv_wc.
 * 	Please note that this call polls the send completion queue associated with the rdma_cm_id, id. 
 * 	If a completion is not found, the call blocks until a request completes. This means, therefore, 
 * 	that the call should only be used on rdma_cm_ids which do not share CQs with other rdma_cm_ids, 
 * 	and maintain separate CQs for sends and receive completions.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:50:58+0800
 * @param     id                       [A reference to the communication identifier to check for completions]
 * @param     wc                       [A reference to a work completion structure to fill in]
 * @return                             [A non-negative value (0 or 1) equal to the number of completions found on success,
 *                                         or -1 on failure. If the call fails, errno will be set to indicate the reason for the failure.]
 */
static inline int rdma_get_send_comp(struct rdma_cm_id *id, struct ibv_wc *wc)
{
	struct ibv_cq *cq;
	void *context;
	int ret;

	do {
		ret = ibv_poll_cq(id->send_cq, 1, wc);
		if (ret)
			break;

		ret = ibv_req_notify_cq(id->send_cq, 0);
		if (ret)
			return rdma_seterrno(ret);

		ret = ibv_poll_cq(id->send_cq, 1, wc);
		if (ret)
			break;

		ret = ibv_get_cq_event(id->send_cq_channel, &cq, &context);
		if (ret)
			return ret;

		assert(cq == id->send_cq && context == id);
		ibv_ack_cq_events(id->send_cq, 1);
	} while (1);

	return (ret < 0) ? rdma_seterrno(ret) : ret;
}

/**
 * [rdma_get_recv_comp retrieves a completed work request a receive operation. 
 * 	Information about the completed request is returned through the ibv_wc, wc parameter, 
 * 	with the wr_id set to the context of the request. 
 * 	Please see ibv_poll_cq for details on the work completion structure, ibv_wc.
 * 	Please note that this call polls the receive completion queue associated with the rdma_cm_id, id. 
 * 	If a completion is not found, the call blocks until a request completes. This means, therefore, 
 * 	that the call should only be used on rdma_cm_ids which do not share CQs with other rdma_cm_ids, 
 * 	and maintain separate CQs for sends and receive completions.]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-11-24T13:53:27+0800
 * @param     id                       [A reference to the communication identifier to check for completions]
 * @param     wc                       [A reference to a work completion structure to fill in]
 * @return                             [A non-negative value equal to the number of completions found on success, or errno on failure]
 */
static inline int rdma_get_recv_comp(struct rdma_cm_id *id, struct ibv_wc *wc)
{
	struct ibv_cq *cq;
	void *context;
	int ret;

	do {
		ret = ibv_poll_cq(id->recv_cq, 1, wc);
		if (ret)
			break;

		ret = ibv_req_notify_cq(id->recv_cq, 0);
		if (ret)
			return rdma_seterrno(ret);

		ret = ibv_poll_cq(id->recv_cq, 1, wc);
		if (ret)
			break;

		ret = ibv_get_cq_event(id->recv_cq_channel, &cq, &context);
		if (ret)
			return ret;

		assert(cq == id->recv_cq && context == id);
		ibv_ack_cq_events(id->recv_cq, 1);
	} while (1);

	return (ret < 0) ? rdma_seterrno(ret) : ret;
}

#ifdef __cplusplus
}
#endif

#endif /* RDMA_CMA_H */
