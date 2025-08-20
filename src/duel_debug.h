#ifndef _DUEL_DEBUG_H_
#define _DUEL_DEBUG_H_

#undef PDEBUG
#ifdef DUEL_DEBUG
#	ifdef __KERNEL__
#		define PDEBUG(fmt, args...) printk(KERN_DEBUG fmt, ## args)
#	else
#		define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#	endif
#else
#	define PDEBUG(fmt, args...)
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...)

#endif