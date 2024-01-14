#include "rtxersrc/ASN1SAX_XEROpenType.h"
#include "rtxersrc/xee_common.hh"
#include "rtxmlsrc/rtSaxCParser.h"
#include "rtsrc/rtBCD.h"

static OSBOOL isBinaryExt;

int asn1Sax_XEROpenType_startElement
   (void *userData, const OSUTF8CHAR* localname, 
    const OSUTF8CHAR* qname, const OSUTF8CHAR* const* atts)
{
   ASN1SAX_XEROpenType* pSaxHandler = (ASN1SAX_XEROpenType*) userData;
   ASN1SAXCDecodeHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSCTXT* pctxt = pSaxBase->mpCtxt;
   int stat = 0;

   isBinaryExt = ! OSCRTLSTRNCMP ((char *)localname, "binext", 6);

   XERCDIAGSTRM4 (pctxt, "asn1Sax_XEROpenType_startElement: '%s' start (%d)\n",
              STRX(pctxt, localname), pSaxBase->mLevel);

   pSaxHandler->mEncCtxt.state = XERSTART;
   pSaxBase->mState = XERSTART;

   /* Encode start element */

   if (! isBinaryExt) {
      stat = xerCopyText (&pSaxHandler->mEncCtxt, "<");  
      if (stat != 0) { return LOG_RTERR (&pSaxHandler->mEncCtxt, stat); }

      stat = xerEncXmlCharData (&pSaxHandler->mEncCtxt, localname, 
                                xerTextLength (localname)); 

      if (stat != 0) { return LOG_RTERR (&pSaxHandler->mEncCtxt, stat); }

      /* TODO: need to add attributes if present */

      stat = xerCopyText (&pSaxHandler->mEncCtxt, ">");
      if (stat != 0) { return LOG_RTERR (&pSaxHandler->mEncCtxt, stat); }
   }
   
   /* Bump level */

   pSaxBase->mLevel++;

   XERCDIAGSTRM4 (pctxt, "asn1Sax_XEROpenType_startElement: '%s' end (%d)\n",
                  STRX(pctxt, localname), pSaxBase->mLevel);

   return 0;
}

int asn1Sax_XEROpenType_characters
   (void *userData, const OSUTF8CHAR* chars, int length)
{
   ASN1SAX_XEROpenType* pSaxHandler = (ASN1SAX_XEROpenType*) userData;
   ASN1SAXCDecodeHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSCTXT* pctxt = pSaxBase->mpCtxt;
   int stat;

   XERCDIAGSTRM2 (pctxt, "asn1Sax_XEROpenType_characters: start\n");
   RTHEXDUMPSTRM (pctxt,(OSOCTET*)chars, (length*2));

   if (! isBinaryExt) {
      stat = xerEncXmlCharData (&pSaxHandler->mEncCtxt, chars, length);
   }
   else { /* we have a binary extension element */
      OSOCTET *bcd = (OSOCTET *) OSCRTLMALLOC (length);
      int len = rtStringToBCD ((const char*)chars, bcd, length, FALSE);
      
      stat = xerEncXmlCharData (&pSaxHandler->mEncCtxt, bcd, len);
   }

   if (stat != 0) { return LOG_RTERR (&pSaxHandler->mEncCtxt, stat); }

   pSaxBase->mState = XERDATA;
   XERCDIAGSTRM2 (pctxt, "asn1Sax_XEROpenType_characters: end\n");

   return 0;
}

/* Check encode buffer for empty element.  This method will modify the 
 * buffer contents if element is found to be empty by replacing last 
 * '>' character with '/>'. 
 */
static OSBOOL asn1Sax_XEROpenType_isEmptyElement 
   (ASN1SAX_XEROpenType* pSaxHandler, const OSUTF8CHAR* qname)
{
   char* bufp;

   pSaxHandler->mEncCtxt.buffer.byteIndex--;
   bufp = (char*) ASN1BUFPTR ((&pSaxHandler->mEncCtxt));
   if (*bufp == '>') { /* no character data */
      *bufp-- = '\0';  /* null out last '>' character */
      while (*bufp != '<') {
         bufp--; 
      }
      bufp++;
      if (xerCmpText (qname, bufp)) {
         /* empty element - append null element indicator */
         xerCopyText (&pSaxHandler->mEncCtxt, "/>");
         return TRUE;
      }
      else
         xerCopyText (&pSaxHandler->mEncCtxt, ">"); /* put back original '>' */
   }
   else
      pSaxHandler->mEncCtxt.buffer.byteIndex++;    /* restore byte index */

   return FALSE;
}

