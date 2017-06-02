#ifndef INCLUDE_LT_H
#define INCLUDE_LT_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// Make type names more consistent and easier to write.
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

typedef float       f32;
typedef double      f64;

typedef size_t      usize;
typedef ptrdiff_t   isize;

_Static_assert(sizeof(u8) == 1, "u8 should have 1 byte");
_Static_assert(sizeof(u16) == 2, "u16 should have 2 bytes");
_Static_assert(sizeof(u32) == 4, "u16 should have 4 bytes");
_Static_assert(sizeof(u64) == 8, "u16 should have 8 bytes");

_Static_assert(sizeof(i8) == 1, "i8 should have 1 byte");
_Static_assert(sizeof(i16) == 2, "i16 should have 2 bytes");
_Static_assert(sizeof(i32) == 4, "i16 should have 4 bytes");
_Static_assert(sizeof(i64) == 8, "i16 should have 8 bytes");

_Static_assert(sizeof(f32) == 4, "f32 should have 4 byte");
_Static_assert(sizeof(f64) == 8, "f64 should have 8 bytes");

/////////////////////////////////////////////////////////
//
// General Macros and Functions
//
// Macros that make working with c either better or more consistent.
//

#ifndef lt_global
#define lt_global        static  // Global variables
#endif
#ifndef lt_internal
#define lt_internal      static  // Internal linkage
#endif
#ifndef lt_local_persist
#define lt_local_persist static  // Local persisting variables
#endif

#ifndef LT_UNUSED
#define LT_UNUSED(x) ((void)(x))
#endif

#ifndef lt_min
#define lt_min(a, b) ((a) < (b) ? (a) : b)
#endif

#ifndef lt_max
#define lt_max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef lt_inside_open_interval
#define lt_inside_open_interval(x, min, max)  ((min) < (x) && (x) < (max))
#endif

#ifndef lt_in_closed_interval
#define lt_in_closed_interval(x, min, max)  ((min) <= (x) && (x) <= (max))
#endif

#ifndef LT_BIT
#define LT_BIT(x) (1<<(x))
#endif

#ifndef lt_free
#define lt_free(p) do { \
        free(p);        \
        p = NULL;       \
    } while(0)
#endif

#ifndef LT_FAIL
#  ifdef LT_DEBUG
#    define LT_FAIL(...) do {                                        \
        fprintf(stderr, "****** RUNTIME FAILURE ******\n");          \
        fprintf(stderr, "%s(line %d)", __FILE__, __LINE__);          \
        fprintf(stderr, __VA_ARGS__);                                \
        fprintf(stderr, "*****************************\n");          \
        fflush(stderr);                                              \
        abort();                                                     \
    } while(0)
#  else
#    define LT_FAIL(...)
#  endif // LT_DEBUG
#endif // LT_FAIL

#ifndef LT_ASSERT
#  ifdef LT_DEBUG
#    define LT_ASSERT2(cond, file, number) do {                             \
        if (!(cond)) {                                                  \
            fprintf(stderr, "******************************\n");        \
            fprintf(stderr, "*********** ASSERT ***********\n");        \
            fprintf(stderr, "**                            \n");        \
            fprintf(stderr, "**  Condition: %s\n", #cond);              \
            fprintf(stderr, "**  %s (line %d)\n", file, number);        \
            fprintf(stderr, "**                            \n");        \
            fprintf(stderr, "******************************\n");        \
            fprintf(stderr, "******************************\n");        \
            fflush(stderr);                                             \
            abort();                                                    \
        }                                                               \
    } while(0)
#    define LT_ASSERT(cond) LT_ASSERT2(cond, __FILE__, __LINE__)
#  else
#    define LT_ASSERT(cond)
#  endif // LT_DEBUG
#endif // LT_ASSERT

inline bool lt_is_little_endian() {
    i32 num = 1;
    if (*(char*)&num == 1)
        return true;
    else
        return false;
}

/////////////////////////////////////////////////////////
//
// String
//
// Basic string implementation. The string is a struct that wraps a c string.
// This decision is made for ease of implementation (Still suck at C).
//
typedef struct String {
    isize     len;
    isize     capacity;
    char     *data;
} String;

// String functions
String *string_make       (const char *cstr);
String *string_make_ptrs  (u8 *start, u8 *end);
String *string_make_length(i32 len);

void   string__append_str(String *a, const String *b);
void   string__append_cstr(String *a, const char *b);

String *string__concat_str(const String *a, const String *b);
String *string__concat_cstr(const char *a, const char *b);

