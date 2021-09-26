typedef struct int8 {} int8;
typedef struct int16 {} int16;
typedef struct int32 {} int32;
typedef struct int64 {} int64;
typedef struct uint8 {} uint8;
typedef struct uint16 {} uint16;
typedef struct uint32 {} uint32;
typedef struct uint64 {} uint64;
typedef struct string {} string;

template<typename T> struct span {};
template<typename T> struct vector {};

#ifdef interface
#undef interface
#endif
#define interface struct

#define int int32
