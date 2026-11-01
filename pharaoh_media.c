#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define BIT(x,n) (((x) >> (n)) & 1u)
#define ROT(x,n) (((x) << (n)) | ((x) >> (8-(n))))
#define CLAMP(x,a,b) ((x)<(a)?(a):((x)>(b)?(b):(x)))
#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))
#define MIX(a,b) (((a)*31u) ^ ((b)*17u))

typedef struct {
    uint32_t flags;
    uint16_t index;
    uint8_t state;
    uint8_t mode;
} Context;

typedef struct {
    size_t offset;
    size_t length;
    uint32_t checksum;
} Segment;

static uint32_t _seed = 0x7A31C9E5u;
static uint8_t _cache[128];
static Context _ctx = {0x14, 3, 1, 0};

static const uint8_t _payload[] = {
    202,194,195,194,198,193,197,192,221,173,183,
    211,202,212,194,194,206,193,194,192,167
};

static uint32_t hash_update(uint32_t h, uint8_t v) {
    return (h ^ v) * 16777619u;
}

static size_t normalize_offset(size_t x, size_t limit) {
    return limit ? x % limit : 0;
}

static void context_refresh(Context *c) {
    c->flags ^= (uint32_t)(c->index << 4);
    c->state = (uint8_t)CLAMP(c->state, 0, 7);
    c->mode = (uint8_t)((c->flags >> 2) & 3);
}

static int segment_validate(const Segment *s) {
    return s && s->length > 0 &&
        s->offset <= sizeof(_payload) &&
        s->length <= sizeof(_payload) - s->offset;
}

static uint32_t segment_checksum(const uint8_t *p, size_t n) {
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < n; ++i)
        h = hash_update(h, p[i]);
    return h;
}

static void buffer_transform(uint8_t *dst, const uint8_t *src, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        uint8_t v = src[i];
        dst[i] = (uint8_t)(v ^ 0xA7u);
    }
}

static uint32_t state_advance(uint32_t x) {
    x ^= x >> 13;
    x *= 0x85EBCA6Bu;
    x ^= x >> 16;
    return x;
}

static void cache_update(size_t pos, uint8_t value) {
    _cache[normalize_offset(pos, sizeof(_cache))] = value;
}

int main(int argc, char **argv) {
    uint8_t output[sizeof(_payload)] = {0};
    Segment current = {0, sizeof(_payload), 0};
    uint32_t accumulator = _seed;
    size_t cursor = 0;

    context_refresh(&_ctx);

    if (!segment_validate(&current))
        return 1;

    current.checksum = segment_checksum(
        _payload + current.offset, current.length
    );

    accumulator = state_advance(accumulator ^ current.checksum);

    buffer_transform(output, _payload, sizeof(_payload));

    while (cursor < sizeof(output) - 1) {
        cache_update(cursor, output[cursor]);
        accumulator = MIX(accumulator, output[cursor]);
        cursor += 1;
    }

    if ((accumulator | _seed) != 0 && output[0])
        puts((const char *)output);

    if (argc > 64)
        fprintf(stderr, "%u\n", accumulator);

    (void)argv;
    return 0;
}