#ifndef string_append
#define string_append(a, b) _Generic((b), \
                                     const String*: string__append_str, \
                                     const char*: string__append_cstr)(a, b)
#endif
#ifndef string_concat
#define string_concat(a, b) _Generic((a), \
                                     const String*: string__concat_str, \
                                     char*: string__concat_cstr,  \
                                     const char*: string__concat_cstr)(a, b)
#endif

i32    string_cmp        (const String *a, const String *b);
bool   string_eq         (const String *a, const String *b);
bool   string_empty      (const String *str);
void   string_free       (String *str);

typedef enum FileError {
    FileError_None,

    FileError_Read,
    FileError_Seek,
    FileError_NotExists,
    FileError_Unknown,

    FileError_Count,
} FileError;

typedef struct FileContents {
    void     *data;
    isize     size;
    FileError error;
} FileContents;

FileContents *file_read_contents(const char *filename);
void          file_free_contents(FileContents *fc);
isize         file_get_size(const char *filename);


/////////////////////////////////////////////////////////
//
// Array
//
// Heavily influenced by gb.h's Array (https://github.com/gingerBill/gb/blob/master/gb.h).
// TODO(leo): Add more functions necessary. API is still too small.
//

#define Array(Type) Type *

typedef struct ArrayHeader {
    isize length;
    isize capacity;
} ArrayHeader;

#ifndef ARRAY_GROW_FORMULA
#define ARRAY_GROW_FORMULA(x) (2*(x) + 8)
#endif

#define ARRAY_HEADER(x)   ((ArrayHeader*)(x) - 1)
#define array_length(x)   (ARRAY_HEADER(x)->length)
#define array_capacity(x) (ARRAY_HEADER(x)->capacity)
#define array_init_reserve(x, cap) do {                                 \
        void **lt__array = (void**)&(x);                                \
        ArrayHeader *lt__ah = (ArrayHeader*)malloc(sizeof(ArrayHeader) + sizeof(*(x))*(cap)); \
        lt__ah->length = 0;                                             \
        lt__ah->capacity = cap;                                         \
        *lt__array = (void*)(lt__ah+1);                                 \
    } while (0)

#define array_init(x) array_init_reserve(x, ARRAY_GROW_FORMULA(0));

#define array_free(x) do {                      \
        ArrayHeader *lt__ah = ARRAY_HEADER(x);  \
        free(lt__ah);                           \
    } while (0)

#define array_set_capacity(x, capacity) do {                            \
        if (x) {                                                        \
            void **lt__array_ = (void **)&(x);                          \
            *lt__array_ = lt__array_set_capacity((x), (capacity), sizeof(*(x))); \
        }                                                               \
    } while (0)

// TODO: Implement this function
void *lt__array_set_capacity(void *array, isize capacity, isize element_size);

#define array_grow(x, min_capacity) do {                            \
        isize new_capacity = ARRAY_GROW_FORMULA(array_capacity(x)); \
        if (new_capacity < min_capacity) {                          \
            new_capacity = min_capacity;                            \
        }                                                           \
        array_set_capacity(x, new_capacity);                        \
    } while (0)

#define array_append(x, item) do {                   \
        if (array_capacity(x) < array_length(x)+1)   \
            array_grow(x, 0);                        \
        (x)[array_length(x)++] = (item);             \
    } while (0)


/////////////////////////////////////////////////////////
//
// Vector
//
// Definition of a vector structure.
//
typedef union Vec3f {
    f32 vals[3];
    struct {
        f32 x, y, z;
    };
    struct {
        f32 r, g, b;
    };
} Vec3f;

typedef union Vec3i {
    i32 vals[3];
    struct {
        i32 x, y, z;
    };
    struct {
        i32 r, g, b;
    };
} Vec3i;

typedef union Vec4i {
    i32 vals[4];
    struct {
        i32 x, y, z, w;
    };
    struct {
        i32 r, g, b, a;
    };
} Vec4i;

Vec3i vec3i_add(const Vec3i *a, const Vec3i *b);
Vec3i vec3i_sub(const Vec3i *a, const Vec3i *b);
Vec3f vec3f_add(const Vec3f *a, const Vec3f *b);
Vec3f vec3f_sub(const Vec3f *a, const Vec3f *b);
Vec4i vec4i_add(const Vec4i *a, const Vec4i *b);
Vec4i vec4i_sub(const Vec4i *a, const Vec4i *b);

#ifndef vec_add
#define vec_add(a, b) _Generic((a),                     \
                               Vec3f*: vec3f_add(a,b), \
                               Vec3i*: vec3i_add(a,b), \
                               Vec4i*: vec4i_add(a,b))
