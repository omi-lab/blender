/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2001-2002 NaN Holding BV. All rights reserved. */
#pragma once

/** \file
 * \ingroup bke
 */

#ifdef __cplusplus
extern "C" {
#endif

struct _object;
typedef struct _object PyObject;
struct _ts;
typedef struct _ts PyThreadState;

typedef struct wmOperatorType wmOperatorType;

typedef struct PointerRNA PointerRNA;

struct PyNotify_OmiContext{
  int (*callback)(struct PyNotify_OmiContext *);
  PyObject *omi_callback;
  PyThreadState *ts;
};

typedef struct PyNotify_OmiContext PyNotify_OmiContext;

const char* python_omi_arg_name();

int omi_pycallback(struct PyNotify_OmiContext* self);

void init_notify_context(struct PyNotify_OmiContext* self, PyObject *kw);

int is_notify_context_empty(struct PyNotify_OmiContext* self);

void set_notify_context(struct PyNotify_OmiContext* self, PointerRNA* ptr, wmOperatorType *ot);

void release_notify_context(struct PyNotify_OmiContext* self);

void output_python_arguments(PyObject *args);

#ifdef __cplusplus
}
#endif
