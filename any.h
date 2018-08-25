#ifndef ANY_H
#define ANY_H

#include <stdint.h>

/* #define ANY_NO_BOXING */
/* #define ANY_NAN_BOXING */

#if defined(ANY_NO_BOXING) && defined(ANY_NAN_BOXING)
# error more than one switch specified
#endif

#if ! (defined(ANY_NO_BOXING) || defined(ANY_NAN_BOXING))
# if __x86_64__ && (defined(__GNUC__) || defined(__clang__))
#  define ANY_NAN_BOXING
# else
#  define ANY_NO_BOXING
# endif
#endif

typedef int32_t any_int_t; // or int64_t
typedef double any_float_t; // or float or N/A

typedef struct any any_t;

/*
typedef enum {
    ANY_FLOAT,
    ANY_TAG0,
    ANY_TAG1,
    ANY_TAG2,
    ANY_TAG3,
    ANY_TAG4,
} any_tag_t;
static inline any_tag_t any_tag(any_t a);
static inline any_t float_any(any_float_t f);
static inline any_float_t any_float(any_t a);
static inline any_t ptr_any(any_tag_t tag, void *p);
static inline void *any_ptr(any_t a);
static inline any_t int_any(any_tag_t tag, any_int_t i);
static inline any_int_t any_int(any_t a);
*/

#if defined(ANY_NAN_BOXING)

/**
 *   double : 0000 0000 0000 0000 ... 7FEF FFFF FFFF FFFF
 *   +inf   : 7FF0 0000 0000 0000
 *     -    : 7FF0 0000 0000 0001 ... 7FF7 FFFF FFFF FFFF
 *   +qNaN  : 7FF8 0000 0000 0000
 *     -    : 7FF8 0000 0000 0001 ... 7FFF FFFF FFFF FFFF
 *   double : 8000 0000 0000 0000 ... FFEF FFFF FFFF FFFF
 *   -inf   : FFF0 0000 0000 0000
 *     -    : FFF0 0000 0000 0001 ... FFFF FFFF FFFF FFFF
 */

typedef unsigned short any_tag_t;

#define ANY_FLOAT 0x0000u
#define ANY_TAG0  0x7FF1u
#define ANY_TAG1  0x7FF2u
#define ANY_TAG2  0x7FF3u
#define ANY_TAG3  0x7FF4u
#define ANY_TAG4  0x7FF5u

struct any {
    uint64_t v;
};

static inline any_tag_t any_tag(any_t a) {
    if (a.v <= 0x7FF0000000000000ul
        || a.v == 0x7ff8000000000000ul
        || (0x8000000000000000ul <= a.v && a.v <= 0xFFF0000000000000ul)) {
        return ANY_FLOAT;
    }
    return (a.v & 0xFFFF000000000000ul) >> 48;
}

static inline any_t float_any(any_float_t f) {
    union { double f; any_t a; } u;
    if (f != f) {
        return (any_t) { 0x7ff8000000000000ul };
    }
    u.f = f;
    return u.a;
}

static inline any_float_t any_float(any_t a) {
    union { double f; any_t a; } u;
    u.a = a;
    return u.f;
}

static inline any_t ptr_any(any_tag_t tag, void *ptr) {
    return (any_t) { (uint64_t) tag << 48 | ((uint64_t) ptr & 0xFFFFFFFFFFFFul) };
}

static inline void *any_ptr(any_t a) {
    uint64_t p = a.v & 0x0000FFFFFFFFFFFFul;
    if (p & 0x800000000000ul) {
        p |= 0xFFFF000000000000ul;
    }
    return (void *) p;
}

static inline any_t int_any(any_tag_t tag, any_int_t i) {
    return (any_t) { (uint64_t) tag << 48 | ((uint64_t) i & 0xFFFFFFFFFFFFul) };
}

static inline any_int_t any_int(any_t a) {
    return a.v & 0x0000FFFFFFFFFFFFul;
}

#elif defined(ANY_NO_BOXING)

typedef unsigned char any_tag_t;

#define ANY_FLOAT 1
#define ANY_TAG0  2
#define ANY_TAG1  3
#define ANY_TAG2  4
#define ANY_TAG3  5
#define ANY_TAG4  6

struct any {
    union {
        any_float_t f;
        any_int_t i;
        void *p;
    } payload;
    any_tag_t tag;
};

static inline any_tag_t any_tag(any_t a) {
    return a.tag;
}

static inline any_t float_any(any_float_t f) {
    return (struct any) { .payload.f = f, .tag = ANY_FLOAT };
}

static inline any_float_t any_double(any_t a) {
    return a.payload.f;
}

static inline any_t ptr_any(any_tag_t tag, void *p) {
    return (struct any) { .payload.p = p, .tag = tag };
}

static inline void *any_ptr(any_t a) {
    return a.payload.p;
}

static inline any_t int_any(any_tag_t tag, any_int_t i) {
    return (struct any) { .payload.i = i, .tag = tag };
}

static inline any_int_t any_int(any_t a) {
    return a.payload.i;
}

#endif

#endif
