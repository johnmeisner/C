
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "aerolinkPKIConnection.h"
#include "../tlsImpl.h"

// not defined by OBS/Aerolink header, useful for watching multiple connection activity
extern void dumpUCI(void);

// just for the test
uint32_t userdata[32];

//char mbr[] ={0x01,0x80,0x80,0x40,0x19,0x34,0x87,0xf5,0xce,0x19,0x77,0xf0,0x00,0xcf,00,00,01,0x3f,01,0xe8,0x8c,0x99,0x36}; //,84,00,20,82,00,01,01,da,f0,1b,fd,00,01,1b,fd,01,e8,8c,9a,10,05,00,00,07}

char mbr[170] = {"This is a functionality test.  For the time being, it is all we have.  Hoping for a response with a bunch of data so I can see the recv function work. - thanks "};

static int32_t connectionHandle = 0;
static int numloops = 0;

struct flags
{
    bool rxdata;
    int  networkAvail;
    int  connstat;
};

static struct flags f;

// data available callback 
void tlsrecvtest(int32_t result, uint8_t const* const data, size_t dataLen, void *userData)
{
    printf("ReceiveDataCallback called! data*: %p, datalen: %u,  userdata: %p\n", data, dataLen, userdata);
    printf("  Recvd data: %s\n", data);
    f.rxdata = true;
}

// network available callback
void ncbtest(bool available, void *userdata)
{

    if( true == available )
    {    
        printf("%s, Network is UP\n",__func__);
        f.networkAvail = 1;
    }
    else if( false == available )
    { 
        printf("%s, Network is DOWN\n",__func__);
        f.networkAvail = 0;
    }
    else
        printf("%s, BAD data in NetAvailableCallback\n",__func__);
    
       
}

// start here
int32_t main(int32_t argc, char** argv)
{
    int  i,j, sbytes = 0;
    static char ipaddr[48];

    f.rxdata = false;
    f.networkAvail = 0;
    f.connstat     = 0;

    memset(ipaddr, 0, sizeof(ipaddr));

    if( argc > 1 ) 
    {
        strcpy(ipaddr, argv[1]);
    }
    else 
    {
        strcpy(ipaddr, "2001:470:e0fb:9999::1");
    }

    printf("%s(): use ipaddr: %s\n", __func__, ipaddr);

    for( j=0; j<2; j++)
    {
        printf("\n\nmain(): Start Loop\n");

        printf("main(): register Network Available callback\n");
        AerolinkPKI_registerNetworkAvailCallback( ncbtest, userdata );

        // loop waiting for network avail callback to fire
        for(;;)
        {
            usleep(600000);
            if( f.networkAvail == 1 )
            {
                printf("%s(): received Network Available callback\n", __func__);
                break;
            }
            else
                continue;
        }

        printf("%s(): request connection to: %s\n", __func__, ipaddr);

        // connect now 
        if( (connectionHandle = AerolinkPKI_connect( ipaddr, 443, 0, tlsrecvtest,  userdata )) < AerolinkPKI_ERR_NONE )
        {
            printf("%s(): AerolinkPKI_connect returns NOT success\n", __func__);

            printf("%s()  calling AerolintTLS_close\n",__func__);
            AerolinkPKI_close( connectionHandle );
            printf("%s()  calling AerolintTLS_unregisterNetworkAvailCallback\n",__func__);
            AerolinkPKI_unregisterNetworkAvailCallback();

            return(-1);
        }
        else
        {
            printf("%s(): AerolinkPKI_connect returns success, cHandle: %u\n", __func__, connectionHandle);
        }

        // send
        printf("%s(): call AerolinkPKI_send\n", __func__);
        if( (sbytes = AerolinkPKI_send( connectionHandle, (uint8_t const*)mbr, sizeof(mbr))) > 0 )
        {
            printf("test_main(): AerolLinkTLS_send returns bytes: %d\n", sbytes);
        }
        else
        {
            printf("test_main(): AerolLinkTLS_send returns fail: %d\n", sbytes);
            return 0;
        }
        
        // loop waiting for recvd data callback to fire
        printf("%s(): wait for dataCallback\n", __func__);
        for(i=0;i<15;i++)
        {
            usleep(600000);
            if( f.rxdata == true )
            {
               f.rxdata = false;
               break;
            }
        }
        
        // to prevent premature close
        for( i=0; i<3; i++)
        {
            sleep(1);
        }

        //dumpUCI();

        printf("%s()  calling AerolintTLS_close\n",__func__);
        AerolinkPKI_close( connectionHandle );
        AerolinkPKI_unregisterNetworkAvailCallback();

        // reinit
        f.rxdata = false;
        f.connstat = 0;
        f.networkAvail = 0;

        printf("\n%s(): End Loop: %d\n",__func__, ++numloops);
   }
        return 0;
}
