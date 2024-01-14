/**************************************************************************
 *                                                                        *
 *     File Name:  imfsend.c  (immediate forward send utility)            *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define MAX_STR_LEN   200
#define MAX_FILE_SIZE (1024 * 4)   /* For now limit to 1K chunks */

#define SLEEP_1_MS      1000
#define SLEEP_5_MS      2500
#define SLEEP_10_MS     10000
#define SLEEP_100_MS    100000
#define SLEEP_1_SEC     1000000
#define SLEEP_5_SEC     5000000
#define SLEEP_10_SEC    10000000
#define SLEEP_20_SEC    20000000


#define IMF_START_TIME ""
#define IMF_STOP_TIME  ""
#define IMF_INTERVAL   (0)
#define IMF_SLEEP_TIME  (1 * SLEEP_1_MS) //SLEEP_100_MS

#define SAR_START_TIME "09/20/2019, 18:37"
#define SAR_STOP_TIME  "09/20/2020, 18:38"
#define SAR_INTERVAL   (1)
#define SAR_SLEEP_TIME  SLEEP_100_MS


//#define BIG_AMH_MAPS_100

typedef enum {
    IFALSE = 0,
    ITRUE
} IBOOL;

static IBOOL sendOnce = IFALSE;

static struct sockaddr_in targetsvr;
static unsigned short int port = 0;
static IBOOL mainloop = ITRUE;
static int fbcount = 0;    /* file bytes */
static char *databuf = NULL;

static int fbcount2 = 0;    /* file bytes */


static char bigbuf[MAX_FILE_SIZE];

#if 0

