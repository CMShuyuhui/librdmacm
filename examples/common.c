/*
 * Copyright (c) 2005-2006,2012 Intel Corporation.  All rights reserved.
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
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <byteswap.h>

#include <rdma/rdma_cma.h>
#include "common.h"

int use_rs = 1;
/**
 * [get_rdma_addr 根据hints获取ip和端口]
 * hints 代表服务端，则 src 和port就是相应的ip和端口
 * hints是客户端，则 dst 和port就是对应的ip和端口
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-12-06T15:35:41+0800
 */
int get_rdma_addr(char *src, char *dst, char *port,
		  struct rdma_addrinfo *hints, struct rdma_addrinfo **rai)
{
	struct rdma_addrinfo rai_hints, *res;
	int ret;

	if (hints->ai_flags & RAI_PASSIVE)
		return rdma_getaddrinfo(src, port, hints, rai);

	rai_hints = *hints;
	if (src) {
		rai_hints.ai_flags |= RAI_PASSIVE;
		ret = rdma_getaddrinfo(src, NULL, &rai_hints, &res);
		if (ret)
			return ret;

		rai_hints.ai_src_addr = res->ai_src_addr;
		rai_hints.ai_src_len = res->ai_src_len;
		rai_hints.ai_flags &= ~RAI_PASSIVE;
	}

	ret = rdma_getaddrinfo(dst, port, &rai_hints, rai);
	if (src)
		rdma_freeaddrinfo(res);

	return ret;
}

/**
 * [size_str 根据size的大小，初始化str的表达方式,按照1024位单位]
 * size = 5.2 *1024*1024 *1024 ----> str = 5.2g
 * size = 5.2 *1024*1024 ----> str = 5.2m
 * size = 5.2 *1024 ----> str = 5.2k
 * size = 1000 ----> str = 1000
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-12-06T14:58:40+0800
 */
void size_str(char *str, size_t ssize, long long size)
{
	long long base, fraction = 0;
	char mag;

	if (size >= (1 << 30)) {
		base = 1 << 30;
		mag = 'g';
	} else if (size >= (1 << 20)) {
		base = 1 << 20;
		mag = 'm';
	} else if (size >= (1 << 10)) {
		base = 1 << 10;
		mag = 'k';
	} else {
		base = 1;
		mag = '\0';
	}

	if (size / base < 10)
		fraction = (size % base) * 10 / base;
	if (fraction) {
		snprintf(str, ssize, "%lld.%lld%c", size / base, fraction, mag);
	} else {
		snprintf(str, ssize, "%lld%c", size / base, mag);
	}
}
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
void cnt_str(char *str, size_t ssize, long long cnt)
{
	if (cnt >= 1000000000)
		snprintf(str, ssize, "%lldb", cnt / 1000000000);
	else if (cnt >= 1000000)
		snprintf(str, ssize, "%lldm", cnt / 1000000);
	else if (cnt >= 1000)
		snprintf(str, ssize, "%lldk", cnt / 1000);
	else
		snprintf(str, ssize, "%lld", cnt);
}

int size_to_count(int size)
{
	if (size >= (1 << 20))
		return 100;
	else if (size >= (1 << 16))
		return 1000;
	else if (size >= (1 << 10))
		return 10000;
	else
		return 100000;
}

/**
 * [format_buf 初始化buf，0到uint8_t递增赋值]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-12-06T15:09:48+0800
 */
void format_buf(void *buf, int size)
{
	uint8_t *array = buf;
	static uint8_t data;
	int i;

	for (i = 0; i < size; i++)
		array[i] = data++;
}
/**
 * [verify_buf 核实buf，根据format_buf赋值后，该buf中的数据就应该是跟规则一致的，判断是否发生了更改]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-12-06T15:11:27+0800
 * @return 0表示未发生更改， -1 表示发生了更改，打印输出
 */
int verify_buf(void *buf, int size)
{
	static long long total_bytes;
	uint8_t *array = buf;
	static uint8_t data;
	int i;

	for (i = 0; i < size; i++, total_bytes++) {
		if (array[i] != data++) {
			printf("data verification failed byte %lld\n", total_bytes);
			return -1;
		}
	}
	return 0;
}
/**
 * [do_poll poll监听fd事件]
 * 统一接入平台API
 * @AuthorHTL 胡宇辉
 * @DateTime  2016-12-06T15:15:20+0800
 * @param     fds                      [socket fd]
 * @param     timeout                  [超时时间，INFTIM 永远等待；0 立即返回，不阻塞进程； >0 等待指定数目的毫秒数  ]
 * @return                             [-1 表示出错， ]
 */
int do_poll(struct pollfd *fds, int timeout)
{
	int ret;
	//  返回：就绪描述字的个数,此处是1 ，0超时，-1出错
	do {
		ret = rs_poll(fds, 1, timeout);
	} while (!ret);
	// POLLERR	发生错误
	// POLLHUP	发生挂起
	// struct pollfd {
	// int fd;        /* 文件描述符 */
	// short events;  /*等待的事件 */
	// short revents; /* 实际发生了的事件 */
	// };
	return ret == 1 ? (fds->revents & (POLLERR | POLLHUP)) : ret;
}
