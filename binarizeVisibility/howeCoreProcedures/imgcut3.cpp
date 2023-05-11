/****************************************************************************/
//
// Matlab interface file:  IMGCUT3
//
// Written 5/10 by N. Howe, based on imgcut2.
// Calls graph cut routines from Kolmogorov & Boykov code,
// starting with image data inputs.
//
/****************************************************************************/

#include "mex.h"
#include "graph.h"

#define malloc mxMalloc
#define calloc mxCalloc
#define realloc mxRealloc
#define free mxFree

// macro for checking error conditions
#ifndef errCheck
#define errCheck(a,b) if (!(a)) mexErrMsgTxt((b));
#endif

/****************************************************************************/
//
// gateway driver to call the multiway cut from matlab
//
// This is the matlab entry point
void 
mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  int i, j;
  int ndim = 2;
  long n, m, nmin;
  double *src, *snk, *nbr; 
  double flow = 0;
  double *out;
	
  // check for proper number and size of arguments
  errCheck(nrhs == 4,"Arguments:  src snk d1 d2");
  errCheck(mxGetNumberOfDimensions(prhs[0])==2,"2D arguments only");
  errCheck(mxGetNumberOfDimensions(prhs[1])==2,"2D arguments only");
  errCheck(nrhs == ndim+2,"Number of args must match image dimensionality.");
  const int *dim1 = mxGetDimensions(prhs[0]);
  const int *dim2 = mxGetDimensions(prhs[1]);
  for (i = 0; i < ndim; i++) {
    errCheck(dim1[i] == dim2[i],"First two arguments must be same size.");
  }
  for (i = 0; i < ndim; i++) {
    //mexPrintf("%d:  %d vs. %d\n",i,ndim,mxGetNumberOfDimensions(prhs[2+i]));
    errCheck(ndim == mxGetNumberOfDimensions(prhs[2+i]),
             "Difference arguments must be same dimensionality.");
    dim2 = mxGetDimensions(prhs[2+i]);
    for (j = 0; j < ndim; j++) {
      //mexPrintf("Arg %d dimension %d:  %d vs. %d.\n",i,j,dim1
      errCheck(dim1[j] == dim2[j]+(j==i),"Difference argument sizes bad.");
    }
  }
  if (nlhs > 2) {
    mexErrMsgTxt("Too many output arguments.");
  }
  src = mxGetPr(prhs[0]);
  snk = mxGetPr(prhs[1]);

  //mexPrintf("c\nc maxflow - push-relabel (highest level)\nc\n");

  // set up graph data structure
  typedef Graph<double,double,double> GraphType;
  // had used int,int,int up to 6/30/11 -- could change cause trouble?
  GraphType *g = new GraphType(dim1[0]*dim1[1],
                               (dim1[0]-1)*dim1[1]+dim1[0]*(dim1[1]-1)); 

  // add nodes
  for (i = 0; i < dim1[0]*dim1[1]; i++) {
    g -> add_node(); 
  }

  // add source/sink weights
  for (i = 0; i < dim1[0]*dim1[1]; i++) {
    g -> add_tweights(i,src[i],snk[i]);
  }

  // add neighbor links
  nbr = mxGetPr(prhs[2]);
  for (i = 0; i < dim1[0]-1; i++) {
    for (j = 0; j < dim1[1]; j++) {
      if (nbr[i+j*(dim1[0]-1)] > 0) {
        g -> add_edge(i+j*dim1[0],i+j*dim1[0]+1,
                      nbr[i+j*(dim1[0]-1)],nbr[i+j*(dim1[0]-1)]);
      }
    }
  }
  nbr = mxGetPr(prhs[3]);
  for (i = 0; i < dim1[0]; i++) {
    for (j = 0; j < dim1[1]-1; j++) {
      if (nbr[i+j*dim1[0]] > 0) {
        g -> add_edge(i+j*dim1[0],i+(j+1)*dim1[0],
                      nbr[i+j*dim1[0]],nbr[i+j*dim1[0]]);
      }
    }
  }
  
  // do calculation
  flow = g -> maxflow();

  // allocate output space
  plhs[0] = mxCreateNumericArray(ndim, dim1, mxDOUBLE_CLASS, mxREAL);
  out = mxGetPr(plhs[0]);

  // copy results to output array
  for (i = 0; i < dim1[0]; i++) {
    for (j = 0; j < dim1[1]; j++) {
      out[i+j*dim1[0]] = g->what_segment(i+j*dim1[0]) == GraphType::SINK;
    }
  }

  // cost of cut
  if (nlhs == 2) {
    plhs[1] = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(plhs[1]) = flow;
  }
  //mexPrintf("Cost of cut is %f.\n",flow);

  // let Matlab free stuff...
  delete g;
}
