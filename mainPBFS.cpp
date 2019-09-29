
#include "bfsSpMVM.cpp"


int main(int argc, char** argv)
{
  if(argc != 3) {
	std::cout<<"Please enter the # threads and the graph file."<<endl;
	return -1;
   }
  int *srcindex, *dstindex;
  int nvertices, medges;
  int *cols, *rows;
  int nthreads = atoi(argv[1]); // # of threads
  GraphRead(argv[2], nvertices, medges, srcindex, dstindex); // read the file
  CSRGraph(nvertices, medges, srcindex, dstindex, cols, rows); // construct the CSR graph
  graph g = {nvertices, medges, cols, rows}; // graph object
  int srcv = rand()%nvertices; // starting vertex 
  cout<<"# vertices and  # edges:"<<nvertices<<" "<<medges<<endl;
  //SerialBFS2Q(&g, srcv, nthreads); // serial execution
  ParallelBFS2Q(&g, srcv, nthreads); // parallel execution
  return 0;
}


