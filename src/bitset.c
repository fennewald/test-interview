#include "bitset.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

struct bitset {
  size_t length;
  uint8_t * data;
};

static inline void check_index (const bitset_t * self, size_t index) {
  if (index > self->length) {
    fprintf(stderr, "Error, tried to read index %ld of bitset with a length of %ld\n", index, self->length);
    exit(EXIT_FAILURE);
  }
}

bitset_t * bitset_new (size_t length) {
  size_t n_bytes = (length + 7) / 8;
  bitset_t * set = malloc(sizeof(bitset_t));
  set->data = malloc(n_bytes * sizeof(uint8_t));
  set->length = length;
  return set;
}

size_t bitset_len (const bitset_t * self) {
  return self.length;
}

bool bitset_get (const bitset_t * self, size_t index) {
  check_index(self, index);
  size_t byte_index = index >> 3;
  size_t bit_index = index & 0b111;

  uint8_t byte = self->data[byte_index];

  return (byte >> bit_index) & 1;
}

void bitset_set (bitset_t * self, size_t index, bool value) {
  check_index(self, index);
  size_t byte_index = index >> 3;
  size_t bit_index = index & 0b111;

  uint8_t mask = 1 << bit_index;

  if (value) {
    self->data[byte_index] |= mask;
  } else {
    self->data[byte_index] &= ~mask;
  }
}

void bitset_free (bitset_t * self) {
  free(self);
  free(self->data);
}
