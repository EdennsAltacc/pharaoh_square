#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define RUNTIME_VERSION 0x0307
#define RUNTIME_PAGE_SIZE 4096
#define RUNTIME_MAX_THREADS 16
#define RUNTIME_DEFAULT_TIMEOUT 3000

typedef enum {
    RUNTIME_IDLE = 0,
    RUNTIME_INITIALIZING = 1,
    RUNTIME_ACTIVE = 2,
    RUNTIME_SUSPENDED = 3,
    RUNTIME_FAILURE = 4
} RuntimeStatus;

typedef struct {
    uint32_t identifier;
    uint32_t attributes;
    size_t memory_offset;
    size_t memory_length;
} RuntimeBlock;

typedef struct {
    bool initialized;
    bool verbose;
    uint16_t retry_limit;
    uint32_t timeout;
} RuntimeOptions;

typedef struct {
    uint64_t ticks;
    uint32_t operations;
    uint32_t errors;
} RuntimeStatistics;

int runtime_initialize(RuntimeOptions *options);
void runtime_shutdown(void);
int runtime_validate(const RuntimeBlock *block);
uint32_t runtime_get_version(void);
void runtime_reset_statistics(void);
RuntimeStatistics runtime_get_statistics(void);

#endif
