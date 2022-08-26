/* sieve.c
 * By: Carson Riker
 * 
 * `sieve` is a python extension module that allows users to use a sieve of
 * eratostheneese. It exposes two types:
   * Sieve -- the actual sieve
   * SieveIter -- An iterator over that Sieve
 *
 * Internally, the sieve uses a bitset, defined in `src/bitset.c`. The full
 * interface with this bitset is defined in `src/bitset.h`.
 */

#include "bitset.h"
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

/* Sieve */
typedef struct sieve {
  PyObject_HEAD
  bitset_t * set;
} sieve_t;

// Prototypes
static PyObject * sieve_new (PyTypeObject * type, PyObject * args, PyObject * kwds);
static int sieve_init (sieve_t * self, PyObject * args, PyObject * kwds);
static void sieve_dealloc (sieve_t * self);
static PyObject * sieve_get (PyObject * self, PyObject * args);
static PyObject * sieve_set (PyObject * self, PyObject * args);
static PyObject * sieve_len (PyObject * self, PyObject * args);
static PyObject * sieve_filter (PyObject * self, PyObject * args);
static PyObject * sieve_iter (PyObject * self);

static PyMemberDef sieve_members[] = { {NULL} };
static PyMethodDef sieve_methods[] = {
  {"get", sieve_get, METH_VARARGS, "Check if an index is currently marked as composite"},
  {"set", sieve_set, METH_VARARGS, "Set an index as composite or prime"},
  {"len", sieve_len, METH_NOARGS, "Return the length of the sieve"},
  {"filter", sieve_filter, METH_VARARGS, "Run the sieve for all multiples of n"}, 
  {NULL}
};

static PyTypeObject sieve_type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "sieve.Sieve",
  .tp_doc = "a sieve of eratostheneese",
  .tp_basicsize = sizeof(sieve_t),
  .tp_itemsize = 0,
  .tp_flags = Py_TPFLAGS_DEFAULT,
  .tp_new = sieve_new,
  .tp_iter = sieve_iter,
  .tp_init = (initproc) sieve_init,
  .tp_dealloc = (destructor) sieve_dealloc,
  .tp_members = sieve_members,
  .tp_methods = sieve_methods,
};

static PyObject * sieve_new (PyTypeObject * type, PyObject * args, PyObject * kwds) {
  sieve_t * self;
  self = (sieve_t *) type->tp_alloc(type, 0);
  self->set = NULL;
  return (PyObject *) self;
}

static int sieve_init (sieve_t * self, PyObject * args, PyObject * kwds) {
  size_t length;

  if (!PyArg_ParseTuple(args, "O", &length))
    return -1;

  self->set = bitset_new(length);

  return 0;
}

static void sieve_dealloc (sieve_t * self) {
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * sieve_get (PyObject * self, PyObject * args) {
  size_t index;
  sieve_t * s_self = (sieve_t *) self;

  if (!PyArg_ParseTuple(args, "n", &index))
    return NULL;

  if (bitset_get(s_self->set, index)) {
    Py_RETURN_TRUE;
  } else {
    Py_RETURN_FALSE;
  }
}

static PyObject * sieve_set (PyObject * self, PyObject * args) {
  size_t index;
  int value;
  sieve_t * s_self = (sieve_t *) self;

  if (!PyArg_ParseTuple(args, "np", &index, &value))
    return NULL;

  bitset_set(s_self->set, index, value);

  Py_RETURN_NONE;
}

static PyObject * sieve_len (PyObject * self, PyObject *Py_UNUSED(ignored)) {
  sieve_t * s_self = (sieve_t *) self;
  return PyLong_FromLong(bitset_len(s_self->set));
}

static PyObject * sieve_filter (PyObject * py_self, PyObject * args) {
  size_t n, length;
  sieve_t * self = (sieve_t *) py_self;
  length = bitset_len(self->set);

  if (!PyArg_ParseTuple(args, "n", &n))
    return NULL;

  // If the value is already marked, then it's pointless to try and mark the
  // rest
  if (bitset_get(self->set, n)) {
    Py_RETURN_NONE;
  }

  for (size_t i=2*n; i<length; i += n) {
    bitset_set(self->set, i, true);
  }

  Py_RETURN_NONE;
}


/* Sieve Iterator */
typedef struct sieve_iter {
  PyObject_HEAD
  size_t index;
  sieve_t * root_obj;
} sieve_iter_t;

static PyObject * sieve_iter_new (PyTypeObject * type, PyObject * args, PyObject * kwds);
static void sieve_iter_dealloc (sieve_iter_t * self);
static PyObject * sieve_iter_next (sieve_iter_t * self);

static PyTypeObject sieve_iter_type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "sieve.SieveIter",
  .tp_doc = "an iterator over a sieve.Sieve",
  .tp_basicsize = sizeof(sieve_iter_t),
  .tp_itemsize = 0,
  .tp_flags = Py_TPFLAGS_DEFAULT,
  .tp_new = sieve_iter_new,
  .tp_dealloc = (destructor) sieve_iter_dealloc,
  .tp_iter = PyObject_SelfIter,
  .tp_iternext = (iternextfunc)sieve_iter_next,
};

static PyObject * sieve_iter_new (PyTypeObject * type, PyObject * args, PyObject * kwds) {
  sieve_iter_t * self;
  PyObject * py_root_obj;
  if (!PyArg_UnpackTuple(args, "o", 1, 1, &py_root_obj))
    return NULL;

  self = (sieve_iter_t *) type->tp_alloc(type, 0);
  Py_INCREF(py_root_obj);
  self->index = 0;
  self->root_obj = (sieve_t *) py_root_obj;
  return (PyObject *) self;
}

static void sieve_iter_dealloc (sieve_iter_t * self) {
  Py_DECREF(self->root_obj);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * sieve_iter_next (sieve_iter_t * self) {
  size_t index = self->index;
  size_t length = bitset_len(self->root_obj->set);

  while (bitset_get(self->root_obj->set, index)) {
    index ++;
    if (index >= length) {
      Py_CLEAR(self->root_obj);
      return NULL;
    }
  }
  self->index = index + 1;
  return PyLong_FromLong(index);
}

static PyObject * sieve_iter (PyObject * self) {
  sieve_iter_t * iterator = (sieve_iter_t *) sieve_iter_type.tp_alloc(&sieve_iter_type, 0);
  if (!iterator)
    return NULL;

  Py_INCREF(self);
  iterator->index = 0;
  iterator->root_obj = (sieve_t *)self;

  return (PyObject *) iterator;
}

static PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  .m_name = "sieve",
  .m_doc = "A module that does things",
  .m_size = -1,
};

PyMODINIT_FUNC PyInit_sieve (void) {
  PyObject * mod;

  if (PyType_Ready(&sieve_type) < 0)
    return NULL;

  if (PyType_Ready(&sieve_iter_type) < 0)
    return NULL;

  mod = PyModule_Create(&module);
  if (mod == NULL)
    return NULL;

  Py_INCREF(&sieve_type);
  if (PyModule_AddObject(mod, "Sieve", (PyObject *)&sieve_type) < 0) {
    Py_DECREF(&sieve_type);
    Py_DECREF(mod);
    return NULL;
  }

  Py_INCREF(&sieve_iter_type);
  if (PyModule_AddObject(mod, "SieveIter", (PyObject *)&sieve_iter_type) < 0) {
    Py_DECREF(&sieve_iter_type);
    Py_DECREF(&sieve_type);
    Py_DECREF(mod);
    return NULL;
  }

  return mod;
}
