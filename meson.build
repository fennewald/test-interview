project('sieve', 'c', version: '1.0')

# Bitset library
bitset_inc = include_directories('src')
bitset_lib = library('bitset', ['src/bitset.h', 'src/bitset.c'])
bitset_dep = declare_dependency(include_directories: bitset_inc, link_with: bitset_lib)

# Python module
py3_mod = import('python3')
py3_dep = dependency('python3', version: '>=3.8')
harp_lib = py3_mod.extension_module('sieve', 'src/sieve.c', dependencies: [py3_dep, bitset_dep])

