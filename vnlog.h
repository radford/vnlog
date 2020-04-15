#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef VNLOG_N_FIELDS

  // We know how many fields we have. This is #included from a header generated by
  // vnl-gen-header
  #define vnlog_emit_record_ctx(ctx)     _vnlog_emit_record   (ctx,      VNLOG_N_FIELDS)
  #define vnlog_emit_record()            _vnlog_emit_record   (NULL,     VNLOG_N_FIELDS)
  #define vnlog_init_session_ctx(ctx)    _vnlog_init_session_ctx (ctx,   VNLOG_N_FIELDS)
  #define vnlog_init_child_ctx(dst, src) _vnlog_init_child_ctx(dst, src, VNLOG_N_FIELDS)
  #define vnlog_printf(...)              _vnlog_printf        (NULL,     VNLOG_N_FIELDS, ## __VA_ARGS__)
  #define vnlog_printf_ctx(ctx, ...)     _vnlog_printf        (ctx,      VNLOG_N_FIELDS, ## __VA_ARGS__)
  #define vnlog_flush()                  _vnlog_flush         (NULL,     VNLOG_N_FIELDS)
  #define vnlog_flush_ctx(ctx)           _vnlog_flush         (ctx,      VNLOG_N_FIELDS)
  #define vnlog_free_ctx(ctx)            _vnlog_free_ctx      (ctx,      VNLOG_N_FIELDS)
  #define vnlog_set_output_FILE(ctx,fp)  _vnlog_set_output_FILE(ctx, fp, VNLOG_N_FIELDS)

#else

  // We don't know how many fields we have. This is #included from vnlog.c

  #ifndef VNLOG_C
  #error Please do not include vnlog.h directly. Instead include the header made by vnl-gen-header
  #endif

#endif


/*
This is an interface to produce vnlog output from C programs. Common usage:

  In a shell:

    vnl-gen-header 'int w' 'uint8_t x' 'char* y' 'double z' > vnlog_fields_generated.h

  In a C program test.c:

    #include "vnlog_fields_generated.h"

    int main()
    {
        vnlog_emit_legend();

        vnlog_set_field_value__w(-10);
        vnlog_set_field_value__x(40);
        vnlog_set_field_value__y("asdf");
        vnlog_emit_record();

        vnlog_set_field_value__z(0.3);
        vnlog_set_field_value__x(50);
        vnlog_set_field_value__w(-20);
        vnlog_set_field_value__binary("\x01\x02\x03", 3);
        vnlog_emit_record();

        vnlog_set_field_value__w(-30);
        vnlog_set_field_value__x(10);
        vnlog_set_field_value__y("whoa");
        vnlog_set_field_value__z(0.5);
        vnlog_emit_record();

        return 0;
    }


  $ cc -o test test.c -lvnlog

  $ ./test

  # w x y z binary
  -10 40 asdf - -
  -20 50 - 0.2999999999999999889 AQID
  -30 10 whoa 0.5 -


Note that THIS FILE IS NOT MEANT TO BE #include-ed BY THE USER. IT SHOULD BE
INCLUDED BY THE GENERATED vnlog_fields_generated.h
 */


#define VNLOG_MAX_FIELD_LEN 32

typedef struct
{
    char  c[VNLOG_MAX_FIELD_LEN];
    void* binptr;               // If non-NULL, we have binary data instead of ascii data in c[]
    int   binlen;
} vnlog_field_t;

// If we're building the LIBRARY, we don't know how many fields we'll have. The
// library never assumes it knows this: it never takes sizeof(struct
// vnlog_context_t, instantiates a new context and so on)
//
// Brand-new contexts are filled with 0, so make sure that all elements should
// start out at 0
struct vnlog_context_t
{
    // The root context. The values shared by all contexts in this session are
    // accessed via the root. It is slightly wasteful for all contexts to carry
    // the (unreliable) data even if they're not the root, but it makes the API
    // simpler
    struct vnlog_context_t* root;

    // global state for this whole session. These should be accessed ONLY
    // through the root context.
    FILE*            _fp;
    bool             _emitted_something   : 1;
    bool             _legend_finished     : 1;

    // Each context manages its own set of fields. These could be different in
    // each context instance
    bool             line_has_any_values : 1;

    vnlog_field_t fields[
#ifdef VNLOG_N_FIELDS
                            VNLOG_N_FIELDS
#endif
                            ];
};


