/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
/*
 * The C library's definition of struct termios might differ from
 * the kernel one, and we need to use the kernel layout.
 */
#include <linux/termios.h>

#include "xlat/tcxonc_options.h"
#include "xlat/tcflsh_options.h"
#include "xlat/baud_options.h"
#include "xlat/modem_flags.h"

struct flag_desc {
	const char *name;
	unsigned long flag;
};

static void decode_flags(const char *flag_name, unsigned long flags,
			 const struct flag_desc *fdesc, int fdesc_size)
{
	unsigned long mask = 0;
	bool first = true;
	int i;

	tprintf(" %s={", flag_name);

	for (i = 0; i < fdesc_size; i++, fdesc++) {
		if (mask) {
			if (!(fdesc->flag & ~mask)) {
				if ((flags & mask) == fdesc->flag)
					tprintf("%s", fdesc->name);
				continue;
			} else {
				mask = 0;
			}
		}

		if (!(flags & fdesc->flag))
			continue;

		if (__builtin_popcountl(fdesc->flag) > 1) {
			tprintf("%s%s:", first ? "" : ",", fdesc->name);
			mask = fdesc->flag;
			first = false;
			continue;
		}

		tprintf("%s%s", first ? "" : ",", fdesc->name);
		first = false;
	}

	tprintf("}");
}

