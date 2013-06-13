/* createrepo_c - Library of routines for manipulation with repodata
 * Copyright (C) 2013  Tomas Mlcoch
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#include <Python.h>
#include <assert.h>
#include <stddef.h>

#include "contentstat-py.h"
#include "exception-py.h"
#include "typeconversion.h"

typedef struct {
    PyObject_HEAD
    cr_ContentStat *stat;
} _ContentStatObject;

cr_ContentStat *
ContentStat_FromPyObject(PyObject *o)
{
    if (!ContentStatObject_Check(o)) {
        PyErr_SetString(PyExc_TypeError, "Expected a ContentStat object.");
        return NULL;
    }
    return ((_ContentStatObject *)o)->stat;
}

static int
check_ContentStatStatus(const _ContentStatObject *self)
{
    assert(self != NULL);
    assert(ContentStatObject_Check(self));
    if (self->stat == NULL) {
        PyErr_SetString(CrErr_Exception, "Improper createrepo_c ContentStat object.");
        return -1;
    }
    return 0;
}

/* Function on the type */

static PyObject *
contentstat_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    CR_UNUSED(args);
    CR_UNUSED(kwds);
    _ContentStatObject *self = (_ContentStatObject *)type->tp_alloc(type, 0);
    if (self)
        self->stat = NULL;
    return (PyObject *)self;
}

static int
contentstat_init(_ContentStatObject *self, PyObject *args, PyObject *kwds)
{
    CR_UNUSED(kwds);

    int type;
    GError *tmp_err = NULL;

    if (!PyArg_ParseTuple(args, "i:contentstat_init", &type))
        return -1;

    /* Free all previous resources when reinitialization */
    if (self->stat)
        cr_contentstat_free(self->stat, NULL);

    /* Init */
    self->stat = cr_contentstat_new(type, &tmp_err);
    if (tmp_err) {
        PyErr_Format(CrErr_Exception, "ContentStat initialization failed: %s",
                     tmp_err->message);
        g_error_free(tmp_err);
        return -1;
    }

    return 0;
}

static void
contentstat_dealloc(_ContentStatObject *self)
{
    if (self->stat)
        cr_contentstat_free(self->stat, NULL);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *
contentstat_repr(_ContentStatObject *self)
{
    CR_UNUSED(self);
    return PyString_FromFormat("<createrepo_c.ContentStat object>");
}

/* getsetters */

#define OFFSET(member) (void *) offsetof(cr_ContentStat, member)

static PyObject *
get_num(_ContentStatObject *self, void *member_offset)
{
    if (check_ContentStatStatus(self))
        return NULL;
    cr_ContentStat *rec = self->stat;
    gint64 val = *((gint64 *) ((size_t)rec + (size_t) member_offset));
    return PyLong_FromLongLong((long long) val);
}

static PyObject *
get_int(_ContentStatObject *self, void *member_offset)
{
    if (check_ContentStatStatus(self))
        return NULL;
    cr_ContentStat *rec = self->stat;
    gint64 val = *((int *) ((size_t)rec + (size_t) member_offset));
    return PyLong_FromLongLong((long long) val);
}

static PyObject *
get_str(_ContentStatObject *self, void *member_offset)
{
    if (check_ContentStatStatus(self))
        return NULL;
    cr_ContentStat *rec = self->stat;
    char *str = *((char **) ((size_t) rec + (size_t) member_offset));
    if (str == NULL)
        Py_RETURN_NONE;
    return PyString_FromString(str);
}

static int
set_num(_ContentStatObject *self, PyObject *value, void *member_offset)
{
    gint64 val;
    if (check_ContentStatStatus(self))
        return -1;
    if (PyLong_Check(value)) {
        val = (gint64) PyLong_AsLong(value);
    } else if (PyInt_Check(value)) {
        val = (gint64) PyInt_AS_LONG(value);
    } else {
        PyErr_SetString(PyExc_ValueError, "Number expected!");
        return -1;
    }
    cr_ContentStat *rec = self->stat;
    *((gint64 *) ((size_t) rec + (size_t) member_offset)) = val;
    return 0;
}

static int
set_int(_ContentStatObject *self, PyObject *value, void *member_offset)
{
    long val;
    if (check_ContentStatStatus(self))
        return -1;
    if (PyLong_Check(value)) {
        val = PyLong_AsLong(value);
    } else if (PyInt_Check(value)) {
        val = PyInt_AS_LONG(value);
    } else {
        PyErr_SetString(PyExc_ValueError, "Number expected!");
        return -1;
    }
    cr_ContentStat *rec = self->stat;
    *((int *) ((size_t) rec + (size_t) member_offset)) = (int) val;
    return 0;
}

static int
set_str(_ContentStatObject *self, PyObject *value, void *member_offset)
{
    if (check_ContentStatStatus(self))
        return -1;
    if (!PyString_Check(value) && value != Py_None) {
        PyErr_SetString(PyExc_ValueError, "String or None expected!");
        return -1;
    }
    cr_ContentStat *rec = self->stat;
    char *str = g_strdup(PyObject_ToStrOrNull(value));
    *((char **) ((size_t) rec + (size_t) member_offset)) = str;
    return 0;
}

static PyGetSetDef contentstat_getsetters[] = {
    {"size",            (getter)get_num, (setter)set_num, NULL, OFFSET(size)},
    {"checksum_type",   (getter)get_int, (setter)set_int, NULL, OFFSET(checksum_type)},
    {"checksum",        (getter)get_str, (setter)set_str, NULL, OFFSET(checksum)},
    {NULL, NULL, NULL, NULL, NULL} /* sentinel */
};

/* Object */

PyTypeObject ContentStat_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                              /* ob_size */
    "createrepo_c.ContentStat",     /* tp_name */
    sizeof(_ContentStatObject),     /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor) contentstat_dealloc, /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_compare */
    (reprfunc) contentstat_repr,    /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
    "ContentStat object",           /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    PyObject_SelfIter,              /* tp_iter */
    0,                              /* tp_iternext */
    0,                              /* tp_methods */
    0,                              /* tp_members */
    contentstat_getsetters,         /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc) contentstat_init,    /* tp_init */
    0,                              /* tp_alloc */
    contentstat_new,                /* tp_new */
    0,                              /* tp_free */
    0,                              /* tp_is_gc */
};