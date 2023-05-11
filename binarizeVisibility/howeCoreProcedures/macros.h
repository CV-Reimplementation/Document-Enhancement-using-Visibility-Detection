#ifndef SQR
#define SQR(x) ((x)*(x))
// macro for computing squares
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
// macro for computing minimum
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
// macro for computing maximum
#endif

#ifndef BOUND
#define BOUND(v, l, h) (((v) > (l)) ? (((v) < (h)) ? (v) : (h)) : (l))
  /* macro for bounding a number by two others */
#endif

#ifndef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))
// macro for computing absolute value
#endif

#ifndef SIGN
#define SIGN(a) (((a)<0)?(-1):(((a)>0)?(1):(0)))
// macro for getting the sign of a number
#endif 

#ifndef ROUND
//#define ROUND(a) ((int)(floor((a)+0.5)))
#define ROUND(a) (((a)<0) ? ((int)((a)-0.5)) : ((int)((a)+0.5)))
// macro for computing rounded value
#endif

#ifndef FLOOR
#define FLOOR(a) (((a)<0) ? (((int)(a))-1) : ((int)(a)))
// macro for computing floor value
#endif

// macro for computing the modulus of ints
#ifndef MOD
#define MOD(x,n) (((x)%(n)<0) ? ((x)%(n)+(n)):((x)%(n)))
#endif

// macro for computing the modulus of non-ints
#ifndef FMOD
#define FMOD(x,n) ((x)-((n)*(int)((x)/(n)))+((n)*((x)<0)))
#endif

// macro for checking error conditions
#ifndef errCheck
#define errCheck(a,b) if (!(a)) mexErrMsgTxt((b));
#endif

#ifndef PI
#define PI 3.1415926535897931
#endif

#ifndef TWOPI
#define TWOPI 6.2831853071795862
#endif
