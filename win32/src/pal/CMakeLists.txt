INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )
ADD_LIBRARY(pal STATIC
    config.h
    error.c
    event.c
    file.c
    heap.c
    msgq.c
    mutex.c
    print.c
    rand.c
    string.c
    thread.c
    timer.c
    )
