#ifndef MESSAGES
#define MESSAGES
enum {
    ACQUIRE_LOCK = 1,
    RELEASE_LOCK = 2,
    DO_WORK = 3,
    PRINT = 4,
    READ,
    WRITE,
    QUIT,
    PING,
    TOKEN,
    TOKEN_WANT,
    TOKEN_HELD,
    TOKEN_FREE
};
#endif
