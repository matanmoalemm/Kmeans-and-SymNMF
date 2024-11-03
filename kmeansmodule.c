#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef signed long ssize_t;

typedef struct Node {
    double *data;
    struct Node* next;
} Node;

typedef struct LinkedList{
    Node* head;
    Node* tail;
    int size;
}LinkedList;


double EDistance(double *point1, double *point2,int d);
int converge(double **prev ,double **curr, int curr_itr,int Max_itr, int K , int d, double epsilon);
LinkedList** assign(LinkedList* data,double **currClusters,int d,int K);
double** updateCentroids(LinkedList** assignments,int K , int d);
double** copyClusters(double** original,int k, int d);
void freeCentroids(double** centroids, int K);
PyObject* fit(double** centroids, LinkedList* data,int d,int k,double eps,int Max_iter);

LinkedList* createLinkedList();
Node* createNode(double *data,int d);
void append(LinkedList* list, double *data,int d);
void display(LinkedList* list);
void freeList(LinkedList* list);
int handleMemoryFail();

LinkedList* convert_data(PyObject *pyData,int d);
double** convert_centroids(PyObject *pyCentroids,int d);
double* get_point(PyObject *pyPoint,int d);


LinkedList* createLinkedList() {
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    if (list == NULL) handleMemoryFail();
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

Node* createNode(double *data,int d) {
    double *point;
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) handleMemoryFail();
    point = (double*)calloc(d,sizeof(double));
    
    memcpy(point,data,d*sizeof(double));
    newNode->data = point;
    newNode->next = NULL;
    return newNode;
}

void append(LinkedList* list, double *data,int d) {
    Node* newNode = createNode(data,d);
    if (list->tail == NULL) {
        list->head = newNode;
        list->tail = newNode;
        
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
    }
    list->size++;
}

void freeList(LinkedList* list) {
    Node* current = list->head;
    Node* tmp;
    while (current != NULL) {
        tmp = current;
        current = current->next;
       free(tmp->data);
        free(tmp);
    }
    free(list);
}


int handleMemoryFail() {
    fprintf(stderr, "An Error Has Occurred\n");
    exit(1);
}


double EDistance(double *point1, double *point2,int d){
int i;
double sum = 0.0;
for (i=0; i<d; i++){
  sum += pow(point1[i]-point2[i],2);
}
return sqrt(sum);

}



int converge(double **prev ,double **curr, int curr_itr,int Max_itr, int K , int d, double epsilon){

int i;
  if (curr_itr > Max_itr) return 1;
   for (i = 0;i< K;i++){
    if (EDistance(prev[i],curr[i],d) > epsilon) return 0;
}
return 1;
}

LinkedList** assign (LinkedList* data,double **currClusters,int d,int K){
    int i,j;
    int index;
    double minDist , dist;
    Node* dataNode;
  LinkedList** assignments = (LinkedList**)calloc(K ,sizeof(LinkedList*));
  if (assignments == NULL) handleMemoryFail();
  for (i = 0 ; i < K;i++){
    assignments[i] = createLinkedList();
  }
    dataNode = data->head;
  while (dataNode != NULL){
     minDist = 2147483647;
     index = -1;
    for (j = 0 ; j < K ; j++){
       dist = EDistance(dataNode->data,currClusters[j],d);
        if ( dist < minDist) {
          minDist = dist;
          index = j;   
    }
  }
     append(assignments[index],dataNode->data,d);
    dataNode = dataNode->next;
  }
  return assignments;
}


double** updateCentroids(LinkedList** assignments,int K , int d){
    int i,j;
  double** centroids = (double**)calloc(K , sizeof(double*));
  LinkedList* list; Node* node;
  if (centroids == NULL) handleMemoryFail();
  for (i = 0; i < K; i++) {
      centroids[i] = (double*)calloc(d ,sizeof(double));
      if (centroids[i] == NULL) handleMemoryFail();
  }
  for (i = 0; i < K; i++) {
      list = assignments[i];
      for (j = 0 ; j < d; j++){   
        node = list->head;   
          while(node != NULL){
            centroids[i][j] += node->data[j];
            node = node->next;
                             }
      centroids[i][j] = centroids[i][j] / assignments[i]->size;

          }
          
      
      }
      return centroids;
      }
      double** copyClusters(double** original,int k, int d){
        int i;
        double** newClusters = (double**)calloc(k,sizeof(double*));
        if (newClusters == NULL) handleMemoryFail();
        for (i=0; i < k;i++){
            newClusters[i] = (double*)calloc(d,sizeof(double));
            if (newClusters[i] == NULL) handleMemoryFail();
          memcpy(newClusters[i],original[i],d*sizeof(double));
        }

      return newClusters;
      }