static char msg0[] =  "00604b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg1[] =  "00614b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg2[] =  "00624b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg3[] =  "00634b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg4[] =  "00644b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg5[] =  "00654b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg6[] =  "00664b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg7[] =  "00674b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg8[] =  "00684b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg9[] =  "006984b038003000200b16014976b904256495c8105a026c4258022870080c80005aeabf0eec04fb35d08e5c080a0a6bd44474a0142849222000249090001248080008b01050e0101a000135e66226b009f66ba824aa401414d7ced86ff02829afa5710e8050a0a441000152228000a580a2870080d00009af53d144404fb35d5a254740a0a6be76c38c01414d7d25886802850121e8000a58062870080e00011af0dd11ea04fb35d3fa52980a0a6bd79c69b81428d7c828c5e02851afd1f1d28051435ffd251540a14048bc000116010a1c0203800056bb95441e013ecd752a94d802829af5051a94050a35f26e33040a146bf8cc3a201428d7f83898602851affcf247005050a466000092310000430104470080e00008afee7192c5787b8d7c6bd4145e781428d7d5787a102828601c88e0101c000295f80e2ec4af11f1bacd7a318c0102829af65b1e4e051435f6fa29ec0a1e6bf3c473381450d7fd28be602828601888e0101c000335f3322c4809f66bc34c70581414d7a4a8bee02829af8a11554050a35fb22133c0a146befcc43101428d7e838a3a02879aff2d24c8050a16049a1c0203200002c24d453cb6a67a4a0c0a50248310002241c8001121040008581a6870080d00004b0f350a10da75f914602919b5475261804e20a4610001d2328000e581c6870080d00004b107b0838da7c990e902919b53d3268804e20245d0001cb028d0e0101c00001616822255b946f425a05190248a00010b02cd0e0101c00021618e21db9ba8514b1605053615de0bdc09f66d119c7a401428d9b098c93028c9b2f91132205140248600010b030d0e0101c0002161b0e18d9b896d3ce2051e36bc7e570009ce6c89ac3f481428d98308b14028c9b23010d16050f0248200010604508e0101c0000162dd9f8a1b5da32a4605230c07a11c0203800012c4e9c08fb6a74249600a466d40449160137430208470080e00004b151cff2eda9e992b502919b50e5243204dd0c11411c0203800002ba1340a21529b1a060608608e0101c000015d405ff84a942ad04a30410470080e00000aeb30fd6a549ef68122c1a7438040640000d7b0f751e02829a8d7acd9004fb1245c8002123040010919200082c1e7438040680000577c97ab52a4fe34158a41d0000d21080006583ee870080d00000aee4ef7b254a47682b048320001960dba1c0203800016bd0bbb6e81414d43966583027d9a86d8cbfc04e70244600020b071d0e0101c0000b5e4ede13c0a0a6a1f7b2a9813ecd434a660502738122100010583ae870080e00000af17ef2ecd118f4ffa02760121f00010302ec470080e00005b06dce84a0505363d9d8de80a142c1d1bd220c0c311c0203800016c12db9c501414d8f03637402850b07b6f3e63032c470080e00005b026ae5b80505363b758d3c0a142c2233ce416092a1c0203200016c54abc3601414d8ecb63cb02829b07b6f43e05051243c800192204000c911200062c155438040680002d867b75eb02829b1e82c69e050a160e8de9c0524280002922400012c165438040680002d8562755002829b1d96c74c050a160fede7b41240800025826a870080e00005b11f4eed60505363ced8db80a142c1c83d158a41e000192110000c5828a870080e00005b0f5eed860505363d858c100a142c1c83d4d8241a00018";
static char msg10[] = "006a4b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg11[] = "006b4b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg12[] = "006c4b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg13[] = "006d4b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg14[] = "006e4b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg15[] = "006f4b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg16[] = "00704b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg17[] = "00714b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg18[] = "007284b038003000200b16014976b904256495c8105a026c4258022870080c80005aeabf0eec04fb35d08e5c080a0a6bd44474a0142849222000249090001248080008b01050e0101a000135e66226b009f66ba824aa401414d7ced86ff02829afa5710e8050a0a441000152228000a580a2870080d00009af53d144404fb35d5a254740a0a6be76c38c01414d7d25886802850121e8000a58062870080e00011af0dd11ea04fb35d3fa52980a0a6bd79c69b81428d7c828c5e02851afd1f1d28051435ffd251540a14048bc000116010a1c0203800056bb95441e013ecd752a94d802829af5051a94050a35f26e33040a146bf8cc3a201428d7f83898602851affcf247005050a466000092310000430104470080e00008afee7192c5787b8d7c6bd4145e781428d7d5787a102828601c88e0101c000295f80e2ec4af11f1bacd7a318c0102829af65b1e4e051435f6fa29ec0a1e6bf3c473381450d7fd28be602828601888e0101c000335f3322c4809f66bc34c70581414d7a4a8bee02829af8a11554050a35fb22133c0a146befcc43101428d7e838a3a02879aff2d24c8050a16049a1c0203200002c24d453cb6a67a4a0c0a50248310002241c8001121040008581a6870080d00004b0f350a10da75f914602919b5475261804e20a4610001d2328000e581c6870080d00004b107b0838da7c990e902919b53d3268804e20245d0001cb028d0e0101c00001616822255b946f425a05190248a00010b02cd0e0101c00021618e21db9ba8514b1605053615de0bdc09f66d119c7a401428d9b098c93028c9b2f91132205140248600010b030d0e0101c0002161b0e18d9b896d3ce2051e36bc7e570009ce6c89ac3f481428d98308b14028c9b23010d16050f0248200010604508e0101c0000162dd9f8a1b5da32a4605230c07a11c0203800012c4e9c08fb6a74249600a466d40449160137430208470080e00004b151cff2eda9e992b502919b50e5243204dd0c11411c0203800002ba1340a21529b1a060608608e0101c000015d405ff84a942ad04a30410470080e00000aeb30fd6a549ef68122c1a7438040640000d7b0f751e02829a8d7acd9004fb1245c8002123040010919200082c1e7438040680000577c97ab52a4fe34158a41d0000d21080006583ee870080d00000aee4ef7b254a47682b048320001960dba1c0203800016bd0bbb6e81414d43966583027d9a86d8cbfc04e70244600020b071d0e0101c0000b5e4ede13c0a0a6a1f7b2a9813ecd434a660502738122100010583ae870080e00000af17ef2ecd118f4ffa02760121f00010302ec470080e00005b06dce84a0505363d9d8de80a142c1d1bd220c0c311c0203800016c12db9c501414d8f03637402850b07b6f3e63032c470080e00005b026ae5b80505363b758d3c0a142c2233ce416092a1c0203200016c54abc3601414d8ecb63cb02829b07b6f43e05051243c800192204000c911200062c155438040680002d867b75eb02829b1e82c69e050a160e8de9c0524280002922400012c165438040680002d8562755002829b1d96c74c050a160fede7b41240800025826a870080e00005b11f4eed60505363ced8db80a142c1c83d158a41e000192110000c5828a870080e00005b0f5eed860505363d858c100a142c1c83d4d8241a00018";
static char msg19[] = "00734b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg20[] = "00744b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg21[] = "00754b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg22[] = "00764b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg23[] = "00774b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
#else
static char msg0[] =  "00604b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d09";
static char msg1[] =  "00614b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d0";
static char msg2[] =  "00624b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d";
static char msg3[] =  "00634b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826";
static char msg4[] =  "00644b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a82";
static char msg5[] =  "00654b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a8";
static char msg6[] =  "0066";
static char msg7[] =  "00674b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg8[] =  "00684b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg9[] =  "006984b038003000200b16014976b904256495c8105a026c4258022870080c80005aeabf0eec04fb35d08e5c080a0a6bd44474a0142849222000249090001248080008b01050e0101a000135e66226b009f66ba824aa401414d7ced86ff02829afa5710e8050a0a441000152228000a580a2870080d00009af53d144404fb35d5a254740a0a6be76c38c01414d7d25886802850121e8000a58062870080e00011af0dd11ea04fb35d3fa52980a0a6bd79c69b81428d7c828c5e02851afd1f1d28051435ffd251540a14048bc000116010a1c0203800056bb95441e013ecd752a94d802829af5051a94050a35f26e33040a146bf8cc3a201428d7f83898602851affcf247005050a466000092310000430104470080e00008afee7192c5787b8d7c6bd4145e781428d7d5787a102828601c88e0101c000295f80e2ec4af11f1bacd7a318c0102829af65b1e4e051435f6fa29ec0a1e6bf3c473381450d7fd28be602828601888e0101c000335f3322c4809f66bc34c70581414d7a4a8bee02829af8a11554050a35fb22133c0a146befcc43101428d7e838a3a02879aff2d24c8050a16049a1c0203200002c24d453cb6a67a4a0c0a50248310002241c8001121040008581a6870080d00004b0f350a10da75f914602919b5475261804e20a4610001d2328000e581c6870080d00004b107b0838da7c990e902919b53d3268804e20245d0001cb028d0e0101c00001616822255b946f425a05190248a00010b02cd0e0101c00021618e21db9ba8514b1605053615de0bdc09f66d119c7a401428d9b098c93028c9b2f91132205140248600010b030d0e0101c0002161b0e18d9b896d3ce2051e36bc7e570009ce6c89ac3f481428d98308b14028c9b23010d16050f0248200010604508e0101c0000162dd9f8a1b5da32a4605230c07a11c0203800012c4e9c08fb6a74249600a466d40449160137430208470080e00004b151cff2eda9e992b502919b50e5243204dd0c11411c0203800002ba1340a21529b1a060608608e0101c000015d405ff84a942ad04a30410470080e00000aeb30fd6a549ef68122c1a7438040640000d7b0f751e02829a8d7acd9004fb1245c8002123040010919200082c1e7438040680000577c97ab52a4fe34158a41d0000d21080006583ee870080d00000aee4ef7b254a47682b048320001960dba1c0203800016bd0bbb6e81414d43966583027d9a86d8cbfc04e70244600020b071d0e0101c0000b5e4ede13c0a0a6a1f7b2a9813ecd434a660502738122100010583ae870080e00000af17ef2ecd118f4ffa02760121f00010302ec470080e00005b06dce84a0505363d9d8de80a142c1d1bd220c0c311c0203800016c12db9c501414d8f03637402850b07b6f3e63032c470080e00005b026ae5b80505363b758d3c0a142c2233ce416092a1c0203200016c54abc3601414d8ecb63cb02829b07b6f43e05051243c800192204000c911200062c155438040680002d867b75eb02829b1e82c69e050a160e8de9c0524280002922400012c165438040680002d8562755002829b1d96c74c050a160fede7b41240800025826a870080e00005b11f4eed60505363ced8db80a142c1c83d158a41e000192110000c5828a870080e00005b0f5eed860505363d858c100a142c1c83d4d8241a00018";
static char msg10[] = "006a4b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg11[] = "006b4b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg12[] = "006c4b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg13[] = "006d4b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg14[] = "006e4b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg15[] = "006f4b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg16[] = "00704b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg17[] = "00714b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg18[] = "007284b038003000200b16014976b904256495c8105a026c4258022870080c80005aeabf0eec04fb35d08e5c080a0a6bd44474a0142849222000249090001248080008b01050e0101a000135e66226b009f66ba824aa401414d7ced86ff02829afa5710e8050a0a441000152228000a580a2870080d00009af53d144404fb35d5a254740a0a6be76c38c01414d7d25886802850121e8000a58062870080e00011af0dd11ea04fb35d3fa52980a0a6bd79c69b81428d7c828c5e02851afd1f1d28051435ffd251540a14048bc000116010a1c0203800056bb95441e013ecd752a94d802829af5051a94050a35f26e33040a146bf8cc3a201428d7f83898602851affcf247005050a466000092310000430104470080e00008afee7192c5787b8d7c6bd4145e781428d7d5787a102828601c88e0101c000295f80e2ec4af11f1bacd7a318c0102829af65b1e4e051435f6fa29ec0a1e6bf3c473381450d7fd28be602828601888e0101c000335f3322c4809f66bc34c70581414d7a4a8bee02829af8a11554050a35fb22133c0a146befcc43101428d7e838a3a02879aff2d24c8050a16049a1c0203200002c24d453cb6a67a4a0c0a50248310002241c8001121040008581a6870080d00004b0f350a10da75f914602919b5475261804e20a4610001d2328000e581c6870080d00004b107b0838da7c990e902919b53d3268804e20245d0001cb028d0e0101c00001616822255b946f425a05190248a00010b02cd0e0101c00021618e21db9ba8514b1605053615de0bdc09f66d119c7a401428d9b098c93028c9b2f91132205140248600010b030d0e0101c0002161b0e18d9b896d3ce2051e36bc7e570009ce6c89ac3f481428d98308b14028c9b23010d16050f0248200010604508e0101c0000162dd9f8a1b5da32a4605230c07a11c0203800012c4e9c08fb6a74249600a466d40449160137430208470080e00004b151cff2eda9e992b502919b50e5243204dd0c11411c0203800002ba1340a21529b1a060608608e0101c000015d405ff84a942ad04a30410470080e00000aeb30fd6a549ef68122c1a7438040640000d7b0f751e02829a8d7acd9004fb1245c8002123040010919200082c1e7438040680000577c97ab52a4fe34158a41d0000d21080006583ee870080d00000aee4ef7b254a47682b048320001960dba1c0203800016bd0bbb6e81414d43966583027d9a86d8cbfc04e70244600020b071d0e0101c0000b5e4ede13c0a0a6a1f7b2a9813ecd434a660502738122100010583ae870080e00000af17ef2ecd118f4ffa02760121f00010302ec470080e00005b06dce84a0505363d9d8de80a142c1d1bd220c0c311c0203800016c12db9c501414d8f03637402850b07b6f3e63032c470080e00005b026ae5b80505363b758d3c0a142c2233ce416092a1c0203200016c54abc3601414d8ecb63cb02829b07b6f43e05051243c800192204000c911200062c155438040680002d867b75eb02829b1e82c69e050a160e8de9c0524280002922400012c165438040680002d8562755002829b1d96c74c050a160fede7b41240800025826a870080e00005b11f4eed60505363ced8db80a142c1c83d158a41e000192110000c5828a870080e00005b0f5eed860505363d858c100a142c1c83d4d8241a00018";
static char msg19[] = "00734b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg20[] = "00744b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg21[] = "00754b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg22[] = "00764b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";
static char msg23[] = "00774b201000000000008aa4814082b292c5d7324afacb522c95fffe4fc4d8433f4030407e5258bae6495f596a459280b72ffff00015bbf68cedb5092589c04fb000c37334edd3d10326a826d090";