#ifdef __cplusplus
extern "C" {
#endif

// THIS FUNCTION IS NOT A PART OF THE PUBLIC API. The user should call
//
//     vnlog_set_output_FILE()
//
// Directs the output to a given buffer. If this function is never called, the
// output goes to STDOUT. If it IS called, that must happen before everything
// else. Pass ctx==NULL to set the global context
void _vnlog_set_output_FILE(struct vnlog_context_t* ctx,
                            FILE* _fp,
                            int Nfields);

// THIS FUNCTION IS NOT A PART OF THE PUBLIC API. The user should call
//
//     vnlog_emit_legend()
//
// The header generated by vnl-gen-header converts one call to the other.
// This is called once to write out the legend. Must be called before any data
// can be written
void _vnlog_emit_legend(struct vnlog_context_t* ctx,
                        const char* legend, int Nfields);

// THESE FUNCTIONS ARE NOT A PART OF THE PUBLIC API. Instead the user should call
// either of
//
//     vnlog_set_field_value__FIELDNAME(value)
//     vnlog_set_field_value_ctx__FIELDNAME(ctx, value)
//
// depending on whether they want to use the default context or not. The header
// generated by vnl-gen-header converts one call to the other.

#define VNLOG_TYPES(_)                                                  \
    _(int,          int,         "%d")                                  \
    _(int8_t,       int8_t,      "%" PRId8)                             \
    _(int16_t,      int16_t,     "%" PRId16)                            \
    _(int32_t,      int32_t,     "%" PRId32)                            \
    _(int64_t,      int64_t,     "%" PRId64)                            \
    _(unsigned,     unsigned,    "%u")                                  \
    _(unsigned int, unsignedint, "%u")                                  \
    _(uint8_t,      uint8_t,     "%" PRIu8)                             \
    _(uint16_t,     uint16_t,    "%" PRIu16)                            \
    _(uint32_t,     uint32_t,    "%" PRIu32)                            \
    _(uint64_t,     uint64_t,    "%" PRIu64)                            \
    _(char,         char,        "%c")                                  \
    _(float,        float,       "%.20g") /* should fit into a VNLOG_MAX_FIELD_LEN */ \
    _(double,       double,      "%.20g")                               \
    _(char*,        charp,       "%s")                                  \
    _(const char*,  ccharp,      "%s")

#define DECLARE_SET_FIELD_FUNCTION(type, typename, fmt)                 \
void                                                                    \
_vnlog_set_field_value_ ## typename(struct vnlog_context_t* ctx,        \
                                    const char* fieldname, int idx,     \
                                    type arg);
VNLOG_TYPES( DECLARE_SET_FIELD_FUNCTION )
#undef DECLARE_SET_FIELD_FUNCTION

void
_vnlog_set_field_value_binary(struct vnlog_context_t* ctx,
                              const char* fieldname, int idx,
                              const void* data, int len);

// THIS FUNCTION IS NOT A PART OF THE PUBLIC API. Instead, the user should call
// either of
//
//     vnlog_emit_record()
//     vnlog_emit_record_ctx(ctx)
//
// depending on whether they want to use the default context or not. Once all
// the fields for a record have been set with
// vnlog_set_field_value__FIELDNAME(), this function is called to emit the
// record. Any fields not set get written as -.
//
// This function is thread-safe, and multiple context can be safely written out
// from multiple threads
void _vnlog_emit_record(struct vnlog_context_t* ctx,
                        int Nfields);

// THIS FUNCTION IS NOT A PART OF THE PUBLIC API. Instead, the user should call
// either of
//
//     vnlog_printf(...)
//     vnlog_printf_ctx(ctx, ...)
//
// depending on whether they want to use the default context or not. Writes out
// the given printf-style format to the vnlog. Generally this is a comment
// string, so it should start with a '#' and end in a '\n', but I do not check
// or enforce this.
void _vnlog_printf(struct vnlog_context_t* ctx, int Nfields,
                   const char* fmt, ...);

// THIS FUNCTION IS NOT A PART OF THE PUBLIC API. Instead, the user should call
// either of
//
//     vnlog_flush()
//     vnlog_flush_ctx(ctx)
//
// depending on whether they want to use the default context or not. Flushes the
// output buffer. Useful in conjunction with vnlog_printf()
void _vnlog_flush(struct vnlog_context_t* ctx, int Nfields);

