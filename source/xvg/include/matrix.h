/*
 * @(#)matrix.h    generated by: makeheader    Tue Jun 22 16:36:48 1993
 *
 *		built from:	/rel/bld/matlab4/sun4/ansi.joe/sandbox4/src/include/mathwork.h
 *				/rel/bld/matlab4/sun4/ansi.joe/sandbox4/src/cmex/cmxcbk.c
 */

#ifndef matrix_h
#define matrix_h

typedef double Real;


#ifdef OLDSTYLE
/*
 * modified matrix definition for compatibility with pre v4 mex files
 */

#ifdef THINK_C
#define dummy   unsigned char dummy1,dummy2,type,dummy3; int dummy4
#else
#ifdef applec
#define dummy   unsigned char dummy1,dummy2,type,dummy3; int dummy4
#else
#define dummy   int dummy1,dummy2,type,dummy3
#endif /* applec */
#endif /* THINK_C */

#define mxMAXNAM        20
typedef struct matrix {
        char    name[mxMAXNAM];         /* name is now an array */
        dummy;                          /* type: 0 - matrix, 1 - string */
        int     m;                      /* row dimension */
        int     n;                      /* column dimension */
        Real    *pr;                    /* pointer to real part */
        Real    *pi;                    /* pointer to imag part */
        int     dummy6,dummy7,dummy8;
} Matrix;

#define TEXT    1                       /* mat.type indicating text */
#define MATRIX  0                       /* mat.type indicating matrix */
#define REAL    0
#define COMPLEX 1

#else
/*
 * post v4 mex files use an incomplete matrix structure and access methods
 */
#ifdef __STDC__
/*
 * incomplete definition of Matrix
 */
typedef struct matrix Matrix;
#else
#ifdef VMS
typedef struct matrix Matrix;
#else
#ifdef MMACINTOSH
typedef struct matrix Matrix;
#else
typedef char    Matrix;
#endif /* MMACINTOSH */
#endif /* VMS */
#endif /* __STDC__ */

#define REAL    (0)
#define COMPLEX (1)
#endif /* OLD_STYLE */



extern	Matrix	*mxCreateFull(
    int		m,		/* number of rows in matrix */
    int		n,		/* number of columns */
    int		cmplx_flg	/* complex flag */
    );
extern	void mxFreeMatrix(
    Matrix	*pmat		/* pointer to matrix */
    );
extern	char *mxGetName(
    Matrix *pm		/* pointer to matrix */
    );
extern	void mxSetName(
    Matrix	*pm,		/* pointer to matrix */
    const char	*s		/* string to copy into name */
    );
extern	int mxGetM(
    const Matrix *pm		/* pointer to matrix */
    );
extern	void mxSetM(
    Matrix	*pm,		/* pointer to matrix */
    int		m		/* row dimension */
    );
extern	int mxGetN(
    const Matrix *pm		/* pointer to matrix */
    );
extern	void mxSetN(
    Matrix	*pm,		/* pointer to matrix */
    int		n		/* column dimension */
    );
extern	Real *mxGetPr(
    const Matrix *pm		/* pointer to matrix */
    );
extern	void mxSetPr(	
    Matrix	*pm,		/* pointer to matrix */	
    Real	*pr		/* pointer to real part */
    );
extern	Real *mxGetPi(
    const Matrix *pm		/* pointer to matrix */	
    );	
extern	void mxSetPi(	
    Matrix	*pm,		/* pointer to matrix */
    Real	*pi		/* pointer to imag part */
    );
extern	int mxGetNzmax(	
    const Matrix *pm		/* pointer to matrix */
    );
extern	void mxSetNzmax(
    Matrix	*pm,		/* pointer to matrix */
    int		nzmax		/* number of nonzero elements */
    );
extern	int *mxGetIr(
    const Matrix *pm		/* pointer to matrix */
    );
extern	void mxSetIr(	
    Matrix	*pm,		/* pointer to matrix */
    int		*ir		/* pointer to ir array */
    );
extern	int *mxGetJc(	
    const Matrix *pm		/* pointer to matrix */
    );
extern	void mxSetJc(	
    Matrix	*pm,		/* pointer to matrix */
    int		*jc		/* pointer to sparse jc array */
    );
extern	int mxGetString(
    const Matrix *pm,		/* pointer to matrix */
    char	*str_ptr,	/* pointer to string holding results */
    int		str_len		/* length of string that holds results */
    );
extern	Matrix *mxCreateString(
    const char	*str_ptr	/* input C string */
    );
extern	Real mxGetScalar(
    const Matrix	*pm		/* pointer to matrix */
    );
extern	int mxIsFull(
    const Matrix *pm		/* pointer to matrix */
    );
extern	int mxIsSparse(
    const Matrix *pm		/* pointer to matrix */
    );
extern	int mxIsDouble(
    const Matrix *pm		/* pointer to matrix */
    );
extern	int mxIsString(
    const Matrix *pm		/* pointer to matrix */
    );
extern	int mxIsNumeric(
    const Matrix *pm		/* pointer to matrix */
    );
extern	int mxIsComplex(
    const Matrix *pm		/* pointer to matrix */
    );
extern	Matrix *mxCreateSparse(
    int		m,		/* number of rows */
    int		n,		/* number of columns */
    int		nzmax,		/* initial number of non-zeros elements */
    int		cmplx_flg	/* complex data */
    );

#endif /* matrix_h */
