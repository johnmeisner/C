/**************************************************************************
 *                                                                        *
 *     File Name:  ansi_escapes.h                                         *
 *     ANSI ESCAPE SEQUENCES (ECMA-48 / ANSI X3.64 / ISO/IEC 6429)        *
 *     The ANSI Escape sequences can be used on just about any modern     *
 *     terminal or terminal emulator (putty,xterm,...) to clear the       *
 *     screen, control the cursor position, or highlight/color the        *
 *     output.                                                            *
 *                                                                        *
 **************************************************************************/
#ifndef TERM_ESCAPES
#define TERM_ESCAPES

//---Define Basic display highlight characteristics.
#define ESCRESET         "\033[0m"              /* Reset all graphics properties to normal */
#define ESCBOLD          "\033[1m"              /* Bold */
#define ESCULINE         "\033[4m"              /* Underline */
#define ESCBLINK         "\033[5m"              /* Blink */
#define ESCREVERSE       "\033[7m"              /* Reverse Video */

//---Define color escape sequences.
#define ESCBLACK         "\033[30m"             /* Black */ 
#define ESCRED           "\033[31m"             /* Red */ 
#define ESCGREEN         "\033[32m"             /* Green */ 
#define ESCYELLOW        "\033[33m"             /* Yellow */ 
#define ESCBLUE          "\033[34m"             /* Blue */ 
#define ESCMAGENTA       "\033[35m"             /* Magenta */ 
#define ESCCYAN          "\033[36m"             /* Cyan */ 
#define ESCWHITE         "\033[37m"             /* White */ 
#define ESCBOLDBLACK     "\033[1m\033[30m"      /* Bold Black */ 
#define ESCBOLDRED       "\033[1m\033[31m"      /* Bold Red */ 
#define ESCBOLDGREEN     "\033[1m\033[32m"      /* Bold Green */ 
#define ESCBOLDYELLOW    "\033[1m\033[33m"      /* Bold Yellow */ 
#define ESCBOLDBLUE      "\033[1m\033[34m"      /* Bold Blue */ 
#define ESCBOLDMAGENTA   "\033[1m\033[35m"      /* Bold Magenta */ 
#define ESCBOLDCYAN      "\033[1m\033[36m"      /* Bold Cyan */
#define ESCBOLDWHITE     "\033[1m\033[37m"      /* Bold White */

#define ESCRVBOLDBLACK   "\033[7m\033[1m\033[30m"      /* ReverseVideo Bold Black */ 
#define ESCRVBOLDRED     "\033[7m\033[1m\033[31m"      /* ReverseVideo Bold Red */ 
#define ESCRVBOLDGREEN   "\033[7m\033[1m\033[32m"      /* ReverseVideo Bold Green */ 
#define ESCRVBOLDYELLOW  "\033[7m\033[1m\033[33m"      /* ReverseVideo Bold Yellow */ 
#define ESCRVBOLDBLUE    "\033[7m\033[1m\033[34m"      /* ReverseVideo Bold Blue */ 
#define ESCRVBOLDMAGENTA "\033[7m\033[1m\033[35m"      /* ReverseVideo Bold Magenta */ 
#define ESCRVBOLDCYAN    "\033[7m\033[1m\033[36m"      /* ReverseVideo Bold Cyan */ 
#define ESCRVBOLDWHITE   "\033[7m\033[1m\033[37m"      /* ReverseVideo Bold White */ 

//---Define Cursor Movement and screen clearing escape sequences.
#define ESCCLEARSCREEN "\033[2J"     /* Clear Screen */
#define ESCCLEAR2END   "\033[0J"     /* Clear Screen from cursor down */
#define ESCCLEAR2TOP   "\033[1J"     /* Clear Screen from cursor up */

#define ESCCURSORHOME  "\033[H"      /*Move cursor to upper left corner*/
#define ESCCURSORROW2  "\033[2;1H"
#define ESCCURSORUP    "\033[1A"     /* Move cursor up 1 row */
#define ESCCURSORDOWN  "\033[1B"     /* Move cursor up 1 row */
#define ESCCURSORRIGHT "\033[1C"     /* Move cursor up 1 row */
#define ESCCURSORLEFT  "\033[1D"     /* Move cursor up 1 row */

//  ESCGOTORC -- Move cursor to specified row,column
//  ESCGOTORC(row,col) usage:    printf("%s",ESCGOTO(10,1));
#define ESCGOTORC(row,col) ESCGOTO_HIDDEN(row,col)
#define ESCGOTO_HIDDEN(row,col)  "\033[" #row ";" #col "H"

//Example:  printf("%s%sText1%s%sText2\n",ESCREVERSE,ESCRED,ESCRESET,ESCGOTO(10,1));



#ifdef DEFINE_DPRINTF
#define dprintf(fmt, args...)      fprintf(stderr, fmt, ##args)
#define dprintfred(fmt, args...)   {fprintf(stderr,"%s",ESCBOLDRED);fprintf(stderr, fmt, ##args);fprintf(stderr,"%s",ESCRESET);fflush(stderr);}
#else
#define dprintf(fmt, args...)
#define dprintfred(fmt, args...)
#endif

#endif  /*TERM_ESCAPES*/

