/*
This include file simply includes Qt headers of interest, and disables
annoying warnings that stem from these headers.
*/
#if _MSC_VER > 1500
/* I don't know whether versions prior to VS 2008 had push/pop */
   #pragma warning( push )
   #pragma warning (disable: 4127)
#elif defined __GNUC__
   #pragma GCC system_header
#endif

#include <QStringList>
#include <QLinkedList>
#include <QList>
#include <QVector>
#include <QVarLengthArray>


#if _MSC_VER > 1500
   #pragma warning( pop )
#endif
