/****************************************************************************/
//
// Matlab interface file:  IMGCUTMULTI
//
// Written 6/11 by N. Howe, based on imgcut3.
// Calls graph cut routines from Kolmogorov & Boykov code,
// starting with image data inputs.  Reuses solutions.
//
/****************************************************************************/

#include "mex.h"
#include "macros.h"
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
  int i, j, imult;
  int ndim = 2;
  long n, m, nmin, nmult;
  double *src, *snk, *mult, *nbr; 
  double flow = 0;
  double *out, *cost;
  mxArray* cell;
  //Block<G::node_id>* changed_list = new Block<G::node_id>(128);
  //mexPrintf("Starting.\n");
	
  // check for proper number and size of arguments
  errCheck(nrhs == 5,"Arguments:  src snk d1 d2 mult");
  errCheck(mxGetNumberOfDimensions(prhs[0])==2,"2D arguments only");
  errCheck(mxGetNumberOfDimensions(prhs[1])==2,"2D arguments only");
  errCheck(nrhs == ndim+3,"Number of args must match image dimensionality.");
  const mwSize *dim1 = mxGetDimensions(prhs[0]);
  const mwSize *dim2 = mxGetDimensions(prhs[1]);
  for (i = 0; i < ndim; i++) {
    errCheck(dim1[i] == dim2[i],"First two arguments must be same size.");
  }
  errCheck(mxIsDouble(prhs[2]),"Edge connections must be doubles.");
  errCheck(mxIsDouble(prhs[3]),"Edge connections must be doubles.");
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
  errCheck(mxIsDouble(prhs[4]),"Multipliers must be double.");
  errCheck(!mxIsComplex(prhs[4]),"Multipliers must be real.");
  if (nlhs > 2) {
    mexErrMsgTxt("Too many output arguments.");
  }
  src = mxGetPr(prhs[0]);
  snk = mxGetPr(prhs[1]);
  nmult = mxGetNumberOfElements(prhs[4]);
  errCheck(nmult>0,"Must specify at least one multiple.");
  mult = mxGetPr(prhs[4]);
  //mexPrintf("Arguments read.\n");
  //mexPrintf("%d multipliers:  ",nmult);
  //for (i = 0; i < nmult; i++) {
    //mexPrintf("%f ",mult[i]);
  //}
  //mexPrintf("\n");


  //mexPrintf("c\nc maxflow - push-relabel (highest level)\nc\n");

  // set up graph data structure
  typedef Graph<double,double,double> GraphType;
  GraphType *g = new GraphType(dim1[0]*dim1[1],
                               (dim1[0]-1)*dim1[1]+dim1[0]*(dim1[1]-1)); 
  //mexPrintf("Graph set up.\n");

  // add nodes
  for (i = 0; i < dim1[0]*dim1[1]; i++) {
    g -> add_node(); 
  }
  //mexPrintf("Nodes added.\n");

  // add source/sink weights
  for (i = 0; i < dim1[0]*dim1[1]; i++) {
    g -> add_tweights(i,src[i],snk[i]);
  }
  //mexPrintf("Source/sink weights added.\n");

  // add neighbor links
  nbr = mxGetPr(prhs[2]);
  for (i = 0; i < dim1[0]-1; i++) {
    for (j = 0; j < dim1[1]; j++) {
      //mexPrintf("At (%d,%d).\n",i,j);
      if (nbr[i+j*(dim1[0]-1)] > 0) {
        //mexPrintf("  Adding edge.\n",i,j);
        g -> add_edge(i+j*dim1[0],i+j*dim1[0]+1,
                      mult[0]*nbr[i+j*(dim1[0]-1)],
                      mult[0]*nbr[i+j*(dim1[0]-1)]);
      }
    }
  }
  //mexPrintf("Neighbor links half added.\n");
  nbr = mxGetPr(prhs[3]);
  for (i = 0; i < dim1[0]; i++) {
    for (j = 0; j < dim1[1]-1; j++) {
      if (nbr[i+j*dim1[0]] > 0) {
        g -> add_edge(i+j*dim1[0],i+(j+1)*dim1[0],
                      mult[0]*nbr[i+j*dim1[0]],mult[0]*nbr[i+j*dim1[0]]);
      }
    }
  }
  //mexPrintf("Neighbor links added.\n");

  // prepare output cell array
  plhs[0] = mxCreateCellMatrix(1,nmult);
  if (nlhs == 2) {
    plhs[1] = mxCreateDoubleMatrix(1,nmult,mxREAL);
    cost = mxGetPr(plhs[1]);
  }
  //mexPrintf("Prep complete.\n");

  // do calculations
  for (imult = 0; imult < nmult; imult++) {
    // special treatment for first time through
    if (imult==0) {
      // do calculation (first time)
      flow = g -> maxflow();
      //mexPrintf("Initial calculation.\n");
    } else {
      // make sure multiplier is increasing
      // as long as it is, the residual capacity on edges will only increase,
      // which means we don't need to mess with the flow from source/sink.
      errCheck(mult[imult]>=mult[imult-1],
               "Multipliers must be specified in increasing order");
      //mexPrintf("Repeat calculation.\n");

      // modify graph for reuse on subsequent iterations
      GraphType::arc_id a = g->get_first_arc();
      GraphType::arc_id a2 = g->get_next_arc(a);
      nbr = mxGetPr(prhs[2]);
      for (i = 0; i < dim1[0]-1; i++) {
        for (j = 0; j < dim1[1]; j++) {
          if (nbr[i+j*(dim1[0]-1)] > 0) {
            // if edge was saturated, mark nodes for reconsideration
            if ((g->get_rcap(a)==0)||(g->get_rcap(a2)==0)) {
              g->mark_node(i+j*dim1[0]);
              g->mark_node(i+j*dim1[0]+1);
            }
            g->set_rcap(a,g->get_rcap(a)+
                        (mult[imult]-mult[imult-1])*nbr[i+j*(dim1[0]-1)]);
            g->set_rcap(a2,g->get_rcap(a2)+
                        (mult[imult]-mult[imult-1])*nbr[i+j*(dim1[0]-1)]);
            a = g->get_next_arc(a2);
            a2 = g->get_next_arc(a);
          }
        }
      }
      nbr = mxGetPr(prhs[3]);
      for (i = 0; i < dim1[0]; i++) {
        for (j = 0; j < dim1[1]-1; j++) {
          if (nbr[i+j*dim1[0]] > 0) {
            // if edge was saturated, mark nodes for reconsideration
            if ((g->get_rcap(a)==0)||(g->get_rcap(a2)==0)) {
              g->mark_node(i+j*dim1[0]);
              g->mark_node(i+(j+1)*dim1[0]);
            }
            g->set_rcap(a,g->get_rcap(a)+
                        (mult[imult]-mult[imult-1])*nbr[i+j*dim1[0]]);
            g->set_rcap(a2,g->get_rcap(a2)+
                        (mult[imult]-mult[imult-1])*nbr[i+j*dim1[0]]);
            a = g->get_next_arc(a2);
            a2 = g->get_next_arc(a);
          }
        }
      }

      // do repeat calculation
      flow = g -> maxflow(true);
      //flow = g -> maxflow(true, changed_list);
    }

    // allocate output space
    cell = mxCreateDoubleMatrix(dim1[0], dim1[1], mxREAL);
    mxSetCell(plhs[0],imult,cell);
    out = mxGetPr(cell);
    
    // copy results to output array
    for (i = 0; i < dim1[0]; i++) {
      for (j = 0; j < dim1[1]; j++) {
        out[i+j*dim1[0]] = g->what_segment(i+j*dim1[0]) == GraphType::SINK;
      }
    }
    
    // cost of cut
    if (nlhs == 2) {
      cost[imult] = flow;
    }
    //mexPrintf("Cost of cut is %f.\n",flow);
  }  

  // let Matlab free stuff...
  delete g;
}
