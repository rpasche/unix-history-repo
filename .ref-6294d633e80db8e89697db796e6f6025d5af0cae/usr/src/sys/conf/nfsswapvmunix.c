/*
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rick Macklem at The University of Guelph.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)nfsswapvmunix.c	7.2 (Berkeley) %G%
 */

/*
 * Sample NFS swapvmunix configuration file.
 * This should be filled in by the bootstrap program.
 * See /sys/nfs/nfsdiskless.h for details of the fields.
 */

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/socket.h>
#include <sys/mount.h>

#include <net/if.h>
#include <nfs/nfsv2.h>
#include <nfs/nfsdiskless.h>

extern int nfs_mountroot();
int (*mountroot)() = nfs_mountroot;

dev_t	rootdev = NODEV;
dev_t	argdev  = NODEV;
dev_t	dumpdev = NODEV;

struct	swdevt swdevt[] = {
	{ NODEV,	0,	5000 },	/* happy:/u/swap.dopey  */
	{ 0, 0, 0 }
};
struct nfs_diskless nfs_diskless = {
	{ { 'q', 'e', '0', '\0' },
	  { 0x10, 0x2, { 0x0, 0x0, 0x83, 0x68, 0x30, 0x2, } },
	  { 0x10, 0x2, { 0x0, 0x0, 0x83, 0x68, 0x30, 0xff, } },
	  { 0x10, 0x0, { 0x0, 0x0, 0xff, 0xff, 0xff, 0x0, } },
 	},
	{ 0x10, 0x2, { 0x0, 0x0, 0x83, 0x68, 0x30, 0x12, } },
	{
	  (struct sockaddr *)0, SOCK_DGRAM, 0, (nfsv2fh_t *)0,
	  0, 8192, 8192, 10, 100, (char *)0,
	},
	{
		0xf,
		0x9,
		0x0,
		0x0,
		0x1,
		0x0,
		0x0,
		0x0,
		0xc,
		0x0,
		0x0,
		0x0,
		0x6,
		0x0,
		0x0,
		0x0,
		0x27,
		0x18,
		0x79,
		0x27,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
	},
	{ 0x10, 0x2, { 0x8, 0x1, 0x83, 0x68, 0x30, 0x5, } },
	"happy",
	{
	  (struct sockaddr *)0, SOCK_DGRAM, 0, (nfsv2fh_t *)0,
	  0, 8192, 8192, 10, 100, (char *)0,
	},
	{
		0x0,
		0x9,
		0x0,
		0x0,
		0x1,
		0x0,
		0x0,
		0x0,
		0xc,
		0x0,
		0x0,
		0x0,
		0x2,
		0x0,
		0x0,
		0x0,
		0xd0,
		0x48,
		0x42,
		0x25,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
	},
	{ 0x10, 0x2, { 0x8, 0x1, 0x83, 0x68, 0x30, 0x5, } },
	"happy",
};