#endif
/* Protos */

static char * find_me_some_payload(void);

static void sigHandler(int __attribute__((unused)) sig)
{
    mainloop = IFALSE;
}

static char start_date[10][25];
static char stop_date[10][25];
static void populate_stop_start_table(void)
{
  int count;

      count = sizeof("09/09/2019, 23:30");
      printf("\nCount=%d\n",count);

      memcpy (&start_date[0][0],"09/09/2019, 23:30", count);
      memcpy (&start_date[1][0],"09/09/2019, 23:31", count);
      memcpy (&start_date[2][0],"09/09/2019, 23:32", count);
      memcpy (&start_date[3][0],"09/09/2019, 23:33", count);
      memcpy (&start_date[4][0],"09/09/2019, 23:34", count);
      memcpy (&start_date[5][0],"09/09/2019, 23:35", count);
      memcpy (&start_date[6][0],"09/09/2019, 23:36", count);
      memcpy (&start_date[7][0],"09/09/2019, 23:37", count);
      memcpy (&start_date[8][0],"09/09/2019, 23:38", count);
      memcpy (&start_date[9][0],"09/09/2019, 23:39", count);

      memcpy (&stop_date[0][0],"09/09/2020, 23:40", count);
      memcpy (&stop_date[1][0],"09/09/2020, 23:41", count);
      memcpy (&stop_date[2][0],"09/09/2020, 23:42", count);
      memcpy (&stop_date[3][0],"09/09/2020, 23:43", count);
      memcpy (&stop_date[4][0],"09/09/2020, 23:44", count);
      memcpy (&stop_date[5][0],"09/09/2020, 23:45", count);
      memcpy (&stop_date[6][0],"09/09/2020, 23:46", count);
      memcpy (&stop_date[7][0],"09/09/2020, 23:47", count);
      memcpy (&stop_date[8][0],"09/09/2020, 23:48", count);
      memcpy (&stop_date[9][0],"09/09/2020, 23:49", count);

}

