#import "types.h"

typedef struct
{
    uint8_t pending;
    uint8_t message_type;
    uint8_t callback_id;
    uint16_t body_len;
    uint8_t message[1024];
} Outbox;

typedef struct
{
    uint8_t ready;
    uint8_t _pad;
    uint32_t len;
    uint8_t data[1024];
} Inbox;

extern void js_pop_outbox();

void _make_request(Outbox *outbox);
int _poll_inbox(Inbox *inbox);
uint8_t _dummy_receive(Inbox *inbox);
