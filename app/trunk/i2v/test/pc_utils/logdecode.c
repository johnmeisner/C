/**************************************************************************
 *                                                                        *
 *     File Name:  logdecode.c                                            *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "DSRC.h"
#include "rtxsrc/rtxMemLeakCheck.h"
#include "rtxsrc/rtxDiag.h"

#define STR_MAX 150
#define WSM_MAX_DATA 1400

#define DEFAULT_OUTPUT "decoded.txt"

/* create a human readible file listing contents of encoded J2735 message */

typedef enum {
    BFALSE,
    BTRUE
} BOOLEAN;

typedef enum {
    LD_MAP,
    LD_TIM,
    LD_SPAT,
} J2735_TYPES;

typedef enum {
    ASN1_DER,
    ASN1_PERA,
    ASN1_PERU,
} ASN_TYPES;

static char filename[STR_MAX + 1];
static char ofile[STR_MAX + 1];
static char path[STR_MAX + 1];
static char fstring[WSM_MAX_DATA];
static int  msgtype = -1;
static int  asndecodetype = -1;
static OSCTXT decodeCtxt;

static int readFile(FILE *f, char *buf)
{
    char stringbuf[WSM_MAX_DATA * 3 + 1];  /* by convention log messages stored in ascii space separated list */
    int i, size = 0;
    char *tok;

    size = fread(stringbuf, sizeof(char), (3 * WSM_MAX_DATA), f);
    tok = strtok(stringbuf, " ");
    for (i=0; i<WSM_MAX_DATA && (i*3) < size; i++) {
        if (NULL == tok) break;
        buf[i] = strtoul(tok, NULL, 16);
        tok = strtok(NULL, " ");
    }
    fclose(f);

    return i;
}

static void printApproach(FILE *f, Approach *val, const char *label)
{
    OSRTDListNode *node = NULL;
    int i, j;
    double dummy1, dummy2;
    VehicleReferenceLane *vl;
    VehicleComputedLane *cl;

    if (val->m.namePresent) {
        fprintf(f, "%s name: %s\n", label, val->name);
    }
    if (val->m.idPresent) {
        fprintf(f, "%s id: %d\n", label, val->id);
    }

    if (val->m.drivingLanesPresent) {
        node = val->drivingLanes.head;
        for (i=0; i<val->drivingLanes.count; i++) {
            if (NULL == node) {
                break;
            }
            vl = (VehicleReferenceLane *)node->data;
            fprintf(f, "\nVehicle reference lane %d:\n", (i+1));
            fprintf(f, "lane number: %d\n", vl->laneNumber);
            if (vl->m.laneWidthPresent)
                fprintf(f, "lane width: %d in centimeters\n", vl->laneWidth);
            fprintf(f, "lane attributes (see J2735 spec): 0x%.8X\n", vl->laneAttributes);
            fprintf(f, "lane node list (vals in cm):");
            for (j=0; j<vl->nodeList.n; j++) {
                /* offsets are 16 bits x first then y in network order */
                dummy1 = (vl->nodeList.elem[j].data[0] << 8) + vl->nodeList.elem[j].data[1];  /* X */
                dummy2 = (vl->nodeList.elem[j].data[2] << 8) + vl->nodeList.elem[j].data[3];  /* Y */
                fprintf(f, " X:%.2f Y:%.2f", dummy1, dummy2);
            }
            fprintf(f, "\n");
            node = node->next;
        }
    }

    if (val->m.computedLanesPresent) {
        node = val->computedLanes.head;
        for (i=0; i<val->computedLanes.count; i++) {
            if (NULL == node) {
                break;
            }
            cl = (VehicleComputedLane *)node->data;
            fprintf(f, "\nVehicle computed lane %d:\n", (i+1));
            fprintf(f, "lane number: %d\n", cl->laneNumber);
            if (cl->m.laneWidthPresent)
                fprintf(f, "lane width: %d in centimeters\n", cl->laneWidth);
            fprintf(f, "reference lane number %d\n", cl->refLaneNum);
            if (cl->m.laneAttributesPresent)
                fprintf(f, "lane attributes (see J2735 spec): 0x%.8X\n", cl->laneAttributes);
            fprintf(f, "line offset (in cm from middle of ref lane): %d\n", cl->lineOffset);
            node = node->next;
        }
    }
}

