#include <Python.h>
#include <pthread.h>
#define GGIL_DECLARE  PyGILState_STATE ___save
#define GGIL_ENSURE  ___save = PyGILState_Ensure();
#define GGIL_RELEASE  PyGILState_Release(___save);


static PyObject* _matrix_multiply(PyObject *matrix_A, PyObject *matrix_B) {
    if (PySequence_Size(PySequence_GetItem(matrix_A, 0)) != PySequence_Size(matrix_B)){
        return NULL;
    }

    Py_INCREF(matrix_A);
    Py_INCREF(matrix_B);

    int row_size = PySequence_Size(matrix_A);
    int col_size = PySequence_Size(PySequence_GetItem(matrix_B, 0));

    double **result = malloc(row_size * sizeof(double*));
    for (int i = 0; i < row_size; ++i) {
        result[i] = malloc(col_size * sizeof(double));
    }

    int size = PySequence_Size(matrix_B);
    for (int i = 0; i < row_size; i++) {
        for (int j = 0; j < col_size; j++) {
            result[i][j] = 0.0;
            for (int k = 0; k < size; k++) {
                PyObject *item_A = PySequence_GetItem(PySequence_GetItem(matrix_A, i), k);
                PyObject *item_B = PySequence_GetItem(PySequence_GetItem(matrix_B, k), j);
                //printf("yes\n");
                double value_A = PyFloat_AS_DOUBLE(item_A);
                if (!PyFloat_Check(item_A))
                    value_A = PyLong_AS_LONG(item_A) * (double)1.0;

                double value_B = PyFloat_AS_DOUBLE(item_B);
                if (!PyFloat_Check(item_B)) value_B = PyLong_AS_LONG(item_B) * (double)1.0;

                result[i][j] += value_A * value_B;
            }
        }
    }
    //printf("next\n");

    GGIL_DECLARE;
    GGIL_ENSURE;
    PyObject* py_result = PyList_New(row_size);
    for(int i = 0; i < row_size; i++){
        PyObject* row = PyList_New(col_size);
        for(int j = 0; j < col_size; j++){
            PyList_SET_ITEM(row, j, PyFloat_FromDouble(result[i][j]));
        }
        PyList_SET_ITEM(py_result, i, row);
    }
    //printf("saved result\n");

    GGIL_RELEASE;

    for(int i=0; i<row_size; i++) free(result[i]);
    free(result);

    return py_result;
}
static PyObject* matrix_multiply(PyObject *self, PyObject *args){
    PyObject *matrix_A, *matrix_B;
    if (!PyArg_ParseTuple(args, "OO", &matrix_A, &matrix_B))
    {
        return NULL;
    }
    return _matrix_multiply(matrix_A, matrix_B);
}

pthread_t thread;

static void* _matrix_multiply_async(void *args_void_pointer) {
    PyObject **args = (PyObject**) args_void_pointer;
    PyObject *matrix_A= args[0], *matrix_B= args[1], *loop= args[2], *future= args[3];

    PyObject* res = _matrix_multiply(matrix_A, matrix_B);

    GGIL_DECLARE;
    GGIL_ENSURE;
    PyObject *method = PyObject_GetAttrString(future, "set_result");
    PyObject_CallMethodObjArgs(loop, Py_BuildValue("s", "call_soon_threadsafe"), method, res, NULL);
    GGIL_RELEASE;
    Py_DECREF(matrix_A);
    Py_DECREF(matrix_B);
    Py_DECREF(loop);
    Py_DECREF(future);
    free(args);
    return NULL;
}

static PyObject* matrix_multiply_async(PyObject *self, PyObject *args) {
    PyObject *matrix_A, *matrix_B, *loop;
    if (!PyArg_ParseTuple(args, "OOO", &matrix_A, &matrix_B, &loop))
    {
        return NULL;
    }
    PyObject *future = PyObject_CallMethodNoArgs(loop, Py_BuildValue("s", "create_future"));
    PyObject **values = malloc(4 * sizeof(PyObject*));

    Py_INCREF(matrix_A);
    Py_INCREF(matrix_B);
    Py_INCREF(loop);
    Py_INCREF(future);

    values[0] = matrix_A;
    values[1] = matrix_B;
    values[2] = loop;
    values[3] = future;
    //PyEval_InitThreads();
    pthread_create(&thread, NULL, _matrix_multiply_async, (void*)values);
    return future;
}

// Таблица методов
static PyMethodDef MatrixMultiplyAioMethods[] = {
    {"matrix_multiply", matrix_multiply, METH_VARARGS, "Multiplies two matrices."},
    {"matrix_multiply_async", matrix_multiply_async, METH_VARARGS, "Multiplies two matrices async to other thread."},
    {NULL, NULL, 0, NULL}  // Завершающий элемент таблицы
};

// Структура модуля
static struct PyModuleDef matrixmultiplyaiomodule = {
    PyModuleDef_HEAD_INIT,
    "matrixmultiplyaio",
    "Matrix multiplication module.",
    -1,
    MatrixMultiplyAioMethods
};

// Инициализация модуля
PyMODINIT_FUNC PyInit_matrixmultiplyaio(void) {
    return PyModule_Create(&matrixmultiplyaiomodule);
}