// THIS FUNCTION IS NOT A PART OF THE PUBLIC API. Instead, the user should call
//
//     vnlog_init_child_ctx(ctx_dst, ctx_src)
//
// This function is used for sub-tables in an vnlog. I.e. we can accumulate
// multiple records at the same time. All the fields in any record are written
// all at once when vnlog_emit_record_ctx() is called for the context of each
// record. This is used for different records in the SAME vnlog. If you want
// a completely different vnlog, see vnlog_init_session_ctx(). The idiom
// is:
//
//     void f(void)
//     {
//         vnlog_emit_legend();
//
//         vnlog_set_field_value__x(...);
//         // we just set some fields in this record, and in the middle of filling
//         // this record we write other records
//         {
//             struct vnlog_context_t ctx;
//             vnlog_init_child_ctx(&ctx, NULL); // child of the global context
//             for(...)
//             {
//                 vnlog_set_field_value_ctx__y(&ctx, ...);
//                 ...
//                 vnlog_emit_record_ctx(&ctx);
//             }
//         }
//
//         // Now we resume the previous record. We still remember the value of x
//         vnlog_set_field_value__z(...);
//         vnlog_emit_record();
//     }
//
// ctx_from is the context from which we pull information global to this
// vnlog session. This is things like the FILE we're writing data to. If
// NULL, we use the global context.
void _vnlog_init_child_ctx( struct vnlog_context_t* ctx,
                            const struct vnlog_context_t* ctx_src,
                            int Nfields);

// THIS FUNCTION IS NOT A PART OF THE PUBLIC API. Instead, the user should call
//
//     vnlog_init_session_ctx(ctx)
//
// This function is used to initialize a new context to start a new vnlog
// session. MOST USES OF VNLOG DO NOT NEED TO USE THIS FUNCTION. Different
// sessions represent completely different vnlog data files. These can have
// different legends, and should send their output to different places using
// vnlog_set_output_FILE(). Since the legends are defined at compile-time,
// the code using each session MUST live in separate source files.
//
// The idiom is:
//
//   file1.c:
//
//     #include "vnlog_fields_generated1.h"
//     void f(void)
//     {
//         // Write some data out to the default context and default output (STDOUT)
//         vnlog_emit_legend();
//         ...
//         vnlog_set_field_value__w(...);
//         vnlog_set_field_value__x(...);
//         ...
//         vnlog_emit_record();
//     }
//
//   file2.c:
//
//     #include "vnlog_fields_generated2.h"
//     void g(void)
//     {
//         // Make a new session context, send output to a different file, write
//         // out legend, and send out the data
//         struct vnlog_context_t ctx;
//         vnlog_init_session_ctx(&ctx);
//
//         FILE* fp = fopen(...);
//         vnlog_set_output_FILE(&ctx, fp);
//
//         vnlog_emit_legend_ctx(&ctx);
//         ...
//         vnlog_set_field_value__a(...);
//         vnlog_set_field_value__b(...);
//         ...
//         vnlog_emit_record();
//     }
void _vnlog_init_session_ctx( struct vnlog_context_t* ctx,
                              int Nfields);

// THIS FUNCTION IS NOT A PART OF THE PUBLIC API. Instead, the user should call
//
//     vnlog_clear_fields_ctx(ctx, do_free_binary)
//
// Clears out the data in a context and makes it ready to be used for the next
// record. It is rare for the user to have to call this manually. The most
// common case is handled automatically (clearing out a context after emitting a
// record). One area where this is useful is when making a copy of a context:
//
//     struct vnlog_context_t ctx1;
//     .... do stuff with ctx1 ... add data to it ...
//
//     struct vnlog_context_t ctx2 = ctx1;
//     // ctx1 and ctx2 now both have the same data, and the same pointers to
//     // binary data. I need to get rid of the pointer references in ctx1
//
//     vnlog_clear_fields_ctx(&ctx1, false);
void _vnlog_clear_fields_ctx(struct vnlog_context_t* ctx, int Nfields, bool do_free_binary);

// THIS FUNCTION IS NOT A PART OF THE PUBLIC API. Instead, the user should call
//
//     vnlog_free_ctx(ctx)
//
// Frees memory for an vnlog context. Do this before throwing the context
// away. Currently this is only needed for context that have binary fields, but
// this should be called in for all contexts, just in case
void _vnlog_free_ctx( struct vnlog_context_t* ctx, int Nfields );

#ifdef __cplusplus
}
#endif