/* Not all J2735 map fields supported at the current time - primarily only
   adding support for types encoded by Denso software */
static BOOLEAN mapsave(FILE *f, MapData *map)
{
    int i, j;
    unsigned int udummy;
    OSRTDListNode *intnode = NULL, *lnode = NULL;
    Intersection *inp;
    ApproachObject *ap;

    fprintf(f, "Map Data Contents\n");
    fprintf(f, "=========================\n\n");
    fprintf(f, "DSRC msg ID: 0x%X\n", map->msgID);
    fprintf(f, "Msg Count: %d\n", map->msgCnt);
    if (map->m.namePresent) {
        fprintf(f, "Name: %s\n", map->name);
    }
    if (map->m.layerTypePresent) {
        switch (map->layerType) {
            case none:
                fprintf(f, "Layer type 'None'\n");
                break;
            case mixedContent:
                fprintf(f, "Layer type 'Mixed Content'\n");
                break;
            case intersectionData:
                fprintf(f, "Layer type 'Intersection Data'\n");
                break;
            case curveData:
                fprintf(f, "Layer type 'Curve Data'\n");
                break;
            case roadwaySectionData:
                fprintf(f, "Layer type 'Roadway Section Data'\n");
                break;
            case parkingAreaData:
                fprintf(f, "Layer type 'Parking Area Data'\n");
                break;
            case sharedLaneData:
                fprintf(f, "Layer type 'Shared Lane Data'\n");
                break;
            default:
                fprintf(f, "Unknown layer type: %d\n", map->layerType);
                break;
        }
    }
    if (map->m.layerIDPresent) {
        fprintf(f, "Layer ID: %d\n", map->layerID);
    }

    if (map->m.intersectionsPresent) {
        intnode = map->intersections.head;
        for (i=0; i<map->intersections.count; i++) {
            if (NULL == intnode) {
                break;
            }
            inp = (Intersection *)intnode->data;
            lnode = NULL;
            ap = NULL;
            fprintf(f, "\nIntersection %d data:\n", (i+1));
            fprintf(f, "-------------------------\n");
            if (inp->m.namePresent) {
                fprintf(f, "Intersection name: %s\n", inp->name);
            }
            fprintf(f, "Intersection ID: ");
            udummy = 0;
            for (j=0; j<inp->id.numocts; j++) udummy += inp->id.data[j] << (24 - (j * 8));  /* network order */
            fprintf(f, "%d\n", udummy);
            if (inp->m.refPointPresent) {
                fprintf(f, "Position information:\n");
                fprintf(f, "latitude: %.6f\n", (double)inp->refPoint.lat / 10000000);
                fprintf(f, "longitude: %.6f\n", (double)inp->refPoint.long_ / 10000000);
                if (inp->refPoint.m.elevationPresent) 
                    fprintf(f, "elevation: %d\n", inp->refPoint.elevation);
            }
            if (inp->m.refInterNumPresent) {
                fprintf(f, "Intersection reference intersection number: ");
                udummy = 0;
                for (j=0; j<inp->id.numocts; j++) udummy += inp->id.data[j] << (24 - (j * 8));  /* network order */
                fprintf(f, "%d\n", udummy);
            }
            if (inp->m.orientationPresent) {
                fprintf(f, "Heading orientation: %d\n", inp->orientation);
            }
            if (inp->m.laneWidthPresent) {
                fprintf(f, "lane width: %d in centimeters\n", inp->laneWidth);
            }
            if (inp->m.typePresent) {
                fprintf(f, "intersection status object: %d\n", inp->type.data[0]);  
            }
            lnode = inp->approaches.head;
            for (j=0; j<inp->approaches.count; j++) {
                if (NULL == lnode) {
                    break;
                }
                ap = (ApproachObject *)lnode->data;
                fprintf(f, "*****************************\n");
                fprintf(f, "Approach %d data:\n", (j+1));
                if (ap->m.refPointPresent) {
                    fprintf(f, "approach position information:\n");
                    fprintf(f, "latitude: %.6f\n", (double)ap->refPoint.lat / 10000000);
                    fprintf(f, "longitude: %.6f\n", (double)ap->refPoint.long_ / 10000000);
                    if (inp->refPoint.m.elevationPresent) 
                        fprintf(f, "elevation: %d\n", ap->refPoint.elevation);
                }
                if (ap->m.laneWidthPresent) {
                    fprintf(f, "lane width: %d in centimeters\n", ap->laneWidth);
                }
                if (ap->m.approachPresent) {
                    printApproach(f, &ap->approach, "approach");
                }
                if (ap->m.egressPresent) {
                    printApproach(f, &ap->egress, "egress");
                }
                lnode = lnode->next;
            } /* approaches.count */
            intnode = intnode->next;
        } /* intersections.count */
    }
    fclose(f);
    return BTRUE;
}

