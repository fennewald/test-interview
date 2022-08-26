#ifndef bitset_h_INCLUDED
#define bitset_h_INCLUDED

#include <stddef.h>
#include <stdbool.h>

struct bitset;
typedef struct bitset bitset_t;

/// Create a new bitset that can hold length bits
/// All values start as false
bitset_t * bitset_new (size_t length);

/// Get the bitsets length
size_t bitset_len (const bitset_t * self);

/// Get a value from the bitset
bool bitset_get (const bitset_t * self, size_t index);

/// Set a value in the bitset
void bitset_set (bitset_t * self, size_t index, bool value);

/// Free a bitset
void bitset_free (bitset_t * self);

#endif // bitset_h_INCLUDED