static void send_100_SAR(int MY_INTERVAL, int MY_SLEEP_TIME, int sock)
{
  int i,j;

    for(i=0;i<10;i++)
    {
      for(j=0;j<10;j++)
      {
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "MAP"
                    ,0x8005
                    ,MY_INTERVAL,&start_date[i][0],&stop_date[j][0],find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending map\n");
            }
      }
    }

}

volatile unsigned int foobar = 0x0;
volatile unsigned int barfoo = 0x0;
static char * find_me_some_payload(void)
{
  char * foo = NULL;
  int    bar = 0x0;

    bar = ( (rand() + 24) % 24 );

    switch(bar)
    {
    case 0:
        foo = msg0;
        foobar |=0x1;    
    break;
    case 1:
        foo = msg1;
        foobar |=0x2;
    break;
    case 2:
        foo = msg2;
        foobar |=0x4;
    break;
    case 3:
        foo = msg3;
        foobar |=0x8;
    break;
    case 4:
        foo = msg4;
        foobar |=0x10;
    break;
    case 5:
        foo = msg5;
        foobar |=0x20;
    break;
    case 6:
        foo = msg6;
        foobar |=0x40;
    break;
    case 7:
        foo = msg7;
        foobar |=0x80;
    break;
    case 8:
        foo = msg8;
        foobar |=0x100;
    break;
    case 9:
        foo = msg9;
        foobar |=0x200;
    break;
    case 10:
        foo = msg10;
        foobar |=0x400;
    break;
    case 11:
        foo = msg11;
        foobar |=0x800;
    break;
    case 12:
        foo = msg12;
        foobar |=0x1000;
    break;
    case 13:
        foo = msg13;
        foobar |=0x2000;
    break;
    case 14:
        foo = msg14;
        foobar |=0x4000;
    break;
    case 15:
        foo = msg15;
        foobar |=0x8000;
    break;
    case 16:
        foo = msg16;
        foobar |=0x10000;
    break;
    case 17:
        foo = msg17;
        foobar |=0x20000;
    break;
    case 18:
        foo = msg18;
        foobar |=0x40000;
    break;
    case 19:
        foo = msg19;
        foobar |=0x80000;
    break;
    case 20:
        foo = msg20;
        foobar |=0x100000;
    break;
    case 21:
        foo = msg21;
        foobar |=0x200000;
    break;
    case 22:
        foo = msg22;
        foobar |=0x400000;
    break;
    case 23:
        foo = msg23;
        foobar |=0x800000;
    break;
    default:
        printf("\nfoobar is fubar!\n");
        foo = msg23;
    break;
    }

    if (0xFFFFFF != foobar)
    //if(0 == ( barfoo % 10))
    {
      printf("\nfoobar=0x%x bar=%d %d\n",foobar, bar, barfoo);
      barfoo++;      
    }

    return foo;

}