static BOOLEAN mapDecodeDER(FILE *f, char *data, int length)
{
    ASN1TAG tag;
    MapData map;    
    int dummy;

    asn1Init_MapData(&map);
    xd_setp(&decodeCtxt, data, length, &tag, &dummy);

    if (tag != TV_MapData) {
        printf("\n%s: invalid tag (%X)\n", __FUNCTION__, tag);
        return BFALSE;
    }

    if (asn1D_MapData(&decodeCtxt, &map, ASN1EXPL, 0) != 0) {
        printf("\n%s: failed asn1d map\n", __FUNCTION__);
        rtxErrPrint(&decodeCtxt);
        rtxErrReset(&decodeCtxt);
        return BFALSE;
    }

    return mapsave(f, &map);
}

static BOOLEAN mapDecodePER(FILE *f, char *data, int length, char aligned)
{
    MapData map;

    asn1Init_MapData(&map);
    pu_setBuffer(&decodeCtxt, data, length, aligned);

    if (asn1PD_MapData(&decodeCtxt, &map) != 0) {
        rtxErrPrint(&decodeCtxt);
        rtxErrReset(&decodeCtxt);
        return BFALSE;
    }

    return mapsave(f, &map);
}

/* see comment above about map data support - same applies to tim */
static BOOLEAN timsave(FILE *f, TravelerInformation *tim)
{
    int i, j, k;
    unsigned int udummy;
    double dummy1, dummy2;
    OSRTDListNode *frame = NULL, *list = NULL;
    TravelerInformation_dataFrames_element *elem = NULL;
    ValidRegion *vr = NULL;
    ITIScodesAndText_element *adv;
    WorkZone_element *pwz;
    GenericSignage_element *pgs;
    SpeedLimit_element *psl;
    ExitService_element *pes;

    fprintf(f, "Tim Data Contents\n");
    fprintf(f, "=========================\n\n");
    fprintf(f, "DSRC msg ID: 0x%X\n", tim->msgID);
    if (tim->m.packetIDPresent) {
        fprintf(f, "Packet ID:");
        for (i=0; i<tim->packetID.numocts; i++) {
            fprintf(f, " %.2X", tim->packetID.data[i]);
        }
        fprintf(f, "\n");
    }
    if (tim->m.urlBPresent) {
        fprintf(f, "url base: %s\n", tim->urlB);
    }
    if (tim->m.dataFrameCountPresent) {
        fprintf(f, "data frame count: %d\n", tim->dataFrameCount);
    }
    fprintf(f, "msg CRC: 0x%.2X%.2X\n", tim->crc.data[0], tim->crc.data[1]);

    frame = tim->dataFrames.head;
    for (i=0; i<tim->dataFrames.count; i++) {
        if (NULL == frame) {
            break;
        }
        vr = NULL;
        list = NULL;
        elem = (TravelerInformation_dataFrames_element *)frame->data;
        fprintf(f, "\nTraveler Information frame %d contents:\n", (i+1));
        fprintf(f, "----------------------------------------\n");
        switch (elem->frameType) {
            case unknown_4: 
                fprintf(f, "encoded as 'Unknown'\n");
                break;
            case advisory: 
                fprintf(f, "encoded as 'Advisory'\n");
                break;
            case roadSignage: 
                fprintf(f, "encoded as 'Road Signage'\n");
                break;
            case commercialSignage: 
                fprintf(f, "encoded as 'Commercial Signage'\n");
                break;
            default:
                fprintf(f, "unknown frame type: %d\n", elem->frameType);
                break;
        }
        switch (elem->msgId.t) {
            case T_TravelerInformation_dataFrames_element_msgId_furtherInfoID:
                fprintf(f, "msg ID 'Further Info ID' byte 1 (%.2X) byte 2 (%.2X)\n", elem->msgId.u.furtherInfoID->data[0],
                                                                                     elem->msgId.u.furtherInfoID->data[1]);
                break;
            case T_TravelerInformation_dataFrames_element_msgId_roadSignID:
                fprintf(f, "Road Sign ID details:\n");
                fprintf(f, "latitude: %.6f\n", (double)elem->msgId.u.roadSignID->position.lat / 10000000);
                fprintf(f, "longitude: %.6f\n", (double)elem->msgId.u.roadSignID->position.long_ / 10000000);
                if (elem->msgId.u.roadSignID->position.m.elevationPresent) {
                    fprintf(f, "elevation: %d\n", elem->msgId.u.roadSignID->position.elevation);
                }
                fprintf(f, "heading slice: %.2X%.2X\n", elem->msgId.u.roadSignID->viewAngle.data[0], 
                                                        elem->msgId.u.roadSignID->viewAngle.data[1]); 
                if (elem->msgId.u.roadSignID->m.mutcdCodePresent) {
                    switch(elem->msgId.u.roadSignID->mutcdCode) {
                        case none_1:
                            fprintf(f, "MUTCD code 'None'\n");
                            break;
                        case regulatory:
                            fprintf(f, "MUTCD code 'Regulatory'\n");
                            break;
                        case warning:
                            fprintf(f, "MUTCD code 'Warning'\n");
                            break;
                        case maintenance:
                            fprintf(f, "MUTCD code 'Maintenance'\n");
                            break;
                        case motoristService:
                            fprintf(f, "MUTCD code 'Motorist Service'\n");
                            break;
                        case guide:
                            fprintf(f, "MUTCD code 'Guide'\n");
                            break;
                        case rec:
                            fprintf(f, "MUTCD code 'Rec'\n");
                            break;
                        default:
                            fprintf(f, "Unknown MUTCD code: %d\n", elem->msgId.u.roadSignID->mutcdCode);
                            break;
                    }
                }
                if (elem->msgId.u.roadSignID->m.crcPresent) {
                    fprintf(f, "msg CRC: 0x%.2X%.2X\n", elem->msgId.u.roadSignID->crc.data[0],
                                                        elem->msgId.u.roadSignID->crc.data[1]);
                }
                break;
            default:
                fprintf(f, "unknown msg ID type: %d\n", elem->msgId.t);
        }

        if (elem->m.startYearPresent) {
            fprintf(f, "Start year: %d\n", elem->startYear);
        }
        fprintf(f, "Start time (minute of the year): %d\n", elem->startTime);
        fprintf(f, "Duration (in minutes): %d\n", elem->duratonTime);
        fprintf(f, "Sign Priority: %d\n", elem->priority);

        if (elem->m.commonAnchorPresent) {
            fprintf(f, "latitude: %.6f\n", (double)elem->commonAnchor.lat / 10000000);
            fprintf(f, "longitude: %.6f\n", (double)elem->commonAnchor.long_ / 10000000);
            if (elem->commonAnchor.m.elevationPresent) {
                fprintf(f, "elevation: %d\n", elem->commonAnchor.elevation);
            }
        }

        if (elem->m.commonLaneWidthPresent) {
            fprintf(f, "common lane width (in cm): %d\n", elem->commonLaneWidth);
        }

        if (elem->m.commonDirectionalityPresent) {
            switch (elem->commonDirectionality) {
                case forward:
                    fprintf(f, "Common direction 'Forward'\n");
                    break;
                case reverse:
                    fprintf(f, "Common direction 'Reverse'\n");
                    break;
                case both:
                    fprintf(f, "Common direction 'Both Reverse and Forward'\n");
                    break;
                default:
                    fprintf(f, "Unknown directionality: %d\n", elem->commonDirectionality);
                    break;
            }
        }

        list = elem->regions.head;
        for (j=0; j<elem->regions.count; j++) {
            if (NULL == list) {
                break;
            }
            vr = (ValidRegion *)list->data;
            fprintf(f, "*****************************\n");
            fprintf(f, "Valid Region %d contents:\n", (j+1));
            fprintf(f, "heading slice: %.2X%.2X\n", vr->direction.data[0], vr->direction.data[1]);
            if (vr->m.extentPresent) {
                fprintf(f, "extent (see J2735 spec - unsupported in Denso software): %d\n", vr->extent);
            }
            switch (vr->area.t) {
                case T_ValidRegion_area_shapePointSet:
                    fprintf(f, "Shape Point Set with attributes that follow\n");
                    if (vr->area.u.shapePointSet->m.anchorPresent) {
                        fprintf(f, "latitude: %.6f\n", (double)vr->area.u.shapePointSet->anchor.lat / 10000000);
                        fprintf(f, "longitude: %.6f\n", (double)vr->area.u.shapePointSet->anchor.long_ / 10000000);
                        if (elem->commonAnchor.m.elevationPresent) {
                            fprintf(f, "elevation: %d\n", vr->area.u.shapePointSet->anchor.elevation);
                        }
                    }
                    if (vr->area.u.shapePointSet->m.laneWidthPresent) {
                        fprintf(f, "lane width (in cm): %d\n", vr->area.u.shapePointSet->laneWidth);
                    }
                    if (vr->area.u.shapePointSet->m.directionalityPresent) {
                        switch (vr->area.u.shapePointSet->directionality) {
                            case forward:
                                fprintf(f, "common direction 'Forward'\n");
                                break;
                            case reverse:
                                fprintf(f, "common direction 'Reverse'\n");
                                break;
                            case both:
                                fprintf(f, "common direction 'Both Reverse and Forward'\n");
                                break;
                            default:
                                fprintf(f, "unknown directionality: %d\n", elem->commonDirectionality);
                                break;
                        }
                    }
                    fprintf(f, "lane node list (vals in cm):");
                    for (k=0; k<vr->area.u.shapePointSet->nodeList.n; k++) {
                        /* offsets are 16 bits x first then y in network order */
                        dummy1 = (vr->area.u.shapePointSet->nodeList.elem[k].data[0] << 8) + 
                                  vr->area.u.shapePointSet->nodeList.elem[k].data[1];  /* X */
                        dummy2 = (vr->area.u.shapePointSet->nodeList.elem[k].data[2] << 8) + 
                                  vr->area.u.shapePointSet->nodeList.elem[k].data[3];  /* Y */
                        fprintf(f, " X:%.2f Y:%.2f", dummy1, dummy2);
                    }
                    fprintf(f, "\n");
                    break;
                case T_ValidRegion_area_circle:
                    fprintf(f, "Circle with attributes that follow\n");
                    fprintf(f, "latitude: %.6f\n", (double)vr->area.u.circle->center.lat / 10000000);
                    fprintf(f, "longitude: %.6f\n", (double)vr->area.u.circle->center.long_ / 10000000);
                    if (vr->area.u.circle->center.m.elevationPresent) {
                        fprintf(f, "elevation: %d\n", vr->area.u.circle->center.elevation);
                    }
                    switch (vr->area.u.circle->raduis.t) {
                        case T_Circle_raduis_radiusSteps:
                            fprintf(f, "Radius %d in radius steps\n", vr->area.u.circle->raduis.u.radiusSteps);
                            break;
                        case T_Circle_raduis_miles:
                            fprintf(f, "Radius %d in miles\n", vr->area.u.circle->raduis.u.miles);
                            break;
                        case T_Circle_raduis_km:
                            fprintf(f, "Radius %d in km\n", vr->area.u.circle->raduis.u.km);
                            break;
                        default:
                            fprintf(f, "Unknown radius value\n");
                            break;
                    }
                    break;
                case T_ValidRegion_area_regionPointSet:
                    fprintf(f, "Region Point Set unsupported in Denso software\n");
                    break;
                default:
                    fprintf(f, "Unsupported valid region type: %d\n", vr->area.t);
                    break;
            }

            list = list->next;
        } /* for j - valid region */

        /* content - list supported but must be of the same type */
        switch (elem->content.t) {
            case T_TravelerInformation_dataFrames_element_content_advisory:
                list = elem->content.u.advisory->head;
                fprintf(f, "*****************************\n");
                fprintf(f, "\nAdvisory content:\n");
                for (j=0; j<elem->content.u.advisory->count; j++) {
                    if (NULL == list) {
                        break;
                    }
                    adv = (ITIScodesAndText_element *)list->data;
                    if (adv->item.t == T_ITIScodesAndText_element_item_itis) {
                        fprintf(f, "ITIS code: %d\n", adv->item.u.itis);
                    } else {
                        fprintf(f, "ITIS text: %s\n", adv->item.u.text);
                    }
                    list = list->next;
                }
                break;
            case T_TravelerInformation_dataFrames_element_content_workZone:
                list = elem->content.u.workZone->head;
                fprintf(f, "*****************************\n");
                fprintf(f, "\nWork zone content:\n");
                for (j=0; j<elem->content.u.workZone->count; j++) {
                    if (NULL == list) {
                        break;
                    }
                    pwz = (WorkZone_element *)list->data;
                    if (pwz->item.t == T_WorkZone_element_item_itis) {
                        fprintf(f, "Work zone code: %d\n", pwz->item.u.itis);
                    } else {
                        fprintf(f, "Work zone text: %s\n", pwz->item.u.text);
                    }
                    list = list->next;
                }
                break;
            case T_TravelerInformation_dataFrames_element_content_genericSign:
                list = elem->content.u.genericSign->head;
                fprintf(f, "*****************************\n");
                fprintf(f, "\nGeneric signage content:\n");
                for (j=0; j<elem->content.u.genericSign->count; j++) {
                    if (NULL == list) {
                        break;
                    }
                    pgs = (GenericSignage_element *)list->data;
                    if (pgs->item.t == T_GenericSignage_element_item_itis) {
                        fprintf(f, "Generic signage code: %d\n", pgs->item.u.itis);
                    } else {
                        fprintf(f, "Generic signage text: %s\n", pgs->item.u.text);
                    }
                    list = list->next;
                }
                break;
            case T_TravelerInformation_dataFrames_element_content_speedLimit:
                list = elem->content.u.speedLimit->head;
                fprintf(f, "*****************************\n");
                fprintf(f, "\nSpeed limit content:\n");
                for (j=0; j<elem->content.u.speedLimit->count; j++) {
                    if (NULL == list) {
                        break;
                    }
                    psl = (SpeedLimit_element *)list->data;
                    if (psl->item.t == T_SpeedLimit_element_item_itis) {
                        fprintf(f, "Speed limit code: %d\n", psl->item.u.itis);
                    } else {
                        fprintf(f, "Speed limit text: %s\n", psl->item.u.text);
                    }
                    list = list->next;
                }
                break;
            case T_TravelerInformation_dataFrames_element_content_exitService:
                list = elem->content.u.exitService->head;
                fprintf(f, "*****************************\n");
                fprintf(f, "\nExit service content:\n");
                for (j=0; j<elem->content.u.exitService->count; j++) {
                    if (NULL == list) {
                        break;
                    }
                    pes = (ExitService_element *)list->data;
                    if (pes->item.t == T_ExitService_element_item_itis) {
                        fprintf(f, "Exit service code: %d\n", pes->item.u.itis);
                    } else {
                        fprintf(f, "Exit service text: %s\n", pes->item.u.text);
                    }
                    list = list->next;
                }
                break;
            default:
                fprintf(f, "*****************************\n");
                fprintf(f, "Unsupported content message type: %d\n", elem->content.t);
                break;
        }

        frame = frame->next;
    }  /* for i */

    fclose(f);
    return BTRUE;
}

