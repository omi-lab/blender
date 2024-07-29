#include "BKE_omi_extension.h"
#include "RNA_access.h"
#include "RNA_types.h"
#include "RNA_define.h"
#include "WM_types.h"

#include "Python.h"


const char* python_omi_arg_name()
{
  return "feed_back_incremental_render";
}

void omi_pycallback(struct PyNotify_OmiContext* self)
{
  PyEval_RestoreThread(self->ts);
  PyObject* result = PyObject_CallNoArgs(self->omi_callback);
  if(result == NULL){
    PyErr_Print();
#ifdef _MSC_VER
    // in visual studio debug build abort does not stop application. Instead it will pop up wait dialog
    // and keep application running. This makes other thread generate massive output to console.
    // For convenience, to make this error message final in the console (to be able to
    // see it easly) we stop application immediately. Thus line will swich off debug dialog on abort:
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
    abort();
    PyErr_Clear(); // clear error if it is happen
  }
  else
    Py_DECREF(result);
  self->ts = PyEval_SaveThread();
}

void init_notify_context(struct PyNotify_OmiContext* self, PyObject *kw)
{
  self->omi_callback = NULL;
  self->ts = NULL;
  self->callback = omi_pycallback;

  if(kw) {
    self->omi_callback = PyDict_GetItem(kw, PyUnicode_FromString(python_omi_arg_name()));
    if(self->omi_callback) {
      if( PyCallable_Check(self->omi_callback)){
        Py_INCREF(self->omi_callback);
      }
      else{
        self->omi_callback = 0;
      }

      PyDict_DelItem(kw, PyUnicode_FromString(python_omi_arg_name()));
    }
  }
}

int is_notify_context_empty(struct PyNotify_OmiContext* self)
{
  return self->omi_callback == NULL;
}

void set_notify_context(struct PyNotify_OmiContext* self, PointerRNA* ptr, wmOperatorType *ot)
{
  if(!is_notify_context_empty(self)) {
    PropertyRNA *prop = RNA_def_string(ot->srna, python_omi_arg_name(), NULL, 256, "", "");
    RNA_def_property_flag(prop, PROP_SKIP_SAVE);

    char address_string[256];
    sprintf(address_string, "%p", self );
    RNA_string_set(ptr, python_omi_arg_name(), address_string);
  }
}

void release_notify_context(struct PyNotify_OmiContext* self)
{
  if(self->omi_callback)
    Py_DECREF(self->omi_callback);
}

void output_python_arguments(PyObject *args)
{
  {
    //ouput list of parameters from arguments for debug
    PyObject* it = PyObject_GetIter(args);
    if(it){
      for(PyObject* item = NULL; item!=NULL; item=PyIter_Next(it)){
        // if (PyUnicode_Check(item)) {
        PyObject *str_obj = PyObject_Str(item);
        if (str_obj != NULL) {
          const char *str = PyUnicode_AsUTF8(str_obj);
          if (str != NULL) {
            printf("ABlinov: Object: %s\n", str);
          } else {
            PyErr_Print();
          }
          Py_DECREF(str_obj);  // Decrease reference count for the string object
        } else {
          PyErr_Print();
        }
        // }
        Py_DECREF(item);
      }
      Py_DECREF(it);
    }
  }
}