static void usage(char *name) 
{
    char *idx = &name[strlen(name) - 1];
    int count = 0;
    char *printname, *basename = NULL;
    IBOOL useRegName = ITRUE;

    /* print name beautification */
    while (idx != name) {
        if (*idx == '/') {
            basename = (char *)malloc(count);
            if (basename != NULL) {
                memset(basename, 0, count);
                strcpy(basename, ++idx);
                useRegName = IFALSE;
            }
            break;
        }
        count++;
        idx--;
    }
    printname = (useRegName) ? name : basename;

    /* now to the actual usage */
    printf("\nUsage: %s -t <target IP> -p <target port> -R <repeat:0..1> -m <mode:0..2>\n", printname);
    printf("\n\twhere mandatory input args are:\n");
    printf("\t-t\tIP address of RSE\n");
    printf("\t-p\tListening port of RSE\n");
    printf("\t-F\tInterval to use when repeating send\n");
    printf("\t\t***Use Ctrl+C to end send***\n\n");

    if (!useRegName) {
        free(basename);
    }
}
unsigned char interval = 0xFF;
int mode               = -1;
static IBOOL parseArgs(int argc, char *argv[])
{
    char c;

    IBOOL invalid = IFALSE;
    IBOOL validIP = IFALSE;
    IBOOL portSet = IFALSE;

    while ((c=getopt(argc, argv, "r:hp:m:t:")) != -1) {
        switch (c) {
            case 'm':
                mode = atoi(optarg);
                break;
            case 'r':
                interval = atoi(optarg);
                    switch (interval) {
                        case 0: sendOnce = ITRUE;  break;
                        case 1: sendOnce = IFALSE; break;
                        default:
                          invalid = ITRUE;
                        break;
                    }
                break;
            case 'p':   /* port */
                port = (unsigned short int)strtoul(optarg, NULL, 0);
                portSet = ITRUE;
                targetsvr.sin_port = htons(port);
                break;
            case 't':   /* IP addr */
                if (!inet_aton(optarg, &targetsvr.sin_addr)) {
                    printf("\nInvalid address\n");
                    invalid = ITRUE;
                } else {
                    validIP = ITRUE;
                }
                break;
            case 'h':   /* help - but suppress getopt invalid arg print */
            default:
                invalid = ITRUE;
                break;
        }
        if (invalid) {
            return IFALSE;
        }
    }
    if (!validIP) {
        printf("\nYou must specify a RSE [-i]p.\n");
        return IFALSE;
    }

    if (!portSet) {
        printf("\nYou must specify a [-p]ort.\n");
        return IFALSE;
    }
    
    if(-1 == mode)
    {
        printf("\nYou must specify a [-m]ode.\n");
        return IFALSE;
    }
    if (0xff == interval) {
        printf("\nYou must specify [-r}epeat or not.\n");
        return IFALSE;
    }

    printf("\nusing mandatory args: ip[%s], port[%d], repeat[%d] mode=[%d]\n", inet_ntoa(targetsvr.sin_addr), port, interval, mode);

    return ITRUE;
}