static BOOLEAN timDecodeDER(FILE *f, char *data, int length)
{
    ASN1TAG tag;
    TravelerInformation tim;    
    int dummy;

    asn1Init_TravelerInformation(&tim);
    xd_setp(&decodeCtxt, data, length, &tag, &dummy);

    if (tag != TV_TravelerInformation) {
        printf("\n%s: invalid tag (%X)\n", __FUNCTION__, tag);
        return BFALSE;
    }

    if (asn1D_TravelerInformation(&decodeCtxt, &tim, ASN1EXPL, 0) != 0) {
        printf("\n%s: failed asn1d tim\n", __FUNCTION__);
        rtxErrPrint(&decodeCtxt);
        rtxErrReset(&decodeCtxt);
        return BFALSE;
    }

    return timsave(f, &tim);
}

static BOOLEAN timDecodePER(FILE *f, char *data, int length, char aligned)
{
    TravelerInformation tim;

    asn1Init_TravelerInformation(&tim);
    pu_setBuffer(&decodeCtxt, data, length, aligned);

    if (asn1PD_TravelerInformation(&decodeCtxt, &tim) != 0) {
        rtxErrPrint(&decodeCtxt);
        rtxErrReset(&decodeCtxt);
        return BFALSE;
    }

    return timsave(f, &tim);
}