static void
decode_termios(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct termios tios;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &tios))
		return;
	if (abbrev(tcp)) {
		struct flag_desc i_flags[] = {
			{ "IGNBRK", 0000001, },
			{ "BRKINT", 0000002, },
			{ "IGNPAR", 0000004, },
			{ "PARMRK", 0000010, },
			{ "INPCK", 0000020, },
			{ "ISTRIP", 0000040, },
			{ "INLCR", 0000100, },
			{ "IGNCR", 0000200, },
			{ "ICRNL", 0000400, },
			{ "IUCLC", 0001000, },
			{ "IXON", 0002000, },
			{ "IXANY", 0004000, },
			{ "IXOFF", 0010000, },
			{ "IMAXBEL", 0020000, },
			{ "IUTF8", 0040000, },
		};
		struct flag_desc o_flags[] = {
			{ "OPOST", 0000001, },
			{ "OLCUC", 0000002, },
			{ "ONLCR", 0000004, },
			{ "OCRNL", 0000010, },
			{ "ONOCR", 0000020, },
			{ "ONLRET", 0000040, },
			{ "OFILL", 0000100, },
			{ "OFDEL", 0000200, },
			{ "NLDLY", 0000400, },
			{   "NL0", 0000000, },
			{   "NL1", 0000400, },
			{ "CRDLY", 0003000, },
			{   "CR0", 0000000, },
			{   "CR1", 0001000, },
			{   "CR2", 0002000, },
			{   "CR3", 0003000, },
			{ "TABDLY", 0014000, },
			{   "TAB0", 0000000, },
			{   "TAB1", 0004000, },
			{   "TAB2", 0010000, },
			{   "TAB3", 0014000, },
			{   "XTABS", 0014000, },
			{ "BSDLY", 0020000, },
			{   "BS0", 0000000, },
			{   "BS1", 0020000, },
			{ "VTDLY", 0040000, },
			{   "VT0", 0000000, },
			{   "VT1", 0040000, },
			{ "FFDLY", 0100000, },
			{   "FF0", 0000000, },
			{   "FF1", 0100000, },
		};
		struct flag_desc c_flags[] = {
			{ "CBAUDEX", 0010000, },
			{ "CBAUD", 0010017, },
			{   "B0", 0000000, },		/* hang up", */
			{   "B50", 0000001, },
			{   "B75", 0000002, },
			{   "B110", 0000003, },
			{   "B134", 0000004, },
			{   "B150", 0000005, },
			{   "B200", 0000006, },
			{   "B300", 0000007, },
			{   "B600", 0000010, },
			{   "B1200", 0000011, },
			{   "B1800", 0000012, },
			{   "B2400", 0000013, },
			{   "B4800", 0000014, },
			{   "B9600", 0000015, },
			{   "B19200", 0000016, },
			{   "B38400", 0000017, },
			{   "BOTHER", 0010000, },
			{   "B57600", 0010001, },
			{   "B115200", 0010002, },
			{   "B230400", 0010003, },
			{   "B460800", 0010004, },
			{   "B500000", 0010005, },
			{   "B576000", 0010006, },
			{   "B921600", 0010007, },
			{   "B1000000", 0010010, },
			{   "B1152000", 0010011, },
			{   "B1500000", 0010012, },
			{   "B2000000", 0010013, },
			{   "B2500000", 0010014, },
			{   "B3000000", 0010015, },
			{   "B3500000", 0010016, },
			{   "B4000000", 0010017, },
			{ "CSIZE", 0000060, },
			{   "CS5", 0000000, },
			{   "CS6", 0000020, },
			{   "CS7", 0000040, },
			{   "CS8", 0000060, },
			{ "CSTOPB", 0000100, },
			{ "CREAD", 0000200, },
			{ "PARENB", 0000400, },
			{ "PARODD", 0001000, },
			{ "HUPCL", 0002000, },
			{ "CLOCAL", 0004000, },
			// { "CIBAUD", 002003600000, },	/* input baud rate */
			{ "CMSPAR", 010000000000, },	/* mark or space (stick) parity */
			{ "CRTSCTS", 020000000000, },	/* flow control */
		};
		struct flag_desc l_flags[] = {
			{ "ISIG", 0000001, },
			{ "ICANON", 0000002, },
			{ "XCASE", 0000004, },
			{ "ECHO", 0000010, },
			{ "ECHOE", 0000020, },
			{ "ECHOK", 0000040, },
			{ "ECHONL", 0000100, },
			{ "NOFLSH", 0000200, },
			{ "TOSTOP", 0000400, },
			{ "ECHOCTL", 0001000, },
			{ "ECHOPRT", 0002000, },
			{ "ECHOKE", 0004000, },
			{ "FLUSHO", 0010000, },
			{ "PENDIN", 0040000, },
			{ "IEXTEN", 0100000, },
			{ "EXTPROC", 0200000, },
		};

		tprints("{");
		printxval(baud_options, tios.c_cflag & CBAUD, "B???");

		decode_flags("c_iflag", tios.c_iflag, i_flags, ARRAY_SIZE(i_flags));
		decode_flags("c_oflag", tios.c_oflag, o_flags, ARRAY_SIZE(o_flags));
		decode_flags("c_cflag", tios.c_cflag, c_flags, ARRAY_SIZE(c_flags));
		decode_flags("c_lflag", tios.c_lflag, l_flags, ARRAY_SIZE(l_flags));

		tprints(" }");

		return;
	}
	tprintf("{c_iflags=%#lx, c_oflags=%#lx, ",
		(long) tios.c_iflag, (long) tios.c_oflag);
	tprintf("c_cflags=%#lx, c_lflags=%#lx, ",
		(long) tios.c_cflag, (long) tios.c_lflag);
	tprintf("c_line=%u, ", tios.c_line);
	if (!(tios.c_lflag & ICANON))
		tprintf("c_cc[VMIN]=%d, c_cc[VTIME]=%d, ",
			tios.c_cc[VMIN], tios.c_cc[VTIME]);
	tprints("c_cc=");
	print_quoted_string((char *) tios.c_cc, NCCS, QUOTE_FORCE_HEX);
	tprints("}");
}

static void
decode_termio(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct termio tio;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &tio))
		return;
	if (abbrev(tcp)) {
		tprints("{");
		printxval(baud_options, tio.c_cflag & CBAUD, "B???");
		tprintf(" %sopost %sisig %sicanon %secho ...}",
			(tio.c_oflag & OPOST) ? "" : "-",
			(tio.c_lflag & ISIG) ? "" : "-",
			(tio.c_lflag & ICANON) ? "" : "-",
			(tio.c_lflag & ECHO) ? "" : "-");
		return;
	}
	tprintf("{c_iflags=%#lx, c_oflags=%#lx, ",
		(long) tio.c_iflag, (long) tio.c_oflag);
	tprintf("c_cflags=%#lx, c_lflags=%#lx, ",
		(long) tio.c_cflag, (long) tio.c_lflag);
	tprintf("c_line=%u, ", tio.c_line);
#ifdef _VMIN
	if (!(tio.c_lflag & ICANON))
		tprintf("c_cc[_VMIN]=%d, c_cc[_VTIME]=%d, ",
			tio.c_cc[_VMIN], tio.c_cc[_VTIME]);