static IBOOL initSock(int *sock)
{
    if ((*sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        printf("\nsocket creation failed\n");
        return IFALSE;
    }

    /* address already set */
    targetsvr.sin_family = AF_INET;

    if (connect(*sock, (struct sockaddr *)&targetsvr, sizeof(targetsvr)) < 0) {
        printf("\nconnect error to RSE\n");
        close(*sock);
        return IFALSE;
    }

    return ITRUE;
}

static void call_msg_send(int MY_INTERVAL,char * MY_START_TIME,char * MY_STOP_TIME,int MY_SLEEP_TIME,int sock)
{

            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST00"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload());
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test00\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST01"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test01\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST02"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test02\n");
            }
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST03"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test03\n");
            }
            usleep(MY_SLEEP_TIME); 
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST04"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test04\n");
            }
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST05"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test05\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST06"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test06\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST07"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test07\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST08"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test08\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST09"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test09\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST10"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test10\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST11"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test11\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST12"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test12\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST13"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test13\n"); 
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST14"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test14\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TEST15"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending test15\n");
            } 
            usleep(MY_SLEEP_TIME);

            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "ICA"
                    ,0x8004
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending ica\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "MAP"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending map\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "PDM"
                    ,0x8005
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending pdm\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "RSA"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending psa\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "RTCM"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending rtcm\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "SPAT"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending spat\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "SSM"
                    ,0x8002
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending ssm\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "TIM"
                    ,0x8003
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending tim\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "CSR"
                    ,0x8003
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending csr\n");
            }
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "EVA"
                    ,0x8003
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending eva\n");
            }
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "NMEA"
                    ,0x8003
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload());
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending nmea\n");
            }
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "PSM"
                    ,0x8003
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending psm\n");
            }
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "PVD"
                    ,0x8003
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending pvd\n");
            }
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "SRM"
                    ,0x8003
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload());
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending srm\n");
            }
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload=%s\n",
                    "BSM"
                    ,0x20
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending bsm\n");
            }
            usleep(MY_SLEEP_TIME);

  return;
}

