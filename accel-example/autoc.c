//====================================================== file = autoc.c =====
//=  Program to compute autocorrelation for a series X of size N            =
//===========================================================================
//=  Notes:                                                                 =
//=    1) Input from input file "in.dat" to stdin (see example below)       =
//=        * Comments are bounded by "&" characters at the beginning and    =
//=          end of the comment block                                       =
//=    2) Output is to stdout                                               =
//=    3) Must manually set NUM_LAG                                         =
//=-------------------------------------------------------------------------=
//= Example "in.dat" file:                                                  =
//=                                                                         =
//=    & Sample series of data which can be integers or reals.              =
//=      There are 11 values in this file. &                                =
//=    50                                                                   =
//=    42                                                                   =
//=    48                                                                   =
//=    61                                                                   =
//=    60                                                                   =
//=    53                                                                   =
//=    39                                                                   =
//=    54                                                                   =
//=    42                                                                   =
//=    59                                                                   =
//=    53                                                                   =
//=-------------------------------------------------------------------------=
//= Example output (for above "in.dat" and NUM_LAG = 3):                    =
//=                                                                         =
//=   c:\work\autoc < in.dat                                                =
//=   ------------------------------------------------- autoc.c -----       =
//=     Autocorrelation for lag    1 = -0.055190                            =
//=     Autocorrelation for lag    2 = -0.173395                            =
//=     Autocorrelation for lag    3 = -0.708910                            =
//=   ---------------------------------------------------------------       =
//=-------------------------------------------------------------------------=
//=  Build: gcc autoc.c -lm, bcc32 autoc.c, cl autoc.c                      =
//=-------------------------------------------------------------------------=
//=  Execute: autoc < in.dat                                                =
//=-------------------------------------------------------------------------=
//=  Author: Kenneth J. Christensen                                         =
//=          University of South Florida                                    =
//=          WWW: http://www.csee.usf.edu/~christen                         =
//=          Email: christen@csee.usf.edu                                   =
//=-------------------------------------------------------------------------=
//=  History: KJC (09/16/98) - Genesis                                      =
//=           KJC (05/27/13) - Modified to use malloc() for heap space      =
//=           KJC (09/13/13) - Corrected error in compute_autoc()           =
//===========================================================================
//----- Include files -------------------------------------------------------
#include <stdio.h>                 // Needed for printf() and feof()
#include <math.h>                  // Needed for pow()�
#include <stdlib.h>                // Needed for exit() and atof()
#include <string.h>                // Needed for strcmp()

//----- Defines -------------------------------------------------------------
#define MAX_SIZE 10000000          // Maximum size of time series data array
#define NUM_LAG       100          // Number of lags to compute for

//----- Globals -------------------------------------------------------------
double    *X;                      // Time series read from "in.dat"
long int   N;                      // Number of values in "in.dat"
double     Mean;                   // Mean of series X
double     Variance;               // Variance of series X

//----- Function prototypes -------------------------------------------------
void   load_X_array(void);         // Load X array
double compute_mean(void);         // Compute Mean for X
double compute_variance(void);     // Compute Variance for X
double compute_autoc(int lag);     // Compute autocorrelation for X

//===========================================================================
//=  Main program                                                           =
//===========================================================================
void main(void)
{
  double   ac_value;                 // Computed autocorrelation value
  int      i;                        // Loop counter

  // Malloc space for X
  X = (double *) malloc(sizeof(double) * MAX_SIZE);
  if (X == NULL)
  {
    printf("*** ERROR - Could not malloc() enough space \n");
    exit(1);
  }

  // Load the series X
  printf("------------------------------------------------- autoc.c -----\n");
  load_X_array();

  // Compute global variables Mean and Variance for X
  Mean = compute_mean();
  Variance = compute_variance();

  // Compute and output AC value (rho) for series X of length N
  for (i=1; i<=NUM_LAG; i++)
  {
    ac_value = compute_autoc(i);
    printf("  Autocorrelation for lag %4ld = %f \n", i, ac_value);
  }
  printf("---------------------------------------------------------------\n");

  // Free the malloced memory
  free(X);
}

//===========================================================================
//=  Function to load X array from stdin and determine N                    =
//===========================================================================
void load_X_array(void)
{
  char      temp_string[1024];     // Temporary string variable

  // Read all values into X
  N = 0;
  while(1)
  {
    scanf("%s", temp_string);
    if (feof(stdin)) goto end;

    // This handles a comment bounded by "&" symbols
    while (strcmp(temp_string, "&") == 0)
    {
      do
      {
        scanf("%s", temp_string);
        if (feof(stdin)) goto end;
      } while (strcmp(temp_string, "&") != 0);
      scanf("%s", temp_string);
      if (feof(stdin)) goto end;
    }

    // Enter value in array and increment array index
    X[N] = atof(temp_string);
    N++;

    // Check if MAX_SIZE data values exceeded
    if (N >= MAX_SIZE)
    {
      printf("*** ERROR - greater than %ld data values \n", MAX_SIZE);
      exit(1);
    }
  }

  // End-of-file escape
  end:

  return;
}

//===========================================================================
//=  Function to compute mean for a series X                                =
//===========================================================================
double compute_mean(void)
{
  double   mean;        // Computed mean value to be returned
  int      i;           // Loop counter

  // Loop to compute mean
  mean = 0.0;
  for (i=0; i<N; i++)
    mean = mean + (X[i] / N);

  return(mean);
}

//===========================================================================
//=  Function to compute variance for a series X                            =
//===========================================================================
double compute_variance(void)
{
  double   var;         // Computed variance value to be returned
  int      i;           // Loop counter

  // Loop to compute variance
  var = 0.0;
  for (i=0; i<N; i++)
    var = var + (pow((X[i] - Mean), 2.0) / N);

  return(var);
}

//===========================================================================
//=  Function to compute autocorrelation for a series X                     =
//=   - Corrected divide by N to divide (N - lag) from Tobias Mueller       =
//===========================================================================
double compute_autoc(int lag)
{
  double   autocv;      // Autocovariance value
  double   ac_value;    // Computed autocorrelation value to be returned
  int      i;           // Loop counter

  // Loop to compute autovariance
  autocv = 0.0;
  for (i=0; i<(N - lag); i++)
    autocv = autocv + ((X[i] - Mean) * (X[i+lag] - Mean) / (N - lag));

  // Autocorrelation is autocovariance divided by variance
  ac_value = autocv / Variance;

  return(ac_value);
}