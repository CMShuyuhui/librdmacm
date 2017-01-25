/*
 * Copyright (c) 2005-2012 Intel Corporation.  All rights reserved.
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
 *
 * $Id$
 */

#include <stdlib.h>
#include <sys/types.h>
#include <byteswap.h>
#include <poll.h>

#include <rdma/rdma_cma.h>
#include <rdma/rsocket.h>
#include <infiniband/ib.h>

#if __BYTE_ORDER == __BIG_ENDIAN
static inline uint64_t cpu_to_be64(uint64_t x) { return x; }
static inline uint32_t cpu_to_be32(uint32_t x) { return x; }
#else
static inline uint64_t cpu_to_be64(uint64_t x) { return bswap_64(x); }
static inline uint32_t cpu_to_be32(uint32_t x) { return bswap_32(x); }
#endif

// use_rs 非0 表示使用rdma的socket方式， 0 表示 常规TCP/IP的socket方式
extern int use_rs;

#define rs_socket(f,t,p)  use_rs ? rsocket(f,t,p)  : socket(f,t,p)
#define rs_bind(s,a,l)    use_rs ? rbind(s,a,l)    : bind(s,a,l)
#define rs_listen(s,b)    use_rs ? rlisten(s,b)    : listen(s,b)
#define rs_connect(s,a,l) use_rs ? rconnect(s,a,l) : connect(s,a,l)
#define rs_accept(s,a,l)  use_rs ? raccept(s,a,l)  : accept(s,a,l)
#define rs_shutdown(s,h)  use_rs ? rshutdown(s,h)  : shutdown(s,h)
#define rs_close(s)       use_rs ? rclose(s)       : close(s)
#define rs_recv(s,b,l,f)  use_rs ? rrecv(s,b,l,f)  : recv(s,b,l,f)
#define rs_send(s,b,l,f)  use_rs ? rsend(s,b,l,f)  : send(s,b,l,f)
#define rs_recvfrom(s,b,l,f,a,al) 	use_rs ? rrecvfrom(s,b,l,f,a,al) : recvfrom(s,b,l,f,a,al)
#define rs_sendto(s,b,l,f,a,al) 	use_rs ? rsendto(s,b,l,f,a,al)   : sendto(s,b,l,f,a,al)
#define rs_poll(f,n,t)	  use_rs ? rpoll(f,n,t)	   : poll(f,n,t)
#define rs_fcntl(s,c,p)   use_rs ? rfcntl(s,c,p)   : fcntl(s,c,p)
#define rs_setsockopt(s,l,n,v,ol) 	use_rs ? rsetsockopt(s,l,n,v,ol) : setsockopt(s,l,n,v,ol)
#define rs_getsockopt(s,l,n,v,ol) 	use_rs ? rgetsockopt(s,l,n,v,ol) : getsockopt(s,l,n,v,ol)

union socket_addr {
	struct sockaddr		sa;
	struct sockaddr_in	sin;
	struct sockaddr_in6	sin6;
};

enum rs_optimization {
	opt_mixed,//混合模式
	opt_latency,//延时
	opt_bandwidth//带宽
};
/**
 * [get_rdma_addr 根据hints获取ip和端口]
 * hints 代表服务端，则 src 和port就是相应的ip和端口
 * hints是客户端，则 dst 和port就是对应的ip和端口
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-12-06T15:35:41+0800
 */
int get_rdma_addr(char *src, char *dst, char *port,
		  struct rdma_addrinfo *hints, struct rdma_addrinfo **rai);
/**
 * [size_str 根据size的大小，初始化str的表达方式]
 * size = 5.2 *1024*1024 *1024 ----> str = 5.2g
 * size = 5.2 *1024*1024 ----> str = 5.2m
 * size = 5.2 *1024 ----> str = 5.2k
 * size = 1000 ----> str = 1000
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-12-06T14:58:40+0800
 */
void size_str(char *str, size_t ssize, long long size);

/**
 * [cnt_str 根据cnt大小，初始化str的表达方式，按照1000位单位]
 * cnt = 500   ----> str = 500
 * cnt = 500 * 1000   ----> str = 500k
 * cnt = 500 * 1000 * 1000  ----> str = 500m
 * cnt = 500 * 1000 * 1000 * 1000   ----> str = 500b
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-12-06T15:02:42+0800
 */
void cnt_str(char *str, size_t ssize, long long cnt);
int size_to_count(int size);
/**
 * [format_buf 初始化buf，0到uint8_t递增赋值]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-12-06T15:09:48+0800
 */
void format_buf(void *buf, int size);
/**
 * [verify_buf 核实buf，根据format_buf赋值后，该buf中的数据就应该是跟规则一致的，判断是否发生了更改]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-12-06T15:11:27+0800
 * @return 0表示未发生更改， -1 表示发生了更改，打印输出
 */
int verify_buf(void *buf, int size);
/**
 * [do_poll poll监听fd事件]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-12-06T15:15:20+0800
 * @param     fds                      [socket fd]
 * @param     timeout                  [超时时间，INFTIM 永远等待；0 立即返回，不阻塞进程； >0 等待指定数目的毫秒数  ]
 * @return                             [-1 表示出错， ]
 */
int do_poll(struct pollfd *fds, int timeout);
