/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
 *
 * This software is furnished under a license and may be used and copied
 * only in accordance with the terms of such license and with the
 * inclusion of the above copyright notice. This software or any other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of the software is hereby
 * transferred.
 *
 * The information in this software is subject to change without notice
 * and should not be construed as a commitment by Objective Systems, Inc.
 *
 * PROPRIETARY NOTICE
 *
 * This software is an unpublished work subject to a confidentiality agreement
 * and is protected by copyright and trade secret law.  Unauthorized copying,
 * redistribution or other use of this work is prohibited.
 *
 * The above notice of copyright on this source code product does not indicate
 * any actual or intended publication of such source code.
 *
 *****************************************************************************/

#include <string.h>
#include "rtxsrc/rtxMemBuf.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxmlsrc/rtLx2Dom.h"

#ifndef DTD_SUPPORT
#define DTD_SUPPORT              1
#endif

#ifndef OUT_ENTITY_REF_CONTENT
#define OUT_ENTITY_REF_CONTENT   0
#endif

#ifdef _DEBUG
static void append (OSRTMEMBUF* pMemBuf, const char* str, size_t len) {
   rtxMemBufAppend (pMemBuf, (const OSOCTET*)str, len);
}

static void appendStr (OSRTMEMBUF* pMemBuf, const char* str) {
   rtxMemBufAppend (pMemBuf, (const OSOCTET*)str, OSCRTLSTRLEN(str));
}

static void appendChar (OSRTMEMBUF* pMemBuf, char c) {
   rtxMemBufAppend (pMemBuf, (const OSOCTET*)&c, 1);
}

#define DECL_C
#else
#define append(pMemBuf, str, len) \
        rtxMemBufAppend (pMemBuf, (const OSOCTET*)str, len)

#define appendStr(pMemBuf, str) \
        rtxMemBufAppend (pMemBuf, (const OSOCTET*)str, OSCRTLSTRLEN(str))

#define appendChar(pMemBuf, c1) \
        c = c1, rtxMemBufAppend (pMemBuf, (const OSOCTET*)&c, 1)

#define DECL_C char c
#endif

static void lx2SerializeAttrValue (const char* value, OSRTMEMBUF* pMemBuf)
{
   const char* s = strchr(value,'\"');
   const char* s1;
   DECL_C;

   if (s) {
      s1 = strchr(value,'\'');
      if (s1) {
         s1 = value;
         appendChar (pMemBuf, '\"');

         while (s) {
            append (pMemBuf, s1, s - s1);
            append (pMemBuf, "&quot;", 6);
            s1 = s + 1;
            s = strchr(s + 1,'\"');
         }

         appendStr (pMemBuf, s1);
         appendChar (pMemBuf, '\"');
      }
      else {
         appendChar (pMemBuf, '\'');
         appendStr (pMemBuf, value);
         appendChar (pMemBuf, '\'');
      }
   }
   else {
      appendChar (pMemBuf, '\"');
      appendStr (pMemBuf, value);
      appendChar (pMemBuf, '\"');
   }
}

#if DTD_SUPPORT

static const char* const ocur[] = { "", "", "?", "*", "+" };

static void lx2SerializeElementContent
(xmlElementContentPtr cont, OSRTMEMBUF* pMemBuf)
{
   DECL_C;

   if (cont->type == XML_ELEMENT_CONTENT_PCDATA)
      append (pMemBuf, "#PCDATA", 7);
   else if (cont->type == XML_ELEMENT_CONTENT_ELEMENT) {
      if (cont->prefix) {
         appendStr (pMemBuf, (const char*)cont->prefix);
         appendChar (pMemBuf, ':');
         appendStr (pMemBuf, (const char*)cont->name);
      }
      else
         appendStr (pMemBuf, (const char*)cont->name);
   }
   else if (cont->c1 && cont->c2){
      appendChar (pMemBuf, '(');
      lx2SerializeElementContent (cont->c1, pMemBuf);

      if (cont->type == XML_ELEMENT_CONTENT_SEQ)
         appendChar (pMemBuf, ',');
      else
         appendChar (pMemBuf, '|');

      lx2SerializeElementContent (cont->c2, pMemBuf);
      appendChar (pMemBuf, ')');
   }

   append (pMemBuf, ocur[cont->ocur], 1);
}

static const char* const attrName[] = {
   "", "CDATA", "ID", "IDREF", "IDREFS", "ENTITY", "ENTITIES",
   "NMTOKEN", "NMTOKENS", "", "NOTATION"
};

static const char* const attrDef[] = {
   "", "", "#REQUIRED", "#IMPLIED", "#FIXED"
};

#endif