void freeCentroids(double** centroids, int K) {
    int i;
    for ( i = 0; i < K; i++) {
        free(centroids[i]);
    }
    free(centroids);
}



PyObject* fit(double** centroids, LinkedList* data,int d,int k,double eps,int Max_iter){
    int i,j;
    PyObject* final_centroids;
    PyObject* point;
    PyObject* python_float;
    
    int is_converged = 0; int cnt = 0 ;
    LinkedList** assignments; double** prevCentroids; double** newCentroids;


    while(is_converged == 0){
        assignments = assign(data,centroids,d,k);
        prevCentroids = copyClusters(centroids,k,d);
        newCentroids = updateCentroids(assignments,k,d);

        cnt ++;
        is_converged = converge(prevCentroids,newCentroids,cnt,Max_iter,k,d,eps);

        for (i =0 ;i<k;i++){
            freeList(assignments[i]);
        }
        free(assignments);
        freeCentroids(prevCentroids,k);
        freeCentroids(centroids, k);
        centroids = newCentroids;

    }
    final_centroids = PyList_New(k);

    for (i = 0; i < k ; i++){
        point = PyList_New(d);
        for (j = 0; j < d; j++){
            python_float = Py_BuildValue("d", centroids[i][j]);
            PyList_SetItem(point, j, python_float);
        }
        PyList_SetItem(final_centroids, i, point);
    }
    
    freeList(data);
    free(centroids);
    return final_centroids;
    
}


static PyObject* fit_c(PyObject *self, PyObject *args)
{
    PyObject *pyCentroids ,*pyData;
    int d, k,iter;
    double eps;
    LinkedList* data;
    double** centroids;
    if(!PyArg_ParseTuple(args, "OOiiid", &pyCentroids, &pyData, &d, &k,&iter ,&eps)) {
        return NULL; /* In the CPython API, a NULL value is never valid for a
                        PyObject* so it is used to signal that an error has occurred. */
    }
    data = convert_data(pyData,d);
    centroids = convert_centroids(pyCentroids,d);

/* This builds the answer ("d" = Convert a C double to a Python floating point number) back into a python object */
    return Py_BuildValue("O", fit(centroids,data,d,k,eps,iter)); /*  Py_BuildValue(...) returns a PyObject*  */
}


static PyMethodDef fitMethods[] = {
    {"fit_c",                   /* the Python method name that will be used */
      (PyCFunction) fit_c, /* the C-function that implements the Python function and returns static PyObject*  */
      METH_VARARGS,           /* flags indicating parameters
accepted for this function */
      PyDoc_STR("fit expects : (double** centroids,LinkedList* data ,int d ,int k ,double eps)")}, /*  The docstring for the function */
    {NULL, NULL, 0, NULL}     /* The last entry must be all NULL as shown to act as a
                                 sentinel. Python looks for this entry to know that all
                                 of the functions for the module have been defined. */
};

static struct PyModuleDef kmeansmodule = {
    PyModuleDef_HEAD_INIT,
    "mykmeanssp", /* name of module */
    NULL, /* module documentation, may be NULL */
    -1,  /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    fitMethods /* the PyMethodDef array from before containing the methods of the extension */
};


PyMODINIT_FUNC PyInit_mykmeanssp(void)
{
    PyObject *m;
    m = PyModule_Create(&kmeansmodule);
    if (!m) {
        return NULL;
    }
    return m;
}

LinkedList* convert_data(PyObject *pyData,int d){
    LinkedList* data;
    Py_ssize_t i,n;
    PyObject* point;
    double* c_point;
    n = PyList_Size(pyData);
    data = createLinkedList();
    for(i = 0 ; i < n; i++){
        point = PyList_GetItem(pyData, i);
        c_point = get_point(point,d);
        append(data,c_point,d);
        free(c_point);

    }
    
    return data;
}

double** convert_centroids(PyObject *pyCentroids,int d){
    double** centroids;
    Py_ssize_t i,n;
    PyObject* point;
    double* c_point;
    n = PyList_Size(pyCentroids);

    centroids = (double**)calloc(n, sizeof(double));
    if (centroids == NULL) handleMemoryFail();
    for(i = 0; i < n; i++){
        point = PyList_GetItem(pyCentroids, i);
        c_point = get_point(point,d);
        centroids[i] = c_point;  
    }
    

    return centroids;
}


double* get_point(PyObject *pyPoint,int d){
    int j;
    PyObject* entry;
    double* c_point;
    c_point = (double*)calloc(d, sizeof(double));
    if (c_point == NULL) handleMemoryFail();
    for(j = 0 ; j < d ; j++){
        entry = PyList_GetItem(pyPoint, j);
        c_point[j] = PyFloat_AsDouble(entry);
    }
    return c_point;
}
