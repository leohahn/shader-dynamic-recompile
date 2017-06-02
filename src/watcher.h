#ifndef WATCHER_H
#define WATCHER_H

#include "lt.h"

typedef struct WatcherEvent {
    String *name;
    i32     inotify_mask;
} WatcherEvent;

void         *watcher_start(void *arg);
void          watcher_stop();
WatcherEvent *watcher_peek_event();
void          watcher_event_peeked();


#endif // WATCHER_H
