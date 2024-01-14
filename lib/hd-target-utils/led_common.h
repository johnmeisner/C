#ifndef LED_COMMON_H_
#define LED_COMMON_H_
#include <sys/types.h>
#include "dn_types.h"

#define QUEUE_NAME  "/led_server"

#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0)
/*
 * === debug macros ===
 */
#define debug_print(fmt, ...) \
        do { printf("%s | %d | %s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#define debug_print_0(fmt) \
        do { printf("%s | %d | %s(): " fmt, __FILE__, \
                                __LINE__, __func__); } while (0)

/*
 *==== LED MQ defines ===
 */
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10                         /* Max queue messages */
#define MAX_MSG_SIZE 32                         /* Size of message - led commands should never exceed 32 bytes */
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

#define LED_OFF         0
#define LED_ON          1

enum {
    LED_RED,
    LED_AMBER,
    LED_GREEN,
};


typedef struct{
    uint8_t led_type;
    uint8_t led_en;
    uint32_t blink_rate;
} __attribute__ ((packed)) led_msg_hdr_t;



typedef struct {
  uint32_t proc_id;
  uint16_t thrd_id;
  led_msg_hdr_t led_attr;
} __attribute__ ((packed)) mq_led_hdr_t;

#endif /* #ifndef LED_COMMON_H_ */