#endif
#ifndef vec_sub
#define vec_sub(a, b) _Generic((a),                     \
                               Vec3f*: vec3f_sub(a,b), \
                               Vec3i*: vec3i_sub(a,b), \
                               Vec4i*: vec4i_sub(a,b))
#endif

#endif // INCLUDE_LT_H


/* -------------------------------------------------------------------------
 *
 *
 *
 *
 *
 *
 *  Implementation
 *
 *
 *
 *
 *
 *
 * ------------------------------------------------------------------------- */

#if defined(LT_IMPLEMENTATION) && !defined(LT_IMPLEMENTATION_DONE)
#define LT_IMPLEMENTATION_DONE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__unix__)
#include <sys/stat.h>
#endif

/////////////////////////////////////////////////////////
//
// String Implementation
//
bool string_empty(const String *str) {
    return str->len == 0;
}

String *string_make_length(i32 len) {
    // Add +1 to be compatible with c strings.
    char *data = calloc(len+1, sizeof(char));

    if (data == NULL) {
        LT_FAIL("Unable to allocate memory for string");
    }

    String *str = malloc(sizeof(*str));
    str->len = len;
    str->capacity = len+1;
    str->data = data;
    return str;
}

void string__allocate_space(String *a, isize new_capacity) {
    LT_ASSERT(new_capacity > a->len);

    char *new_data = realloc(a->data, new_capacity);

    if (new_data == NULL) {
        LT_FAIL("Failed to allocate memory for string");
    }
    // Zeroes the rest of the memory.
    memset(new_data+a->len, 0, new_capacity-a->len);

    a->capacity = new_capacity;
    a->data = new_data;
}

void string__append_str(String *a, const String *b) {
    if (b->len <= 0) { return; }

    isize new_len = a->len + b->len;

    if (new_len >= a->capacity) {
        // TODO(leo), @Speed: At the moment only a static increment in memory is used.
        // This is obviously not ideal nor fast, so it should be improved.
        const isize increase = 10;
        string__allocate_space(a, new_len+increase);
    }

    strncpy(a->data+a->len, b->data, b->len);
    a->len = new_len;
    // Assert that the data is null terminated.
    LT_ASSERT(a->data[new_len] == 0);
}

void string__append_cstr(String *a, const char *b) {
    isize blen = strlen(b);
    isize new_len = a->len + blen;

    if (new_len >= a->capacity) {
        // TODO(leo), @Speed: At the moment only a static increment in memory is used.
        // This is obviously not ideal nor fast, so it should be improved.
        const isize increase = 10;
        string__allocate_space(a, new_len+increase);
    }

    strncpy(a->data+a->len, b, blen);
    a->len = new_len;
    // Assert that the data is null terminated.
    LT_ASSERT(a->data[new_len] == 0);
}

String *string_make(const char* cstr) {
    usize len = strlen(cstr);
    usize capacity = len + 1;
    char *data = calloc(capacity, sizeof(char));
    if (data == NULL) {
        LT_FAIL("Unable to allocate memory for string.");
    }

    strncpy(data, cstr, len);

    String *str = malloc(sizeof(*str));
    str->len = len;
    str->capacity = capacity;
    str->data = data;
    return str;
}

String *string_make_ptrs(u8 *start, u8 *end) {
    LT_ASSERT(start != NULL && end != NULL);

    isize len = end - start + 1;
    isize capacity = len+1;
    LT_ASSERT(len >= 0);

    String *str = malloc(sizeof(*str));
    str->len = len;
    str->capacity = capacity;
    str->data = malloc(capacity);

    memcpy(str->data, start, len);
    str->data[len] = '\0';

    return str;
}

String *string__concat_str(const String *a, const String *b) {
    isize newlen = a->len + b->len;

    String *str = string_make_length(newlen);
    memcpy(str->data, a->data, a->len);
    memcpy(str->data, b->data, b->len);
    return str;
}

String *string__concat_cstr(const char *a, const char *b) {
    isize alen = strlen(a);
    isize blen = strlen(b);

    String *str = string_make_length(alen + blen);
    memcpy(str->data, a, alen);
    memcpy(str->data+alen, b, blen);
    return str;
}

i32 string_cmp(const String *a, const String *b) {
    return strcmp(a->data, b->data);
}

bool string_eq(const String *a, const String *b) {
    return string_cmp(a, b) == 0;
}

void string_free(String *str) {
    lt_free(str->data);
    lt_free(str);
}

