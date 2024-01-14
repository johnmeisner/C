//
// A simple utility library to tally received ubx messages
//

#include "types.h"

typedef struct {
    uint32_t count;
    uint32_t max_size;
    uint32_t last_size;
    uint8_t * last_msg;
} ubx_track_msg_data_t;

void ubx_track_reset();
void ubx_track_add(uint8_t classid, uint8_t msgid, uint32_t size, uint8_t * msgbuff);
uint8_t get_ubx_track_data(uint8_t classid, uint8_t msgid, ubx_track_msg_data_t * ptr);
void ubx_track_dump(int w_msg_names, int w_msg_data);