int lx2DomSerializeToMem (xmlDocPtr doc, xmlNodePtr node, OSRTMEMBUF* pMemBuf)
{
   xmlNodePtr cur = node;
   xmlAttrPtr attr;
   int outContent = OUT_ENTITY_REF_CONTENT;
   DECL_C;

   if (!node || !pMemBuf)
      return RTERR_INVPARAM;

   do {
      /* print current node */
      switch (cur->type) {
      case XML_ELEMENT_NODE:
         append (pMemBuf, "<", 1);
         appendStr (pMemBuf, (const char*)cur->name);

         for (attr = cur->properties; attr; attr = attr->next) {
            if(attr->ns) {
               appendChar (pMemBuf, ' ');
               appendStr (pMemBuf, (const char*)attr->ns->prefix);
               appendChar (pMemBuf, ':');
               appendStr (pMemBuf, (const char*)attr->name);
               appendChar (pMemBuf, '=');
            }
            else {
               appendChar (pMemBuf, ' ');
               appendStr (pMemBuf, (const char*)attr->name);
               appendChar (pMemBuf, '=');
            }

            if (attr->children)
               lx2SerializeAttrValue ((const char*)attr->children->content, pMemBuf);
         }
         break;
      case XML_TEXT_NODE:
         {
            const char* s = (const char*)cur->content;
            const char* s1 = s;

            while (*s) {
               while (*s && *s != '&' && *s != '<') s++;

               append (pMemBuf, s1, s - s1);

               if (*s) {
                  if(*s == '&')
                     append (pMemBuf, "&amp;", 5);
                  else if(*s == '<')
                     append (pMemBuf, "&lt;", 4);

                  s++;
                  s1 = s;
               }
            }
         }
         break;
      case XML_CDATA_SECTION_NODE:
         append (pMemBuf, "<![CDATA[", 9);
         appendStr (pMemBuf, (const char*)cur->content);
         append (pMemBuf, "]]>", 3);
         break;
      case XML_ENTITY_REF_NODE:
         if (outContent)
            appendStr (pMemBuf, (const char*)cur->content);
         else {
            appendChar (pMemBuf, '&');
            appendStr (pMemBuf, (const char*)cur->name);
            appendChar (pMemBuf, ';');
         }
         break;
      case XML_PI_NODE:
         append (pMemBuf, "<?", 2);
         appendStr (pMemBuf, (const char*)cur->name);
         appendChar (pMemBuf, ' ');
         appendStr (pMemBuf, (const char*)cur->content);
         append (pMemBuf, "?>", 2);
         break;
      case XML_COMMENT_NODE:
         append (pMemBuf, "<!--", 4);
         appendStr (pMemBuf, (const char*)cur->content);
         append (pMemBuf, "-->", 3);
         break;
      case XML_DOCUMENT_NODE:
         {
            doc = (xmlDocPtr)cur;
            append (pMemBuf, "<?xml", 5);
            if (doc->version) {
               append (pMemBuf, " version=\"", 10);
               appendStr (pMemBuf, (const char*)doc->version);
               appendChar (pMemBuf, '\"');
            }

            if (doc->encoding) {
               append (pMemBuf, " encoding=\"", 11);
               appendStr (pMemBuf, (const char*)doc->encoding);
               appendChar (pMemBuf, '\"');
            }

            append (pMemBuf, " standalone='", 13);
            if (doc->standalone)
               append (pMemBuf, "yes'?>\n", 7);
            else
               append (pMemBuf, "no'?>\n", 6);
         }
         break;

#if DTD_SUPPORT
      case XML_DTD_NODE:
         {
            xmlDtdPtr dtd = (xmlDtdPtr)cur;
            append (pMemBuf, "<!DOCTYPE ", 10);
            appendStr (pMemBuf, (const char*)dtd->name);

            if (dtd->ExternalID) {
               append (pMemBuf, " PUBLIC ", 8);
               appendStr (pMemBuf, (const char*)dtd->ExternalID);
               appendChar (pMemBuf, ' ');
               appendStr (pMemBuf, (const char*)dtd->SystemID);
            }
            else if (dtd->SystemID) {
               append (pMemBuf, " SYSTEM ", 8);
               appendStr (pMemBuf, (const char*)dtd->SystemID);
            }
         }
         break;
      case XML_ENTITY_DECL:
         {
            xmlEntityPtr ent = (xmlEntityPtr)cur;
            append (pMemBuf, "  <!ENTITY ", 11);

            if (ent->etype == XML_INTERNAL_PARAMETER_ENTITY ||
                ent->etype == XML_EXTERNAL_PARAMETER_ENTITY)
               {
                  append (pMemBuf, "% ", 2);
               }

            appendStr (pMemBuf, (const char*)ent->name);

            if (ent->ExternalID) {
               append (pMemBuf, " PUBLIC ", 8);
               appendStr (pMemBuf, (const char*)ent->ExternalID);
               appendChar (pMemBuf, ' ');
               appendStr (pMemBuf, (const char*)ent->SystemID);
            }
            else if (ent->SystemID) {
               append (pMemBuf, " SYSTEM ", 8);
               appendStr (pMemBuf, (const char*)ent->SystemID);
            }
            else  {
               appendChar (pMemBuf, ' ');
               lx2SerializeAttrValue ((const char*)ent->orig, pMemBuf);
            }

            if((ent->ExternalID || ent->SystemID) && ent->content) {
               append (pMemBuf, " NDATA ", 7);
               appendStr (pMemBuf, (const char*)ent->content);
            }

            append (pMemBuf, ">\n", 2);
         }
         break;
      case XML_ELEMENT_DECL:
         {
            xmlElementPtr elem = (xmlElementPtr)cur;
            xmlElementContentPtr cont = elem->content;

            append (pMemBuf, "  <!ELEMENT ", 12);

            if (elem->prefix) {
               appendStr (pMemBuf, (const char*)elem->prefix);
               appendChar (pMemBuf, ':');
               appendStr (pMemBuf, (const char*)elem->name);
            }
            else
               appendStr (pMemBuf, (const char*)elem->name);

            if (elem->etype == XML_ELEMENT_TYPE_ANY)
               append (pMemBuf, " ANY>\n", 6);
            else if (elem->etype == XML_ELEMENT_TYPE_EMPTY)
               append (pMemBuf, " EMPTY>\n", 8);
            else if (elem->etype == XML_ELEMENT_TYPE_MIXED ||
                     elem->etype == XML_ELEMENT_TYPE_ELEMENT)
               {
                  appendChar (pMemBuf, ' ');
                  lx2SerializeElementContent (cont, pMemBuf);
                  append (pMemBuf, " >\n", 3);
               }

         }
         break;
      case XML_ATTRIBUTE_DECL:
         {
            xmlAttributePtr attr = (xmlAttributePtr)cur;
            xmlEnumerationPtr enums = attr->tree;

            append (pMemBuf, "  <!ATTLIST ", 12);
            appendStr (pMemBuf, (const char*)attr->elem);
            appendChar (pMemBuf, ' ');

            if (attr->prefix) {
               appendStr (pMemBuf, (const char*)attr->prefix);
               appendChar (pMemBuf, ':');
               appendStr (pMemBuf, (const char*)attr->name);
            }
            else
               appendStr (pMemBuf, (const char*)attr->name);

            appendChar (pMemBuf, ' ');
            appendStr (pMemBuf, attrName[attr->atype]);

            if(enums) {
               append (pMemBuf, " (", 2);
               while (enums) {
                  appendStr (pMemBuf, (const char*)enums->name);

                  if (enums->next)
                     appendChar (pMemBuf, '|');
                  enums = enums->next;
               }

               appendChar (pMemBuf, ')');
            }

            if(attr->def == XML_ATTRIBUTE_FIXED)
               append (pMemBuf, " #FIXED", 7);

            if(attr->def == XML_ATTRIBUTE_NONE ||
               attr->def == XML_ATTRIBUTE_FIXED)
               {
                  if (attr->defaultValue) {
                     appendChar (pMemBuf, ' ');
                     lx2SerializeAttrValue ((const char*)attr->defaultValue,
                                             pMemBuf);
                  }
               }
            else if(attr->def <= XML_ATTRIBUTE_IMPLIED) {
               appendChar (pMemBuf, ' ');
               appendStr (pMemBuf, attrDef[attr->def]);
            }

            append (pMemBuf, ">\n", 2);
         }
         break;
#else
      case XML_DTD_NODE:
         break;
#endif

      default:
         break;
      }

      /* step to next node */
      if (cur->children && cur->type == XML_ELEMENT_NODE) {
         appendChar (pMemBuf, '>');
         cur = cur->children;
      }
      else if(cur->children && cur->type == XML_DOCUMENT_NODE) {
         cur = cur->children;
      }
#ifdef DTD_SUPPORT
      else if(cur->children && cur->type == XML_DTD_NODE) {
         append (pMemBuf, " [\n", 3);
         cur = cur->children;
      }
#endif /* DTD_SUPPORT */
      else if(cur->next) {
         if (cur->type == XML_ELEMENT_NODE)
            append (pMemBuf, "/>", 2);
#if DTD_SUPPORT
         else if (cur->type == XML_DTD_NODE)
            append (pMemBuf, ">\n", 2);
#endif
         if (cur != node)
            cur = cur->next;
      }
      else {
         while (cur != node) {
            cur = cur->parent;

            if (cur->type == XML_ELEMENT_NODE) {
               append (pMemBuf, "</", 2);
               appendStr (pMemBuf, (const char*)cur->name);
               appendChar (pMemBuf, '>');
            }
            else if (cur->type == XML_DTD_NODE)
               append (pMemBuf, "]>\n", 3);

            if (cur != node && cur->next) {
               cur = cur->next;
               break;
            }
         }
      }
   } while (cur != node);

   return 0;
}