static BOOLEAN spatsave(FILE *f, SPAT *spat)
{
    fprintf(f, "SPaT Data Contents\n");
    fprintf(f, "=========================\n\n");
    fprintf(f, "sorry - spat not supported right now - logs already decoded\n");
    fclose(f);
    return BTRUE;
}

static BOOLEAN spatDecodeDER(FILE *f, char *data, int length)
{
    ASN1TAG tag;
    SPAT spat;    
    int dummy;

    asn1Init_SPAT(&spat);
    xd_setp(&decodeCtxt, data, length, &tag, &dummy);

    if (tag != TV_SPAT) {
        printf("\n%s: invalid tag (%X)\n", __FUNCTION__, tag);
        return BFALSE;
    }

    if (asn1D_SPAT(&decodeCtxt, &spat, ASN1EXPL, 0) != 0) {
        printf("\n%s: failed asn1d spat\n", __FUNCTION__);
        rtxErrPrint(&decodeCtxt);
        rtxErrReset(&decodeCtxt);
        return BFALSE;
    }

    return spatsave(f, &spat);
}

static BOOLEAN spatDecodePER(FILE *f, char *data, int length, char aligned)
{
    SPAT spat;

    asn1Init_SPAT(&spat);
    pu_setBuffer(&decodeCtxt, data, length, aligned);

    if (asn1PD_SPAT(&decodeCtxt, &spat) != 0) {
        rtxErrPrint(&decodeCtxt);
        rtxErrReset(&decodeCtxt);
        return BFALSE;
    }

    return spatsave(f, &spat);
}