/////////////////////////////////////////////////////////
//
// File Implementation
//
FileContents *file_read_contents(const char *filename) {
    FILE *fp = fopen(filename, "r");
    void *file_data = NULL;

    // TODO(leo), @Robustness: Actually check if the file exists before trying to open it.
    if (fp == NULL) {
        FileContents *ret = malloc(sizeof(*ret));
        ret->error = FileError_NotExists;
        ret->data = NULL;
        ret->size = -1;
        return ret;
    }

    isize file_size = file_get_size(filename);

    if (file_size == -1) {
        fclose(fp);
        FileContents *ret = malloc(sizeof(*ret));
        ret->error = FileError_Unknown;
        ret->data = NULL;
        ret->size = -1;
        return ret;
    }

    file_data = malloc(sizeof(char) * file_size);

    if (file_data == NULL) {
        LT_FAIL("Failed allocating memory\n");
    }

    isize newlen = fread(file_data, sizeof(u8), file_size, fp);
    if (newlen < 0) {
        FileContents *ret = malloc(sizeof(*ret));
        ret->error = FileError_Read;
        ret->data = NULL;
        ret->size = -1;
        return ret;
    }

    LT_ASSERT(newlen == file_size);

    if (ferror(fp) != 0) {
        fputs("Error reading file\n", stderr);
        fclose(fp);
        free(file_data);

        FileContents *ret = malloc(sizeof(*ret));
        ret->error = FileError_Read;
        ret->data = NULL;
        ret->size = -1;
        return ret;
    }
    fclose(fp);

    FileContents *ret = malloc(sizeof(*ret));
    ret->error = FileError_None;
    ret->data = file_data;
    ret->size = file_size;
    return ret;
}

void file_free_contents(FileContents *fc) {
    lt_free(fc->data);
    lt_free(fc);
}

isize file_get_size(const char *filename) {
#if defined(__unix__)
    struct stat st;
    if (stat(filename, &st) < 0) {
        return -1;
    }
    return st.st_size;
#else
    LT_FAIL("Still not implemented\n");
#endif
}

void *lt__array_set_capacity(void *array, isize capacity, isize element_size) {
    ArrayHeader *ah = ARRAY_HEADER(array);

    LT_ASSERT(element_size > 0);

    if (capacity == ah->capacity) {
        return array;
    }

    if (capacity < ah->length) {
        if (ah->capacity < capacity) {
            isize new_capacity = ARRAY_GROW_FORMULA(ah->capacity);
            if (new_capacity < capacity) {
                new_capacity = capacity;
            }
            lt__array_set_capacity(array, new_capacity, element_size);
        }
        ah->length = capacity;
    }

    {
        isize size = sizeof(ArrayHeader) + element_size*capacity;
        ArrayHeader *nh = malloc(size);
        memcpy(nh, ah, sizeof(ArrayHeader) + element_size*ah->length);
        nh->length = ah->length;
        nh->capacity = capacity;
        free(ah);
        return nh+1;
    }
}

/////////////////////////////////////////////////////////
//
// Vector implementation
//
Vec3f vec3f_add(const Vec3f *a, const Vec3f *b) {
    Vec3f r;
    r.x = a->x + b->x;
    r.y = a->y + b->y;
    r.z = a->z + b->z;
    return r;
}

Vec3f vec3f_sub(const Vec3f *a, const Vec3f *b) {
    Vec3f r;
    r.x = a->x - b->x;
    r.y = a->y - b->y;
    r.z = a->z - b->z;
    return r;
}

Vec3i vec3i_add(const Vec3i *a, const Vec3i *b) {
    Vec3i r;
    r.x = a->x + b->x;
    r.y = a->y + b->y;
    r.z = a->z + b->z;
    return r;
}

Vec3i vec3i_sub(const Vec3i *a, const Vec3i *b) {
    Vec3i r;
    r.x = a->x - b->x;
    r.y = a->y - b->y;
    r.z = a->z - b->z;
    return r;
}

Vec4i vec4i_add(const Vec4i *a, const Vec4i *b) {
    Vec4i r;
    r.x = a->x + b->x;
    r.y = a->y + b->y;
    r.z = a->z + b->z;
    r.w = a->w + b->w;
    return r;
}

Vec4i vec4i_sub(const Vec4i *a, const Vec4i *b) {
    Vec4i r;
    r.x = a->x - b->x;
    r.y = a->y - b->y;
    r.z = a->z - b->z;
    r.w = a->w - b->w;
    return r;
}

#endif // LT_IMPLEMENTATION
