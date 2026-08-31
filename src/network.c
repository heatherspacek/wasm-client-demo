#include "network.h"

void _make_request(Outbox *outbox) {
    // unused rn.
    outbox->pending = 1;

    js_pop_outbox();
}

int _poll_inbox(Inbox *inbox) {
   return inbox->ready;
}

uint8_t _dummy_receive(Inbox *inbox) {
    inbox->ready = 0;
    return inbox->data[0];
}
