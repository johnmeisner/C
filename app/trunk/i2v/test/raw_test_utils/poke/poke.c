#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "i2v_general.h"
#include "i2v_util.h"

static i2vIfcList ifc = PHY_NONE;
static lmgrIfcReqState enableIfc = IFC_UNKNOWN;


/* this function tests the enable and disable interface
   logging functionality that is connected to the snmp
   interface */
static void pokeIfcReq(lmgrIfcReqState state, i2vIfcList ifc)
{
    /* verifying utility macros work - could directly call 
       function but the snmp commands will likely be implemented
       in such a way that the utility macros will be all that's 
       used */

    if ((state != IFC_ON) && (state != IFC_OFF)) {
        printf("\ninvalid interface request state - ignoring\n");
        return;
    }

    switch (ifc) {
        case PHY_ETH0:
            if (state == IFC_ON) 
                ACTIVATE_ETH0_LOGGING
            else
                DEACTIVATE_ETH0_LOGGING
            break;
        case PHY_ETH1:
            if (state == IFC_ON) 
                ACTIVATE_ETH1_LOGGING
            else
                DEACTIVATE_ETH1_LOGGING
            break;
        case PHY_ATH0:
            if (state == IFC_ON) 
                ACTIVATE_ATH0_LOGGING
            else
                DEACTIVATE_ATH0_LOGGING
            break;
        case PHY_ATH1:
            if (state == IFC_ON) 
                ACTIVATE_ATH1_LOGGING
            else
                DEACTIVATE_ATH1_LOGGING
            break;
        default:
            printf("\ninvalid interface requested - ignoring\n");
            return;
    }

    printf("\ninterface logging request passed\n");
}


/* future additions will need to set a control variable to indicate
   what action is being requested from the parsed arguments so 
   corresponding functionality can be called */
static WBOOL parseArgs(int argc, char *argv[])
{
    int i;

    for (i=0; i<argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'i':  /* interface arg */
                    if (!strcmp("eth0", argv[i+1])) {
                        ifc = PHY_ETH0;
                    } else if (!strcmp("eth1", argv[i+1])) {
                        ifc = PHY_ETH1;
                    } else if (!strcmp("ath0", argv[i+1])) {
                        ifc = PHY_ATH0;
                    } else if (!strcmp("ath1", argv[i+1])) {
                        ifc = PHY_ATH1;
                    }
                    i++;
                    break;
                case 'l':  /* logging argument */
                    /* arbitrary assignment */
                    switch (atoi(argv[i+1])) {
                        case 0:   /* enable ifc log */
                            enableIfc = IFC_ON;
                            break;
                        case 1:   /* disable ifc log */
                            enableIfc = IFC_OFF;
                            break;
                        default:
                            printf("\nunknown logging argument: %s\n", argv[i+1]);
                            return WFALSE;
                    }
                    break;
                default:
                    printf("\nunsupported option: %s\n", argv[i]);
                    return WFALSE;
            }
        }
    }

    return WTRUE;
}

int main(int argc, char *argv[])
{

    if (!parseArgs(argc, argv)) {
        printf("\ninvalid parameters - ignoring; try again\n");
        return I2V_RETURN_FAIL;
    }

    /* future additions need to provide corresponding function calls */
    pokeIfcReq(enableIfc, ifc);

    return I2V_RETURN_OK;
}

