/* $Id: sargasso.h,v 1.1 1996/06/16 17:27:59 tom Exp $ */
#ifndef SARGASSO_H
#define SARGASSO_H 1

/* These #ifdef:s are implementation dependent stuff for the Sargasso C */
/* Unix C barfs on directives like "#strings", so we keep them	        */
/* indented. Then unix c can't find them, but Sargasso C *can*.		*/
/* Admittedly kludgey, but it works...)					*/
#ifdef SARG10
  #define _UNIXCON  /* Make UNIX-flavored I/O on TOPS */
  #strings low      /* put strings in lowseg mem so we can modify them. */
#endif
#ifdef SARG20
  #define _UNIXCON  /* Make UNIX-flavored I/O on TOPS */
  #strings low      /* put strings in lowseg mem so we can modify them. */
  #include <TOPS20.HDR>
#endif

#endif /* SARGASSO_H */