static BOOLEAN decodeMsg(const char *infile, const char *outfile)
{
    FILE *f;
    int size;

    if (NULL == (f=fopen(infile, "r"))) {
        printf("\n%s: bad file input (%s)\n", __FUNCTION__, infile);
        return BFALSE;
    }

    size = readFile(f, fstring);  /* closes file */

    if (!size) {
        printf("\n%s: input file had no data\n", __FUNCTION__);
        return BFALSE;
    }

    if (NULL == (f=fopen(outfile, "w"))) {
        printf("\n%s: failure to create output file %s\n", __FUNCTION__, outfile);
        return BFALSE;
    }

    if (rtInitContext(&decodeCtxt) != 0) {
        printf("\n%s: failed to initialize context\n", __FUNCTION__);
        rtxErrPrint(&decodeCtxt);
        return BFALSE;
    }

    /* functions close file */
    switch (asndecodetype) {
        case ASN1_DER:
                switch (msgtype) {
                    case LD_MAP:
                        return mapDecodeDER(f, fstring, size);
                    case LD_TIM:
                        return timDecodeDER(f, fstring, size);
                    case LD_SPAT:
                        return spatDecodeDER(f, fstring, size);
                    default: 
                        break;
                }
            break;
        case ASN1_PERA:
                switch (msgtype) {
                    case LD_MAP:
                        return mapDecodePER(f, fstring, size, 1);
                    case LD_TIM:
                        return timDecodePER(f, fstring, size, 1);
                    case LD_SPAT:
                        return spatDecodePER(f, fstring, size, 1);
                    default: 
                        break;
                }
            break;
        case ASN1_PERU:
                switch (msgtype) {
                    case LD_MAP:
                        return mapDecodePER(f, fstring, size, 0);
                    case LD_TIM:
                        return timDecodePER(f, fstring, size, 0);
                    case LD_SPAT:
                        return spatDecodePER(f, fstring, size, 0);
                    default: 
                        break;
                }
            break;
        default:  /* never reaches here */
            break;
    }

    /* should never reach here */
    printf("\n%s: processing error\n", __FUNCTION__);

    return BFALSE;
}

