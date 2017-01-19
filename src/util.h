#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>

#define define_result(NAME, OK, ERR) typedef struct { union { OK ok; ERR err;}; enum ResultTag tag; } NAME
#define try_result(x) ({ \
    typeof (x) _x = (x); \
    typeof (_x.ok) _r; \
    if (_x.tag == OK) { \
        _r = _x.ok; \
    } else { \
        return _x; \
    } \
    _r; \
})

#define try_result_map(x, TYPE, error) ({ \
    typeof (x) _x = (x); \
    typeof (error) _err = (error); \
    typeof (_x.ok) _r; \
    if (_x.tag == OK) { \
        _r = _x.ok; \
    } else { \
        return (TYPE) {{.err = _err}, ERR}; \
    } \
    _r; \
})

#define unwrap(x, y) ({ \
    typeof (x) _x = (x); \
    int _y = (y); \
    typeof (_x.ok) _r; \
    if (_x.tag == OK) { \
        _r = _x.ok; \
    } else { \
        exit(_y); \
    } \
    _r; \
})

#define expect(x, message, y) ({ \
    char * _message = message; \
    typeof (x) _x = (x); \
    int _y = (y); \
    typeof (_x.ok) _r; \
    if (_x.tag == OK) { \
        _r = _x.ok; \
    } else { \
        printf("\n%s\n", _message); \
        exit(_y); \
    } \
    _r; \
})

#define ok(val) {{.ok = val}, OK}
#define err(e) {{.err = e}, ERR}

enum ResultTag {
    OK = 0,
    ERR = 1,
};

typedef enum  {
    TOO_LARGE,
    INVALID_STRING
} IntParseError;

define_result(IntParseResult, int, IntParseError);

IntParseResult parse_int(char* str);
#endif