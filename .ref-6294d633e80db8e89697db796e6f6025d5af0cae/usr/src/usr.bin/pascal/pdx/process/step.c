/*-
 * Copyright (c) 1980 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)step.c	5.3 (Berkeley) %G%";
#endif /* not lint */

/*
 * Continue execution up to the next source line.
 *
 * We call "nextaddr" from the machine module to figure out
 * what the object address is that corresponds to the next source line.
 * If nextaddr returns -1, then the end of the program has been reached.
 *
 * There are two ways to define the next source line depending on what
 * is desired when a procedure or function call is encountered.  Step
 * stops at the beginning of the procedure or call; next skips over it.
 */

#include "defs.h"
#include "process.h"
#include "machine.h"
#include "breakpoint.h"
#include "source.h"
#include "mappings.h"
#include "process.rep"

/*
 * Stepc is what is called when the step command is given.
 * It has to play with the "isstopped" information.
 */

stepc()
{
    if (!isstopped) {
	error("can't continue execution");
    }
    isstopped = FALSE;
    dostep(FALSE);
    isstopped = TRUE;
}

next()
{
    if (!isstopped) {
	error("can't continue execution");
    }
    isstopped = FALSE;
    dostep(TRUE);
    isstopped = TRUE;
}

step()
{
    dostep(FALSE);
}

/*
 * Resume execution up to the given address.  It is assumed that
 * no breakpoints exist between the current address and the one
 * we're stepping to.  This saves us from setting all the breakpoints.
 */

stepto(addr)
ADDRESS addr;
{
    setbp(addr);
    resume();
    unsetbp(addr);
    if (!isbperr()) {
	printstatus();
    }
}

LOCAL dostep(isnext)
BOOLEAN isnext;
{
    register ADDRESS addr;
    register LINENO line;

    addr = pc;
    do {
	addr = nextaddr(addr, isnext);
	line = linelookup(addr);
    } while (line == 0 && !ss_instructions);
    stepto(addr);
    curline = line;
}
