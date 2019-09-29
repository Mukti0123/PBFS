#include<iostream>
#include<float.h>
#include<stdint.h>
#include<stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <vector>
#include <ctime>        
#include <random>
#include <algorithm>
#include <iterator>
#include <math.h>
#include <time.h>
#include <fstream>
#include <iomanip>
#include <sys/time.h>
#include <atomic>
#include <list>
#include<iostream>
#include<cstdlib>
#include<fstream>
#include<string>
#include<omp.h>
using namespace std;

typedef struct Graph{
  int n; //number of vertices in the graph
  int m; //number of edges in the graph
  int* cols; // array contains the column indices for nonzeros
  int* rows; // an array of col indices for the first nonzero in each row of the matrix
}graph;

int snedges,// edge count for serial execution
pnedges; // edge count for parallel execution

// Parallel BFS using two queues, Ax = y, A: graph, x: source vector, y: output vector
void ParallelBFS2Q(graph* g, int srcv, int nthreads){
  int* distance = new int[g->n];
  int level = 0;
  omp_set_num_threads(nthreads); // set number of threads
#pragma omp parallel 
{
  for(int i = 0; i < g->n; i++)
    distance[i] = -1; // all vertices are -1 distance from srcv 
}
  distance[srcv] = level;// init the srcv distance to 0
  int* x = new int[g->n];//source vector
  int* y = new int[g->n];//output vector
  int x_size = 0; // source vector size
  int y_size = 0; // output vector size
  x[x_size++] = srcv; // init the source vertex
  double starttime = omp_get_wtime(); // start time
//#pragma omp parallel
//{
  while(x_size > 0){
        #pragma omp single
        {
            ++level;
        }
        //#pragma omp for schedule(static) nowait
        #pragma omp parallel for 
            for(int i = 0; i < x_size; i++){
              int nAdj = g->rows[x[i]+1] - g->rows[x[i]];// number of adjacent vertices
              int* nAdjlist = &g->cols[g->rows[x[i]]];// cols. index array
              //#pragma omp parallel for
              for(int j= 0; j < nAdj; j++){
              #pragma omp critical
                {
                pnedges++; //increase the edge visited counter
                if(distance[nAdjlist[j]] == -1){ // check visited or not
                  //y[nAdjlist[j]] = y[nAdjlist[j]] | nz[j] & x[i]; // nz[] : nonzero values in row-major order
                  distance[nAdjlist[j]] = level; // set the distance from source
                  y[y_size++] = nAdjlist[j]; // push the vertex into the new queue, y: output vector and inc. its size 
                }
               } 
              }
            }
        #pragma omp barrier 
        #pragma omp single
        {
            int* temp = x;
            x = y;
            y = temp;
            x_size = y_size;
            y_size = 0;
        } 
     } // End of while loop
//}
  double endtime = omp_get_wtime();
  cout<<"Time taken(in sec) for Parallel execution:"<<endtime -  starttime <<endl;
  cout<<"Number of visited edges :"<<pnedges<<endl;
  delete []distance;
  delete []x;
  delete []y;
}



// serial BFS using two queues, Ax = y, A: graph, x: source vector, y: output vector
void SerialBFS2Q(graph* g, int srcv, int nthreads){
  int* distance = new int[g->n]; // distance array from srcv vertex
  int level = 0;
  for(int i = 0; i < g->n; i++)
    distance[i] = -1; // all vertices are -1 distance from srcv 
  distance[srcv] = level; // init the srcv distance to 0
  int* x = new int[g->n];//source vector
  int* y = new int[g->n];//output vector
  int x_size = 0; // source vector size
  int y_size = 0; // output vector size
  x[x_size++] = srcv; // init the source vertex
  double starttime = omp_get_wtime();
  while(x_size > 0){ // Until source vector is not empty, queue empty
    level = level + 1; // increase the level
    for(int i = 0; i < x_size; i++){ // iterate all source vertices, 
      int nAdj = g->rows[x[i]+1] - g->rows[x[i]];//number of adjacent vertices
      int* nAdjlist = &g->cols[g->rows[x[i]]];// cols. index array
      for(int j= 0; j < nAdj; j++) {
        snedges++; // increase the edge visited counter
        if(distance[nAdjlist[j]] == -1) { // if not visited 
          //y[nAdjlist[j]] = y[nAdjlist[j]] | nz[j] & x[i]; // nz[] : nonzero values in row-major order
          distance[nAdjlist[j]] = level; // set the distance from from source
          y[y_size++] = nAdjlist[j]; // push the vertex into the new queue, y: output vector and inc. its size 
        }
      }
    }
    // swap two queues and change the queue size accordingly 
    int* temp = x;
    x = y;
    y = temp;
    x_size = y_size;
    y_size = 0;
  }
  double endtime = omp_get_wtime();
  cout<<"Time taken(in sec) for Serial execution:"<<endtime -  starttime <<endl;
  cout<<"Number of visited  edges :"<<snedges<<endl;
  delete []distance;
  delete []x;
  delete []y;
}

// graph read from file to store in CSR format 
void GraphRead(char* filename, int& n, int& m, int*& srcindex, int*& dstindex){
  ifstream infile;
  string line;
  infile.open(filename);
  getline(infile, line, ' ');
  n = atoi(line.c_str()); // number of vertices
  getline(infile, line);
  m = atoi(line.c_str()); //number of edges
  int u, v;
  int index = 0;
  m = m * 2;
  srcindex = new int[m];
  dstindex = new int[m];
  for(int i = 0; i < m/2; i++){
    getline(infile, line, ' ');
    u = atoi(line.c_str());
    getline(infile, line);
    v = atoi(line.c_str());
    srcindex[index] = u;
    dstindex[index] = v;
    index =  index + 1;
    srcindex[index] = v;
    dstindex[index] = u;
    index =  index + 1;
  }
  infile.close();
}

// Graph represented as CSR format
void CSRGraph(int n, int m, int* srcindex, int* dstindex, int*& cols, int*& rows){
  cols = new int[m];
  rows = new int[n+1];
  int* tempsrcindex = new int[n];
  for(int i = 0; i < m; i++)
    cols[i] = 0;
  for(int i = 0; i < n+1; i++)
    rows[i] = 0;
  for(int i = 0; i < n;i ++)
    tempsrcindex[i] = 0;
  for(int i = 0; i < m; i++)
    ++tempsrcindex[srcindex[i]];
  for(int i = 0; i < n; i++)
    rows[i+1] = rows[i] + tempsrcindex[i];
  copy(rows, rows + n, tempsrcindex);
  for(int i = 0; i < m; i++)
    cols[tempsrcindex[srcindex[i]]++] = dstindex[i];
  delete []tempsrcindex;
}

