/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2002 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "portable.h"

#include "siglock.h"

using namespace std;

#if HAVE_SIGHUP
static void (*sig_hup)(int);
#endif
#if HAVE_SIGQUIT
static void (*sig_quit)(int);
#endif
static void (*sig_int)(int);
static void (*sig_term)(int);

static int sig_ignore_sig;

void sig_ignore(int sig)
{
	if (sig_ignore_sig == 0)
		sig_ignore_sig = sig;
}

void sig_lock()
{
	sig_ignore_sig = 0;
#if HAVE_SIGHUP
	sig_hup = signal(SIGHUP, sig_ignore);
#endif
#if HAVE_SIGQUIT
	sig_quit = signal(SIGQUIT, sig_ignore);
#endif
	sig_int = signal(SIGINT, sig_ignore);
	sig_term = signal(SIGTERM, sig_ignore);
}

void sig_unlock()
{
#if HAVE_SIGHUP
	signal(SIGHUP, sig_hup);
#endif
#if HAVE_SIGQUIT
	signal(SIGQUIT, sig_quit);
#endif
	signal(SIGINT, sig_int);
	signal(SIGTERM, sig_term);

	if (sig_ignore_sig)
		raise(sig_ignore_sig);
}