int asn1Sax_XEROpenType_endElement
   (void *userData, const OSUTF8CHAR* localname, const OSUTF8CHAR* qname)
{
   ASN1SAX_XEROpenType* pSaxHandler = (ASN1SAX_XEROpenType*) userData;
   ASN1OpenType* pMsgData = pSaxHandler->mpMsgData;
   ASN1SAXCDecodeHandlerBase* pSaxBase = &pSaxHandler->mSaxBase;
   OSCTXT* pctxt = pSaxBase->mpCtxt;
   int stat = 0;
   const OSOCTET* data;

   XERCDIAGSTRM4 (pctxt, "asn1Sax_XEROpenType_endElement: '%s' start (%d)\n",
              STRX(pctxt, localname), pSaxBase->mLevel);

   pSaxHandler->mEncCtxt.state = XERSTART;

   if (!ISCOMPLETE (pSaxHandler)) {
      /* Encode end element */
      if (! isBinaryExt) {
         if (!asn1Sax_XEROpenType_isEmptyElement (pSaxHandler, localname)) {
            stat = xerCopyText (&pSaxHandler->mEncCtxt, "</");
            if (stat != 0) { return LOG_RTERR (&pSaxHandler->mEncCtxt, stat); }

            stat = xerEncXmlCharData (&pSaxHandler->mEncCtxt, 
                                      localname, xerTextLength (localname));

            if (stat != 0) { return LOG_RTERR (&pSaxHandler->mEncCtxt, stat); }

            stat = xerCopyText (&pSaxHandler->mEncCtxt, ">");
            if (stat != 0) { return LOG_RTERR (&pSaxHandler->mEncCtxt, stat); }
         }
      }

      /* Update mem link and open type data variable */

      data = pSaxHandler->mEncCtxt.buffer.data;

      pMsgData->data = data;
      pMsgData->numocts = (OSUINT32)pSaxHandler->mEncCtxt.buffer.byteIndex;

      /* If last element, set state to XEREND */

      if (--pSaxBase->mLevel == 0) pSaxBase->mState = XEREND;
   }

   XERCDIAGSTRM4 (pctxt, "asn1Sax_XEROpenType_endElement: '%s' end (%d)\n",
                  STRX(pctxt, localname), pSaxBase->mLevel);

   return 0;
}

void asn1Sax_XEROpenType_init
  (OSCTXT* pctxt, ASN1SAX_XEROpenType* pSaxHandler,
   ASN1OpenType* pvalue, OSINT16 level)
{
   int stat;

   pSaxHandler->mpMsgData = 0;

   if ((stat = rtInitSubContext (&pSaxHandler->mEncCtxt, pctxt)) != 0)
      { LOG_RTERR (&pSaxHandler->mEncCtxt, stat); return; }

   /* rtxCtxtSetMsgMemHeap (&pSaxHandler->mEncCtxt, pctxt); */
   xerSetEncBufPtr (&pSaxHandler->mEncCtxt, 0, 1024, TRUE);

   OSCRTLMEMSET(pvalue, 0, sizeof(ASN1OpenType)); 

   pSaxHandler->mpMsgData = pvalue;
   pSaxHandler->mSaxBase.mpStartElement = asn1Sax_XEROpenType_startElement;
   pSaxHandler->mSaxBase.mpEndElement   = asn1Sax_XEROpenType_endElement;
   pSaxHandler->mSaxBase.mpCharacters   = asn1Sax_XEROpenType_characters;
   pSaxHandler->mSaxBase.mpCtxt = pctxt;
   pSaxHandler->mSaxBase.mLevel = pSaxHandler->mSaxBase.mStartLevel = level;
}

void asn1Sax_XEROpenType_free
  (OSCTXT* pctxt, ASN1SAX_XEROpenType* pSaxHandler)
{
   if (pSaxHandler->mpMsgData != 0) {
      pSaxHandler->mEncCtxt.buffer.data = 0; /* to prevent from freeing */
      pSaxHandler->mEncCtxt.buffer.size = 0;
      rtFreeContext (&pSaxHandler->mEncCtxt);
   }
}