#else /* !_VMIN */
	if (!(tio.c_lflag & ICANON))
		tprintf("c_cc[VMIN]=%d, c_cc[VTIME]=%d, ",
			tio.c_cc[VMIN], tio.c_cc[VTIME]);
#endif /* !_VMIN */
	tprints("c_cc=\"");
	for (int i = 0; i < NCC; ++i)
		tprintf("\\x%02x", tio.c_cc[i]);
	tprints("\"}");
}

static void
decode_winsize(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct winsize ws;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &ws))
		return;
	tprintf("{ws_row=%d, ws_col=%d, ws_xpixel=%d, ws_ypixel=%d}",
		ws.ws_row, ws.ws_col, ws.ws_xpixel, ws.ws_ypixel);
}

#ifdef TIOCGSIZE
static void
decode_ttysize(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct ttysize ts;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &ts))
		return;
	tprintf("{ts_lines=%d, ts_cols=%d}",
		ts.ts_lines, ts.ts_cols);
}
#endif

static void
decode_modem_flags(struct tcb *const tcp, const kernel_ulong_t addr)
{
	int i;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &i))
		return;
	tprints("[");
	printflags(modem_flags, i, "TIOCM_???");
	tprints("]");
}

int
term_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	switch (code) {
	/* struct termios */
	case TCGETS:
#ifdef TCGETS2
	case TCGETS2:
#endif
	case TIOCGLCKTRMIOS:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TCSETS:
#ifdef TCSETS2
	case TCSETS2:
#endif
	case TCSETSW:
#ifdef TCSETSW2
	case TCSETSW2:
#endif
	case TCSETSF:
#ifdef TCSETSF2
	case TCSETSF2:
#endif
	case TIOCSLCKTRMIOS:
		decode_termios(tcp, arg);
		break;

	/* struct termio */
	case TCGETA:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TCSETA:
	case TCSETAW:
	case TCSETAF:
		decode_termio(tcp, arg);
		break;

	/* struct winsize */
	case TIOCGWINSZ:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TIOCSWINSZ:
		decode_winsize(tcp, arg);
		break;

	/* struct ttysize */
#ifdef TIOCGSIZE
	case TIOCGSIZE:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TIOCSSIZE:
		decode_ttysize(tcp, arg);
		break;
#endif

	/* ioctls with a direct decodable arg */
	case TCXONC:
		tprints(", ");
		printxval64(tcxonc_options, arg, "TC???");
		break;
	case TCFLSH:
		tprints(", ");
		printxval64(tcflsh_options, arg, "TC???");
		break;
	case TCSBRK:
	case TCSBRKP:
	case TIOCSCTTY:
		tprintf(", %d", (int) arg);
		break;

	/* ioctls with an indirect parameter displayed as modem flags */
	case TIOCMGET:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TIOCMBIS:
	case TIOCMBIC:
	case TIOCMSET:
		decode_modem_flags(tcp, arg);
		break;

	/* ioctls with an indirect parameter displayed in decimal */
	case TIOCGPGRP:
	case TIOCGSID:
	case TIOCGETD:
	case TIOCGSOFTCAR:
	case TIOCGPTN:
	case FIONREAD:
	case TIOCOUTQ:
#ifdef TIOCGEXCL
	case TIOCGEXCL:
#endif
#ifdef TIOCGDEV
	case TIOCGDEV:
#endif
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TIOCSPGRP:
	case TIOCSETD:
	case FIONBIO:
	case FIOASYNC:
	case TIOCPKT:
	case TIOCSSOFTCAR:
	case TIOCSPTLCK:
		tprints(", ");
		printnum_int(tcp, arg, "%d");
		break;

	/* ioctls with an indirect parameter displayed as a char */
	case TIOCSTI:
		tprints(", ");
		printstrn(tcp, arg, 1);
		break;

	/* ioctls with no parameters */

	case TIOCSBRK:
	case TIOCCBRK:
	case TIOCCONS:
	case TIOCNOTTY:
	case TIOCEXCL:
	case TIOCNXCL:
	case FIOCLEX:
	case FIONCLEX:
#ifdef TIOCVHANGUP
	case TIOCVHANGUP:
#endif
#ifdef TIOCSSERIAL
	case TIOCSSERIAL:
#endif
		break;

	/* ioctls which are unknown */

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