static void call_legacy_msg_send(int MY_INTERVAL,char * MY_START_TIME,char * MY_STOP_TIME,int MY_SLEEP_TIME,int sock)
{

            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload1=%s\n",
                    "SPAT"
                    ,0x8003
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending legacy SPAT\n");
            } 

            usleep(MY_SLEEP_TIME);

            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload1=%s\n",
                    "TIM"
                    ,0x8004
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending legacy TIM\n");
            } 
            usleep(MY_SLEEP_TIME);
            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload1=%s\n",
                    "RTCM"
                    ,0x8005
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending legacy RTCM\n");
            } 
            usleep(MY_SLEEP_TIME);

            memset(bigbuf,'\0',sizeof(bigbuf));
            sprintf(bigbuf,"Version=0.7\nType=%s\nPSID=0x%x\nPriority=7\nTxMode=ALT\nTxChannel=172\nTxInterval=%d\nDeliveryStart=%s \nDeliveryStop=%s\nSignature=False\nEncryption=False\nPayload1=%s\n",
                    "MAP"
                    ,0x8006
                    ,MY_INTERVAL,MY_START_TIME,MY_STOP_TIME,find_me_some_payload()); 
            fbcount = strlen(bigbuf);
            if (fbcount != write(sock, bigbuf, fbcount)) {
                printf("error sending legacy MAP\n");
            }

            usleep(MY_SLEEP_TIME);

  return;
}

