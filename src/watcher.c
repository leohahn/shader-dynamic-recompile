#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <pthread.h>
#include <memory.h>
#include <linux/limits.h>
#include <errno.h>

#include "lt.h"
#include "watcher.h"

#ifdef __linux__
#define LEN_NAME PATH_MAX
#else
// @FIXME(leo): This should not be harcoded for other operating systems.
#define LEN_NAME         16
#endif
#define EVENT_SIZE       (sizeof(struct inotify_event))
#define MAX_EVENTS       1024
#define BUF_LEN          (MAX_EVENTS * (EVENT_SIZE + LEN_NAME))
#define PATH             "/home/lhahn/dev/c/shader-loader/resources"
#define EVENT_BUFFER_LEN 50

typedef struct EventBuffer {
    isize                head;
    isize                tail;
    isize                maxlen;
    WatcherEvent        *events;
} EventBuffer;

// Event buffer
static EventBuffer     g_event_buffer = {0, 0, 0, 0};
static pthread_mutex_t g_mutex_event_buffer;
// Running state
static bool            g_running      = false;
static pthread_mutex_t g_mutex_running;


void initialize_event_buffer(EventBuffer *eb, isize maxlen) {
    eb->head = 0;
    eb->tail = 0;
    eb->maxlen = maxlen;
    eb->events = malloc(sizeof(WatcherEvent) * maxlen);

    for (isize i = 0; i < maxlen; i++) {
        eb->events[i].inotify_mask = -1;
        eb->events[i].name = NULL;
    }
}

void free_event_buffer(EventBuffer *eb) {
    lt_free(eb->events);
}

void set_running(bool r) {
    pthread_mutex_lock(&g_mutex_running);
    g_running = r;
    pthread_mutex_unlock(&g_mutex_running);
}

void push_event(EventBuffer *buf, const struct inotify_event *ie) {
    pthread_mutex_lock(&g_mutex_event_buffer);

    isize next_head = buf->head + 1;
    if (next_head >= buf->maxlen) {
        next_head = 0;
    }

    if (next_head == buf->tail) {
        LT_FAIL("Circular buffer was overrun\n");
    }

    buf->events[buf->head].inotify_mask = ie->mask;
    buf->events[buf->head].name = string_make(ie->name);
    buf->head = next_head;

    pthread_mutex_unlock(&g_mutex_event_buffer);
}

void consume_event(EventBuffer *buf) {
    pthread_mutex_lock(&g_mutex_event_buffer);

    LT_ASSERT(buf->head != buf->tail);

    isize next_tail = buf->tail + 1;
    if (next_tail >= buf->maxlen) {
        next_tail = 0;
    }

    buf->events[buf->tail].inotify_mask = -1;
    string_free(buf->events[buf->tail].name);
    buf->tail = next_tail;

    pthread_mutex_unlock(&g_mutex_event_buffer);
}

WatcherEvent *watcher_peek_event() {
    pthread_mutex_lock(&g_mutex_event_buffer);

    if (g_event_buffer.head == g_event_buffer.tail) {
        pthread_mutex_unlock(&g_mutex_event_buffer);
        return NULL;
    }

    WatcherEvent *e = &g_event_buffer.events[g_event_buffer.tail];

    pthread_mutex_unlock(&g_mutex_event_buffer);
    return e;
}

void watcher_event_peeked() {
    consume_event(&g_event_buffer);
}

void watcher_stop() {
    set_running(false);
}

void *watcher_start(void *arg) {
    LT_UNUSED(arg);

    const isize MAX_NUM_EVENTS = 10;

    // Initialize the mutexes.
    pthread_mutex_init(&g_mutex_running, NULL);
    pthread_mutex_init(&g_mutex_event_buffer, NULL);
    // Initialize the event buffer with maximum number of events.
    initialize_event_buffer(&g_event_buffer, MAX_NUM_EVENTS);

    i32 fd = inotify_init1(IN_NONBLOCK);

    if (fd < 0) {
        pthread_mutex_destroy(&g_mutex_running);
        pthread_mutex_destroy(&g_mutex_event_buffer);
        LT_FAIL("Failed starting inotify.\n");
    }

    i32 wd = inotify_add_watch((i32)fd, PATH, IN_MODIFY|IN_CREATE|IN_DELETE);

    if (wd < 0) {
        pthread_mutex_destroy(&g_mutex_running);
        pthread_mutex_destroy(&g_mutex_event_buffer);
        LT_FAIL("Could not add watch to %s\n", PATH);
    }

    char buf[BUF_LEN] = {0};

    set_running(true);

    printf("Watching folder %s\n", PATH);
    while (g_running) {
        memset(buf, 0, BUF_LEN);
        isize i = 0;
        isize len = read(fd, buf, BUF_LEN);

        if (len < 0) {
            if (errno == EAGAIN) {
                continue;
            } else {
                perror("Error reading for inotify event");
                continue;
            }
        }

        LT_ASSERT(len < (isize)BUF_LEN);

        while (i < len) {
            struct inotify_event *event = (struct inotify_event *)&buf[i];

            if (event->len) {
                // Push event to the circular buffer.
                push_event(&g_event_buffer, event);
            }

            i += EVENT_SIZE + event->len;
        }
    }
    printf("Finished watching folder %s\n", PATH);

    // Cleanup resources.
    inotify_rm_watch(fd, wd);
    close(fd);
    free_event_buffer(&g_event_buffer);
    pthread_mutex_destroy(&g_mutex_running);
    pthread_mutex_destroy(&g_mutex_event_buffer);

    pthread_exit(NULL);
}