static BOOLEAN parseArgs(int argc, char *argv[])
{
    int i;

    for (i=0; i<argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'e':  /* encoding */
                    asndecodetype = atoi(argv[i+1]);
                    i++;  /* account for string arg */
                    if ((asndecodetype < ASN1_DER) || (asndecodetype > ASN1_PERU)) {
                        printf("\nunsupported encoding type\n");
                        return BFALSE;
                    }
                    break;
                case 'f':  /* file name */
                    if (strlen(argv[i+1]) > STR_MAX) {
                        printf("\nfile name too long\n");
                        return BFALSE;
                    } else {
                        strcpy(filename, argv[i+1]);
                        i++;  /* account for string arg */
                    }
                    break;
                case 'o':  /* output file */
                    if (strlen(argv[i+1]) > STR_MAX) {
                        printf("\noutput file name too long\n");
                        return BFALSE;
                    } else {
                        strcpy(ofile, argv[i+1]);
                        i++;  /* account for string arg */
                    }
                    break;
                case 'p':  /* path */
                    if (strlen(argv[i+1]) > STR_MAX) {
                        printf("\npath name too long\n");
                        return BFALSE;
                    } else {
                        strcpy(path, argv[i+1]);
                        i++;  /* account for string arg */
                    }
                    break;
                case 't':   /* message type */
                    msgtype = atoi(argv[i+1]);
                    i++;  /* account for string arg */
                    if ((msgtype < LD_MAP) || (msgtype > LD_SPAT)) {
                        printf("\nunsupported j2735 message format\n");
                        return BFALSE;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    return BTRUE;
}

void usage(char *name)
{
    while ((*name == ' ') || (*name == '/') || (*name == '.')) name++;

    printf("\nusage: %s -e <0|1|2> -f <encoded filename> -t <0|1|2> [-o <output filename> -p <path to files>]\n\n", name);
    printf("  where -e is the encoding type (DER:0, PERA:1, PERU:2)\n");
    printf("        -t is the file type (MAP:0, TIM:1, SPAT:2)\n");
    printf("        -o and -p are optional parameters\n");
}

int main(int argc, char *argv[])
{
    BOOLEAN ret = BFALSE;
    char fullname[2 * STR_MAX + 1] = {0};
    char target[2 * STR_MAX + 1] = {0};

    memset(filename, 0, sizeof(filename));
    memset(ofile, 0, sizeof(ofile));
    memset(path, 0, sizeof(path));
    memset(fstring, 0, sizeof(fstring));

    if (!parseArgs(argc, argv)) {
        usage(argv[0]);
        return -1;
    }

    if ((asndecodetype < 0) || (msgtype < 0) || (!strlen(filename))) {
        printf("\nerror: you must specify message type, encoding type, and file name\n");
        usage(argv[0]);
        return -1;
    }

    if (!strlen(ofile)) {   /* not a mandatory cmd line option */
        strcpy(ofile, DEFAULT_OUTPUT);
    }

    if (strlen(path)) {
        sprintf(fullname, "%s/%s", path, filename);
        sprintf(target, "%s/%s", path, ofile);
    } else {
        strcpy(fullname, filename);
        strcpy(target, ofile);
    }

/* debug - print args */
    //printf("\ninput file %s, outfile %s, encoding %d, filetype %d\n", fullname, target, asndecodetype, msgtype);

    ret = decodeMsg(fullname, target);
    rtxMemReset(&decodeCtxt);

    if (ret) {
        printf("\nsuccessfully decoded file; result stored in %s\n", target);
    } else {
        printf("\ndecode failed\n");
    }

    return 0;
}