int main(int argc, char *argv[])
{
    int sock = -1;
    struct sigaction sa;
    int i;
    int  Recoverytime = 0;

    memset(&targetsvr, 0, sizeof(targetsvr));

    if (!parseArgs(argc, argv)) {
        usage(argv[0]);
        return IFALSE;
    }

    if (!initSock(&sock)) {
        printf("\nFailed to initialize socket\n\n");
        free(databuf);
        return IFALSE;
    }

        /* register for interrupt signals */
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = sigHandler;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);

        Recoverytime = SLEEP_100_MS - (SLEEP_1_MS * 31) - (SLEEP_1_MS * 35); /* Empirically derived magic number. Likely changes based on size on data and wait periods between */
        Recoverytime = (SLEEP_1_MS * 30);
        for(i=0;(mainloop) && !(i && sendOnce);i++) {

           usleep(Recoverytime); /* Allow RSU time to digest I/O with additional sleep time on Host side. */

           switch(mode) 
           {
             case 0:
               call_msg_send(IMF_INTERVAL,IMF_START_TIME,IMF_STOP_TIME,IMF_SLEEP_TIME,sock);
             break;
             case 1:
               call_msg_send(SAR_INTERVAL,SAR_START_TIME,SAR_STOP_TIME,SAR_SLEEP_TIME,sock);
             break;
             case 2:
               populate_stop_start_table();
               send_100_SAR(SAR_INTERVAL,SAR_SLEEP_TIME,sock);
             break;
             case 3:
               call_legacy_msg_send(IMF_INTERVAL,IMF_START_TIME,IMF_STOP_TIME,IMF_SLEEP_TIME,sock);
             break;
             case 4:
               call_legacy_msg_send(SAR_INTERVAL,SAR_START_TIME,SAR_STOP_TIME,SAR_SLEEP_TIME,sock);
             break;
             case 5:
               call_msg_send(IMF_INTERVAL,IMF_START_TIME,IMF_STOP_TIME,IMF_SLEEP_TIME,sock);
               usleep(IMF_SLEEP_TIME);
               call_legacy_msg_send(IMF_INTERVAL,IMF_START_TIME,IMF_STOP_TIME,IMF_SLEEP_TIME,sock);
             break;
             case 6:
               call_msg_send(SAR_INTERVAL,SAR_START_TIME,SAR_STOP_TIME,SAR_SLEEP_TIME,sock);
               usleep(SAR_SLEEP_TIME);
               call_legacy_msg_send(SAR_INTERVAL,SAR_START_TIME,SAR_STOP_TIME,SAR_SLEEP_TIME,sock);
             break;
             case 7: /* Helter Skelter: Kitchen sink it */
               call_msg_send(IMF_INTERVAL,IMF_START_TIME,IMF_STOP_TIME,IMF_SLEEP_TIME,sock);
               usleep(IMF_SLEEP_TIME);
               call_legacy_msg_send(IMF_INTERVAL,IMF_START_TIME,IMF_STOP_TIME,IMF_SLEEP_TIME,sock);
               usleep(IMF_SLEEP_TIME);
               call_msg_send(SAR_INTERVAL,SAR_START_TIME,SAR_STOP_TIME,SAR_SLEEP_TIME,sock);
               usleep(SAR_SLEEP_TIME);
               call_legacy_msg_send(SAR_INTERVAL,SAR_START_TIME,SAR_STOP_TIME,SAR_SLEEP_TIME,sock);
             break;
             case 8:
               call_msg_send(IMF_INTERVAL,IMF_START_TIME,IMF_STOP_TIME,0,sock);
             break;
             default:
               printf("\nBad mode = %d",mode);
               mainloop = IFALSE;
             break;
           }

           if(0 == (i % 100))
           {
             printf("send successful i=%d\n",i);
           }
        }

    close(sock);
    free(databuf);

    printf("\nExiting...\n\n");

    return ITRUE;
}

