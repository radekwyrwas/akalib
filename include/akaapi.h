/* -------------------------------------------------------------------------
 * Copyright (c) 1995-2011, Andrew Kalotay Associates.  All rights reserved. *
   ------------------------------------------------------------------------- */

#ifndef _AKAAPI_H_
#define _AKAAPI_H_
/* akaapi.h (r. 1.68) for AKA Library Version 2.06 */

#include <stddef.h>

/* -----------------------------------------------------------------
   Overview
   
   This file contains the function prototypes, structure definitions,
   and constants needed for invoking BondOAS(tm).  This file also
   serves as documentation for the API.  It is arranged in the
   following sections:

   1. General Notes
   2. Initialization and Shutdown
   3. Tree Creation
   4. Valuation
   5. Miscellaneous Functions
   6. Error Handling
   7. Memory Allocation and Free
   8. Data Structures and Constants
   9. Deprecated Functions
  ----------------------------------------------------------------- */


/* -----------------------------------------------------------------
   Section 1. General Notes

   BondOAS(tm) is a multi-thread safe C library.  Library interface
   calls take objects in the form of C structures.  These structures
   are described below.  All structures are documented.

   All dates are expressed as 8-digit longs in the form yyyymmdd.
   Percentages are expressed as x 100.  I.e., 10% is 10 rather than 0.10.

   Most AKA structures have memory allocation and free routines.  All
   AKA structures which have allocators (e.g., Bond structures) must
   be allocated using the library provided allocation functions only.
   Using stack local variables or directly allocating the structures
   via malloc will cause unexpected results and memory errors.

   ----------------------------------------------------------------- */


/* -----------------------------------------------------------------
   Section 2. Initialization and Shutdown

   One of the initialization functions must be called before the library
   may be used.

   The library requires a valid user name and key to run.  These must be
   provided to the initialization routines.  The user name and key are
   authorization keys provided to authorized users by AKA.

   The initialization routines return one of the AKAINIT_ERR_ values
   defined below.  The return code must be checked; continuing to call
   BondOAS routines after init fails will cause the library to exit.

   One of the shutdown functions should be called after the library use
   is finished.  Memory will be lost if this is not done.

   The initialization function should not be called more than once.  It
   is valid to call the initialization function again if the shutdown
   function has been called.  This is not recommended as initialization
   and shutdown functions require cycles.

   MULTI-THREAD note: only one thread may call the initialization
   function.  The initialization function creates BondOAS mutex.
   Calling the shutdown function while other threads are still running
   BondOAS routines is not safe.  The configuration settings used in
   the init function are inherited by each thread.  The duration global
   settings can be reset on a per thread basis, see AKA_set_duration_shift().
   ----------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#ifdef FIX_AUTOINDENT_NEVER_DEFINED
}  /* this is here so editors/pretty-printers don't indent on above { */
#endif
#endif

typedef struct AKAInitData AKAINITDATA;

#define AKAINIT_ERR_NONE 0      /* no error */
#define AKAINIT_ERR_AUTHORIZE 1 /* user is not authorized to run library */
#define AKAINIT_ERR_ALLOC 2     /* memory allocation error */

/* The initialization routines return one of AKAINIT_ERR_ defined above.
   AKA_initialize() uses default configuration values; see AKAINITDATA
   below. */
long AKA_initialize(long key, const char *uname);
long AKA_initialize_configure(long key, const char *uname,
			      AKAINITDATA *config);

/* Gets the default initialization configuration values and assigns
   them to the passed in configuration structure. */
void AKA_initialize_get_defaults(AKAINITDATA *config);

/* Returns expiration of key; only valid after call to init, */
void AKA_authorize_expiration_date(int *year, int *month, int *day);

/* Shutdown the library, free internal memory */
void AKA_shutdown();

/* Shutdown the library */
void AKA_shutdown_configure(int unused);

/* The variable, AKA_memory_diagnostics_enabled, controls whether all
   memory allocations in library routines are wrapped in a memory
   diagnostics counter.  Enabling memory diagnostics allows the user
   track library memory usage.  The variable must be set to one prior
   to library initialization in order for memory diaganostics to be
   enabled.  Enabling memory diagnostics will impact performance of
   multi-threaded invocations of the bondoas library. */
extern int AKA_memory_diagnostics_enabled;

/* Provides diagnostics information on library memory usage.
   Allocations, total_memory, and trees are a snapshot of usage when
   the function is called.  The numbers are for unfreed allocations and
   unreleased trees.  After shutdown, all the numbers should be zero.
   This routine is multi-thread safe.  Note, allocations and total_memory
   will always be zero unless AKA_memory_diagnostics_enabled is set. */
void AKA_memory_diagnostics(unsigned long *allocations, size_t *total_memory,
			    unsigned long *trees);



/* -----------------------------------------------------------------
   Section 3. Tree Creation

   Trees are interest rate lattices used to value instruments.  The
   first step in any valuation is to create a tree using one of the
   tree fit routines below.

   These return an AKAHTREE which is a library internal reference.  The
   user is given the handle to pass to other routines. This is not a
   pointer.  The AKATreeRelease() routine frees internal library
   memory.  After AKATreeRelease(), the handle is no longer valid.
   Un-released trees will cause memory leaks.
   An AKAHTREE of zero indicates failure and is not a valid tree.

   MULTI-THREAD note: the tree handle may be shared across threads.
   Freeing the handle via AKATreeRelease() more than once is not safe.
   Freeing the handle while it is in use by multiple threads is not safe.

   Performance Note: When AKABondVal() and AKABondVal2() is used to
   compute par duration, the library will internally attempt to fit
   trees.  The handles to these trees are not exposed to the caller.
   They will be cached internally with the tree structure.
   ----------------------------------------------------------------- */

typedef unsigned long AKAHTREE;   /* Returned from tree fit routines */

/* forward declarations -- definitions in structures section below */
typedef struct AKACurve AKACURVE;
typedef struct AKASpreads AKASPREAD;

/* Fit trees from yield curves; spread curves are optional, pass as NULL */
AKAHTREE AKATreeFit(const AKACURVE *crv, const AKASPREAD *spr);

/* Fit a tree from another tree and a spread curve */
AKAHTREE AKATreeFitSpreads(AKAHTREE tree, const AKASPREAD *spr);

/* Fit a tree from another tree and a shift of the zero or par curve.
   bpShift of 20 means 20 basis points.  Select mode from AKAShiftMode
   defined below. Using SHIFT_NONE here is an error. */
AKAHTREE AKATreeFitShift(AKAHTREE tree, double bpShift, long mode);

/* Fit a tree from a yield curve using zero volatility.  For computing
   Z-Spread.  Z-Spread is the zero-volatility OAS of a bond given its
   price. */
AKAHTREE AKATreeFitZero(const AKACURVE *crv);

/* Return the minimum rate on a tree (.10 is 10%) -1 is error */
double AKATreeMinRate(AKAHTREE hTree);

/* Release a tree handle returned by one of the tree fit routines */
void AKATreeRelease(AKAHTREE tree);


/* -----------------------------------------------------------------
   Section 4. Valuation

   These routines value bonds.

   There are two types of valuation routines.
   1. Simple Valuation -- return a single value through the function return
   2. Report Based -- return multiple values via a report structure

   Simple Valuation routines return their answer as the function
   return value.  Error return values are as described below.  These
   routines also set the error handling flags like the Report Based
   routines.

   Report Based routines require a report structure to be passed in.  All 
   report structures except AKABONDREPORT must be created with the 
   appropriate allocation function and must be freed with the corresponding 
   free function.  The return code for report based functions is not used.
   See the error handling section below for checking errors.

   MULTI-THREAD note: AKAHTREE may be shared across threads.  Pointers
   to AKABOND and AKASCEN are to const objects in these functions and
   therefore may be shared across threads.  However, changing the values
   of an AKABOND or AKASCEN while it is in use by another thread
   violates the const requirement and is not thread safe.

   ----------------------------------------------------------------- */
/* forward declarations -- definitions below */
typedef struct AKABond AKABOND;
typedef struct AKAScen AKASCEN;
typedef struct AKABondReport AKABONDREPORT;
typedef struct AKABondYieldReport AKABONDYIELDREPORT;
typedef struct AKAFlowReport AKAFLOWREPORT;
typedef struct AKAFwdReport AKAFWDREPORT;
typedef struct AKAKRDurReport AKAKRDURREPORT;
typedef struct AKAkrdursetup AKAKRDURSETUP;
typedef struct AKAScenReport AKASCENREPORT;
typedef struct AKAVolReport AKAVOLREPORT;
typedef struct AKAYldWorst AKAYLDWORST;
typedef struct AKAPrcWorst AKAPRCWORST;
typedef struct AKAAtaxYld AKAATAXYLD;
typedef struct AKAAtaxBasis AKAATAXBASIS;

/* ********************************************
   SIMPLE VALUATION routines
   ******************************************** */
/* Price a bond; returns price; -1 indicates an error */
double AKABondPrice(long pvdate, AKAHTREE hTree, const AKABOND *bond,
		    double oas);

/* Compute the OAS for a bond; returns OAS; -99999 indicates an error */
double AKABondOAS(long pvdate, AKAHTREE hTree, const AKABOND *bond,
		  double price);

/* Determine accrued for a bond - returns accrued; -1 indicates an error */
double AKABondAccrued(long pvdate, const AKABOND *bond);

/* Determine accrued and accrued days for a bond.
   Returns 1 on success, 0 on an error */
double AKABondAccrued2(long pvdate, const AKABOND *bond,
		       double *accrued, int *accrued_days);

/* Spot shift (see AKAShiftMode, below) a tree <treeshift> basis points
   then value a bond; returns price; -1 indicates an error */
double AKABondPriceShifted(long pvdate, AKAHTREE hTree, const AKABOND *bond,
			   double oas, double treeshift);

/* convert between price and yield.  Select cnvfrom/cnvto from AKAQuoteType
   defined below.  AKAQuoteType OAS is not supported for price conversion.
   To convert from OAS use AKABondVal().
   Returns:   value in cnvto units; -99999 indicates an error. */
double AKABondPriceCnv(long pvdate, long cnvfrom, long cnvto, double quote,
		       const AKABOND *bond);

/* Solve for a single fixed spread such that if the same spread is
   added to each coupon in the coupon schedule of the bond, then the
   clean price of the bond, computed using the tree with zero OAS, is
   equal to clean_price provided as input.
   Returns: this spread, in basis points, with sign reversed
   -99999 indicates an error. */
double AKAAssetSwapSpread(double clean_price, 
                          long pvdate, 
                          AKAHTREE hTree, 
                          const AKABOND *bond);


/* Compute the price of a bond at zero oas when the input asset_swap_spread is
   applied.
   Returns: the clean price
   -99999 indicates an error */
double AKAConvertAssetSwapSpread(double asset_swap_spread,
                                 long pvdate, 
                                 AKAHTREE hTree, 
                                 const AKABOND *bond);

/* Compute the I-Spread.  This is the difference between the YTM of a
   bond at the provided price, and the yield on the
   weighted-average-maturity-matched point on the provided yield
   curve. The yield will be straight-line interpolated between points
   on the provided curve. Industry refers to this spread as I-Spread
   if swap curve provided and G-Spread if government curve
   (treasuries) provided.
   Returns: this spread, in basis points, -99999 indicates an error. */
double AKAISpread(const AKABOND *bond, long pvdate, double price,
		  const AKACURVE *crv);


/* ********************************************
   REPORT STRUCTURE BASED VALUATION routines
   ******************************************** */

/* Value a bond and fill out all fields in the bond report (see
   bond report definition below).  Select quoteType from AKAQuoteType */
long AKABondVal(long pvDate, long quoteType, double quote,
		AKAHTREE hTree, const AKABOND *bond,
		AKABONDREPORT *rpt);

/* Like AKABondVal() but allows configuration of which bond report
   values to calculate.  See the definition of the bond report below
   to understand which values are calculated. Set value_x to 1 to
   value, zero to not value. */
long AKABondVal2(long pvDate, long quoteType, double quote,
		 AKAHTREE hTree, const AKABOND *bond,
		 AKABONDREPORT *rpt,
		 int value_option, int value_duration, int value_yield);

/* Perform scenario analysis on a bond, uses efficiency set in init().
   Special Note: Scenario analysis is based on input yield curves and
   the OAS implied by the initial price.  In situations where the user
   inputs a price that implies an unreasonable OAS, the scenario-based
   option exercise and cashflow reinvestment calculations may be
   distorted.  Assuming the input initial yield curve is appropriate
   for the issuer of the security, the user can suppress the use of
   implied OAS for purposes of option exercise and reinvestment by
   entering the initial price as a negative price, i.e., 99.375 is
   entered as -99.375.  In this case only the horizon price will be
   calculated using the implied OAS.  All option exercise decisions
   and reinvestment calculations will be made with an OAS of 0
   relative to the entered curves.

   AKAScenFree() does not release the trees.  The trees in the AKASCEN
   structure must be created and released by the caller.  Using
   AKA_SCEN_GRADUAL is slow.  Using AKA_SCEN_GRADUAL with factor curve
   based trees is particularly slow. */

long AKABondScen(long quoteType, double quote,
		 const AKASCEN *scen, const AKABOND *bond,
		 AKASCENREPORT *rpt);

/* Like AKABondScen() but allows setting of efficiency threshold and returns
   the observed efficiency */
long AKABondScen2(long quoteType, double quote,
		  const AKASCEN *akascen, const AKABOND *bond,
		  AKASCENREPORT *rpt,
		  double eff_threshold, double *efficiency);

/* Like AKABondScen2() but if reinvest is greater than zero, use the
   supplied reinvestment rate rather than the scenario indicated forward
   rate plus the instrument OAS.  Rate is as percent, i.e., for 10% use
   10. Rate is BEY, 30/360 semi-annual */
long AKABondScen3(long quoteType, double quote,
		  const AKASCEN *akascen, const AKABOND *bond,
		  AKASCENREPORT *rpt,
		  double eff_threshold, double *efficiency, double reinvest);

/* Like AKABondScen3() but if ataxinfo is non-null does an after tax
   total return analysis.  The AKAATAXYLD input fields must be filled out
   if ataxinfo is non-null.  The AKAATAXYLD report output fields are not
   computed -- the report is only used in this routine to supply input data.
   To compute aftertax yields see AKAAtaxYield() below. */
long AKABondScen4(long quoteType, double quote,
		  const AKASCEN *akascen, const AKABOND *bond,
		  AKASCENREPORT *rpt,
		  double eff_threshold, double *efficiency, double reinvest,
		  AKAATAXYLD *ataxinfo);

/* Compute the key rate durations for a bond and return the results
   in an allocated report.  The returned report must be freed by the
   caller using AKAKRDurReportFree().  On failure, it returns NULL.
   This routine uses shifts of the par curve at each maturity to do
   duration.  These are created by calling AKAKeyDurSetup(); */
AKAKRDURREPORT *AKABondKeyDur3(long pvdate, const AKABOND *bond,
			       long quoteType, double quote,
			       AKAKRDURSETUP *setup);

/* Construct a structure for computing the key rate durations of a
   bond using the specified duration shift (durbp), at specific
   maturities.  This structure is used in AKABondKeyDur3().  The array
   maturities contains the specific key rates for which to calculate
   duration.  If it is NULL (or maturities_count is zero) all the
   maturities in the curve are used.  maturities_count must be the
   number of elements in the maturities_array.  This structure must
   be freed by the caller using AKAKeyDurSetupFree(). */
AKAKRDURSETUP *AKAKeyDurSetup(const AKACURVE *curve, const AKASPREAD *spread,
			      double durbp,
			      double *maturities, int maturities_count);

/* Frees the structure created in AKAKeyDurSetup().  Always returns NULL. */
AKAKRDURSETUP *AKAKeyDurSetupFree(AKAKRDURSETUP *setup);


/* AKABondKeyDur2() is the same as invoking, AKAKeyDurSetup(), then
   AKABondKeyDur3(), and finally AKAKeyDurSetupFree().  Because
   creating the par shifted trees for each maturity is relatively
   expensive, calling this routine repeatedly with the same
   curve/spread is inefficient.  Use AKABondKeyDur3(), instead. */
AKAKRDURREPORT *AKABondKeyDur2(long pvdate, const AKABOND *bond,
			       long quoteType, double quote,
			       const AKACURVE *curve, const AKASPREAD *spread,
			       int durbp,
			       AKAKRDURREPORT *rpt);


/* AKABondKeyDur() is exactly the same as calling AKABondKeyDur2()
   with the rpt structure being NULL. */
AKAKRDURREPORT *AKABondKeyDur(long pvdate, const AKABOND *bond,
			      long quoteType, double quote,
			      const AKACURVE *curve, const AKASPREAD *spread,
			      int durbp);



/* Compute key-rate durations using SPOT shift.  It must be passed
   an allocated AKAKRDURREPORT with the maturities of the desired key
   rates for dutations, already filled out.  Because it does spot
   shifts of the tree, we recommend using AKABondKeyDur3(), instead. */
long AKABondKRDur(long pvDate, long quoteType, double quote,
		  AKAHTREE hTree, const AKABOND *bond,
		  AKAKRDURREPORT *rpt);


/* Both the following cash flow reports use the same structure.  The
   first simply reports the cash flows from the instrument.  The
   second discounts the flows and calculates bond value
   fundamentals. */

/* report cashflows -- leaves much of the report structure blank */
long AKABondFlowOnly(long pvDate, const AKABOND *bond, AKAFLOWREPORT *rpt);


/* Compute and discount cashflows */
long AKABondFlow(long pvDate, long quoteType, double quote,
		 AKAHTREE hTree, const AKABOND *bond,
		 AKAFLOWREPORT *rpt);

/* Compute yield for each option exercise date in option schedules and to
   maturity.  If both a call and a put are in effect, both are reported.
   The function also returns the lowest of all these yields.  */
long AKAYieldToWorst(long pvDate, long quoteType, double quote,
		     const AKABOND *bond, AKAYLDWORST *rpt);

/* Same as AKAYieldToWorst except that yields are computed to each sink
   date. */
long AKALowestYieldToSink(long pvDate, long quoteType, double quote,
			   const AKABOND *bond, AKAYLDWORST *rpt);

/* Compute yields for each option or flow date of a bond.  The price
   of the bond must be supplied.  The two control flags ascfy and
   tosink extend the behavior.  If tosink is 1 the worst yields are
   for yields to sink dates.  If ascfy is 1 the ytc, ytp, and worst
   yields are on a cash flow basis.  The two flags can be combined.
   If both ascfy is 0 and tosink is 0, the behavior is the same as
   AKABondValueYields().  */
long AKAYieldToWorstEx(long pvDate, long quoteType, double quote,
		       const AKABOND *bond, int tosink, int ascfy,
		       AKAYLDWORST *rpt);

/* Compute price for each option exercise date in option schedules and to
   maturity.  If both a call and a put are in effect, both are reported.
   The function also returns the lowest of all these prices.
   The bond ytm is used as the yield at each date when computing the price.
   If the provided quoteType is not AKA_QUOTE_YTM, the ytm will be calculated
   based on the provided quote. */
long AKAPriceToWorst(long pvDate, long quoteType, double quote,
		     const AKABOND *bond, AKAPRCWORST *rpt);

/* Fill out aftertax yield report -- input fields in report structure
   must be filled out prior to invocation.  Pvdate is purchase date,
   and quote is purchase price. quoteType cannot be OAS. */
long AKAAtaxYield(long pvDate, long quoteType, double quote,
		  const AKABOND *bond, AKAATAXYLD *rpt);

/* Fill out aftertax basis report -- input fields in report structure
   must be filled out prior to invocation.  Pvdate is sale date, and
   quote is sale price. quoteType cannot be OAS. */
long AKAAtaxBasis(long pvDate, long quoteType, double quote,
		  const AKABOND *akabond, AKAATAXBASIS *rpt);

/* Compute yields and modified durations for a bond.  The price of
   the bond must be supplied. */
long AKABondValueYields(long pvdate, double price, const AKABOND *bond,
			AKABONDYIELDREPORT *rpt);

/* Compute yields and modified durations for a bond.  The price of the
   bond must be supplied.  The two control flags ascfy and tosink
   extend the behavior.  If tosink is 1 the worst yields are for
   yields to sink dates.  If ascfy is 1 the ytc, ytp and worst yields
   are on a cash flow basis.  The two flags can be combined.  If both
   ascfy and tosink are 0, the behavior is the same as
   AKABondValueYields().  */
long AKABondValueYieldsEx(long pvdate, double price, const AKABOND *bond,
			  int tosink, int ascfy, AKABONDYIELDREPORT *rpt);


/* ********************************************
   AFTER-TAX VALUATION
   ******************************************** */

/* Enable/Disable all price and OAS calculations performed on this
   bond to be done on an after-tax basis.  This will alter the output
   of most valuation routines except pure yield calculations.  A
   non-zero value to on_off enables, a zero value disables. */
void AKAAfterTax(AKABOND *bond, int on_off);

/* Set the default tax rate used for after-tax valuation of bonds.  Tax-rates
   are as percents, i.e., 15 is 15%.
   If this function is not called the defaults are: 35%, 35%, 15%, 15% */
void AKADefaultTaxRate(double income, double capgain_short, double capgain_long,
		       double capgain_superlong);

/* Set the after-tax rate of a specific bond -- will use defaults otherwise */
void AKABondTaxRate(AKABOND *bond,
		    double income, double capgain_short, double capgain_long,
		    double capgain_superlong);

/* -----------------------------------------------------------------
   Section 5. Miscellaneous Functions
   ----------------------------------------------------------------- */

/* Version information as floating point number in the format v.rr
   where
   v = major version number
   rr = minor release number
   e.g., 1.10 */
double AKA_version();

/* Version information as a string: AKA BondOAS Library version v.rr */
const char *AKA_version_string();

/* Compute par curves at forward times based on rates
   of input tree or curve.  See definition of AKAFWDRATE report below */
long AKAFwdRates(AKAHTREE hTree, AKAFWDREPORT *report);
long AKAFwdRatesFromCurve(AKACURVE *curve, AKAFWDREPORT *report);

/* Probe the term structure of volatility */
long AKAYieldVol(AKAHTREE hTree, AKAVOLREPORT *report);

struct TREESAMPLE;
/* Provide random walks through the expected interest rates in order
   to do sampling valuation analysis. */
int AKA_treesample(AKACURVE *curve, struct TREESAMPLE *sample);

/* Set the duration shift method and basis points - like in AKAINITDATA.
   Permits the user to reset the duration shift paremeters during
   a run of the library.
   MUTLI-THREAD note: this routine is multi-thread safe.
   It will change these settings for the thread only. */
void AKA_set_duration_shift(long smode, double bp_no_options,
			    double bp_with_options);



/* -----------------------------------------------------------------
   Section 6. Error Handling

   Errors during calls to BondOAS routines are pushed on an error stack.
   After the call, the stack can be checked by popping msgs using
   AKA_msg_pop().

   Messages are NOT cleared between calls. The user must clear the stack
   by popping messages or by using AKAFatalReset() which clears everything.

   After a fatal error, results of valuation routines are not defined.

   MUTLI-THREAD note: each thread has its own error message stack.  All
   error handling routines are per thread.  Errors for one thread are
   not visible to any other thread.
   ----------------------------------------------------------------- */

typedef enum {			/* Error Message type enumerations. */
    AKA_MSG_NONE = 0,
    AKA_MSG_INFORMATION = 1,
    AKA_MSG_WARNING = 2,
    AKA_MSG_FATAL = 3
} AKA_MSGTYPE;

#define AKA_MSG_MAXLEN 300	/* max length of an error message string */

/* Pop the messages off the message stack - returns AKA_MSG_NONE when empty
   text must be at least char[AKA_MSG_MAXLEN].
   The order messages are popped is determined by the AKAInitData configuration
   setting, msg_pop_order. */
AKA_MSGTYPE AKA_msg_pop(char *text);

/* Returns TRUE if there is a fatal error on the message stack */
int AKAFatalStatus();

/* Clears the message stack */
void AKAFatalReset();

/* Push a message onto the error message stack */
void AKA_msg_push(char *text, AKA_MSGTYPE mtype);

/* Suppress all messages at or below mtype level.  Returns suppression
   level prior to new setting.  AKA_msg_suppress(AKA_MSG_FATAL) suppresses
   all message.  AKA_msg_suppress(AKA_MSG_NONE) enables all messages. */
AKA_MSGTYPE AKA_msg_suppress(AKA_MSGTYPE mtype);

/* Returns the highest error level of message on the msg queue. */
AKA_MSGTYPE AKA_msg_status();

/* -----------------------------------------------------------------
   Section 7. Memory Allocation and Free
   These routines must be used to allocate and free BondOAS structures.

   All AKA structures which have allocators (e.g., Bond structures)
   must be allocated using the library provided allocation functions,
   only.  Using stack local variables or directly allocating the
   structures via malloc will cause unexpected results and memory
   errors.
   ----------------------------------------------------------------- */

/* forward declarations -- definitions below */
typedef struct AKASecurity AKASECURITY;
typedef struct AKACoupon AKACOUPON;
typedef struct AKAOption AKAOPTION;
typedef struct AKASink AKASINK;

AKACURVE * AKACurveAlloc(long n);
AKACURVE * AKACurveFree(AKACURVE *curve);

AKASPREAD * AKASpreadAlloc(long n);
AKASPREAD * AKASpreadFree(AKASPREAD *spr);

AKABOND * AKABondAlloc(long nCpns, long nCalls, long nPuts, long nSinks);
AKABOND * AKABondFree(AKABOND *bond);

AKASECURITY * AKASecurityAlloc();
AKASECURITY * AKASecurityFree(AKASECURITY *sec);

AKACOUPON * AKACouponAlloc(long n);
AKACOUPON * AKACouponFree(AKACOUPON *cpn);

AKAOPTION * AKAOptionAlloc(long n);
AKAOPTION * AKAOptionFree(AKAOPTION *opt);

AKASINK * AKASinkAlloc(long n);
AKASINK * AKASinkFree(AKASINK *opt);

AKASCEN * AKAScenAlloc(long n);
AKASCEN * AKAScenFree(AKASCEN *scen);

AKAFLOWREPORT * AKAFlowReportAlloc();
AKAFLOWREPORT * AKAFlowReportSubAlloc(AKAFLOWREPORT *rpt, int nFlows);
AKAFLOWREPORT * AKAFlowReportFree(AKAFLOWREPORT *rpt);

AKAKRDURREPORT * AKAKRDurReportAlloc(long n);
AKAKRDURREPORT * AKAKRDurReportFree(AKAKRDURREPORT *rpt);

AKAFWDREPORT * AKAFwdReportAlloc(long nTimes, long nMats);
AKAFWDREPORT * AKAFwdReportFree(AKAFWDREPORT *rpt);

AKAVOLREPORT * AKAVolReportAlloc(long n);
AKAVOLREPORT * AKAVolReportFree(AKAVOLREPORT *rpt);

AKAYLDWORST * AKAYldWorstReportAlloc();
AKAYLDWORST * AKAYldWorstReportFree(AKAYLDWORST *rpt);

AKAPRCWORST * AKAPrcWorstReportAlloc();
AKAPRCWORST * AKAPrcWorstReportFree(AKAPRCWORST *rpt);

struct TREESAMPLE *AKA_treesample_alloc(int nmats, int npaths, int ntimes);
void AKA_treesample_free(struct TREESAMPLE *sample);


/* -----------------------------------------------------------------
   Section 8. Data Structures and Constants

   It is important to read the notes in Section 7., above, about using
   AKA allocation functions.
   ----------------------------------------------------------------- */

/* MULTI-THREAD note: all initialization settings are global and are across
   all threads. */

/* ********************************************
   INITIALIZATION structure with global configuration settings
   ******************************************** */
struct AKAInitData
{
	/* the following three settings are deprecated and ignored */
    char   treePath[512];
    long   maxTrees;
    long   maxTreesPurge;

    long   msg_pop_order;	/* 0 - last on first off (compatibility default)
				   1 - first on first off (recommended) */
    long   enable_tls;		/* controls use of thread local storage
				   -1 -- do not use tls
				    0  -- use default setting for architecture
				    1 -- enable tls
				   The default settings are:
				   linux/linux64 : tls enabled
				   win32/64 lib_md : tls enabled
				   win32/64 lib_mt, dll : tls disabled
				   solaris : ignored, cannot be enabled */

    long   treeCacheSize;	/* deprecated and ignored */

    long   _reserved3;		/* reserved for future use */

	/* duration calculation settings */
    long   durationShift;    /* Select from AKAShiftMode:
				default: AKA_SHIFT_PAR */
    double durBPNoOpts;      /* Duration shift when no options present
				default: 10.0 */
    double durBPOpts;        /* Duration shift when options present
				default: 40.0 */

	/* scenario analysis settings for old style scenario analysis calls */
    double efficiency;       /* Efficiency of option exercise
				default: 100.0 */
};


/* ********************************************
   RATE TERM structure
   ******************************************** */
struct AKACurve	      /* Curve specified at a list of maturities in years */
{
    long    mode;     /* Volatilty type.  Select from AKAVolatilityMode: */
                      /* AKA_VOLMODE_MEANREV:  Supply mean reversion,
			 compute lvol */
                      /* AKA_VOLMODE_LONGVOL:  Supply lvol,
			 compute mean reversion */

    double  vol;      /* Short-rate volatility, 18% represented as 18.0 */
    double  lvol;     /* 30-year rate volatility, 12% represented as 12.0 */
    double  alpha;    /* Mean reversion speed, 2% represented as 2.0 */


    long    type;     /* Select from AKACurveType */

    long    reseverd; /* reserved for future use */

    long    n;        /* Number of entries */

    double *time;     /* Years in appropriate daycount convention */
    double *yield;    /* Rates:  7% as 7.0;  Discount factors:  0.9 as 0.9 */
};


/* ********************************************
   ISSUER SPREAD structure
   ******************************************** */
struct AKASpreads    /* BEY corporate bond spreads */
{
    long    n;       /* Number of points */
    double *time;    /* Time in 30/360 years */
    double *spread;  /* Spread in BEY bp; 25 bp represented as 25.0 */
};


/* ********************************************
   BOND structure, and supporting sub-structures

   It is important to read the notes in Section 7., above, about using
   AKA allocation functions.
   ******************************************** */
struct AKABond
{
    AKASECURITY *sec;

    AKACOUPON   *cpn;   /* Set to NULL for fixed-coupon bond */

    AKAOPTION   *call;  /* Issuer's option */
    AKAOPTION   *put;   /* Holder's option */

    AKASINK     *sink;  /* Sinking fund options */
};


struct AKASecurity
{
    char    name[80];     /* Optional identifier */

    long    idate;        /* Issue date (yyyymmdd) (optional; defaults
			   * to ddate) */
    long    ddate;        /* Dated date (yyyymmdd) */
    long    fcdate;       /* First coupon date (optional; defaults to
			     the first flow date after the idate,
			     determined by the frequency, counting
			     back from the lcdate) */
    long    lcdate;       /* Last regular coupon date (optional;
			   * defaults to maturity) */
    long    mdate;        /* Maturity date */
	/* NOTE:
	   To specify an interest at maturity bond, set the fcdate to
	   the mdate and either leave the lcdate unspecified (zero) or
	   set it to the mdate as well.
	*/
    long    daycount;     /* Select from AKADaycount */
    long    frequency;    /* Select from AKAFrequency */
    long    ex_cpn_days;  /* Number of days bond trades ex-coupon
			   * (optional; defaults to zero)*/
    long    payday;       /* The day of the month on which the coupon
			     is paid.  Optional; defaults to day of
			     the month of the lcdate.  It should be
			     set when the payday is at the end of the
			     month.  For example, if a bond matures on
			     6/30, does it pay interest on 12/30 or
			     12/31?  Set payday to 31 to obtain the
			     latter behavior.  Warning: Setting payday
			     to 30 or 31 for bonds that do not mature
			     on those days will shift the cash flows
			     to those days.  Set to -1 to have library
			     use the last day of the month for the
			     payday only if the maturity date is on
			     the last day of the month, i.e., use
			     payday of 31st if appropriate. */

    double  redemption_value;  /* Usually 100, sometimes not at par.
                                  Useful for valuing pre-refunded bonds that
                                  are defeased to the first call date and the
				  call is at a premium. */

    double  coupon;       /* Fixed coupon, ignored for step-ups */

    long    yld_method;	  /* select from AKA_YIELD_METHOD */

    double  issue_price;    /* defaults to par, only used in after-tax
			       valuation */
    void *taxinfo;   /* configuration data for AKAAfterTax, do
			not assign or access directly */
};


struct AKACoupon  /* Used for step-up coupons */
{
    long    n;        /* Number of coupon/date pairs */
    long    type;     /* Select from AKAPeriodType */
    long   *date;     /* Coupon period boundary dates (see type) */
    double *cpn;      /* Coupons, 7.0% represented as 7.0 */
};


struct AKAOption  /* Used for call and put options */
{
    long    n;        /* Number of strike dates (0 => No option) */
    long    type;     /* Select from AKAOptionType */
    long    delay;    /* Number of days notice required */
    long   *date;     /* Strike dates */
    double *px;       /* Strike prices */
};


struct AKASink  /* Sinking fund option */
{
    long    n;             /* Number of sink dates (0 => No sinking fund) */
    long    delivery;      /* Issuer's option to buy bonds in open market */
                           /* and deliver to trustee to meet SF requirement */
                           /* 1 if present, 0 otherwise */
    long    allocation;    /* Select from AKAAllocation */
    double  face;          /* Original face in dollars */
    double  outstanding;   /* Current amount outstanding in dollars */
    double  acceleration;  /* 100 for a double-up, 200 for triple-up, etc. */
    double  accumulation;  /* Face amount in dollars held by accumulators */
    long   *date;          /* Sink dates */
    double *amt;           /* Sink amounts in dollars */
    double *px;            /* Sink prices (usually 100) */
};


/* ********************************************
   SCENARIO valuation structure
   ******************************************** */
struct AKAScen  /* Simple interest rate scenario */
{
    long      type;   /* Select from AKAScenType */
    long      n;      /* Number of points (n >= 2) */
    long     *dates;  /* Vector of dates (yyyymmdd) */
    AKAHTREE *trees;  /* Vectors of prevailing trees */
};


/* ********************************************
   REPORT BASED VALUATION routines structures
   ******************************************** */
struct AKABondReport
{
	/* price is always calculated (or supplied) */
    double  price;     /* Dollar price as a percentage of par */
	/* accrued is always calculated */
    double  accrued;   /* Accrued as a percentage of par */
	/* option value is calculated if value options is set */
    double  optval;    /* Option value as a percentage of par */
	/* oas is always calculated (or supplied) */
    double  oas;       /* Option-adjusted spread, 25 bp = 25.0 */

	/* duration values are calculated if value duration is set */
    double  effDur;    /* Option adjusted duration in years */
    double  effCon;    /* (vup - 2*v + vdn)/(v*(dr)^2) */
    double  effDV01;   /* (price + accrued)*effDur/100.0 */

	/* yields are calculated if value yields is set */
    double  ytm;       /* Yield to maturity, 7% = 7.0; ignores amortization */
    double  ytc;       /* Yield to next call */
    double  ytp;       /* Yield to next put */
    double  cfy;       /* Cashflow yield; includes amoritzation */
    double  wam;       /* Weighted average maturity in years */

	/* modified duration values are calculated if value yields is set */
    double  modDur;    /* Modified duration in years */
    double  modCon;    /* Modified convexity, same units as effCon */
    double  modDV01;   /* (price + accrued)*modDur/100.0 */
};


struct AKABondYieldReport
{
    double  ytm;       /* Yield to maturity, 7% = 7.0 */
    double  ytc;       /* Yield to call */
    double  ytp;       /* Yield to put */
    double  cfy;       /* Cashflow yield */
    double  wam;       /* Weighted average maturity in years */

    double  modDur;    /* Modified duration in years */
    double  modCon;    /* Modified convexity, same units as effCon */
    double  modDV01;   /* (price + accrued)*modDur/100.0 */
    double  modDurCall; /* Modified duration to call */

	/* to worst values */
    long worstdate;		/* date of worst option exercise */
    double ytw;			/* yield to worst */
    double modDurWorst;		/* modified duration to worst */
    double modConWorst;		/* modified convexity to worst */
};

struct AKAFlowReport
{
    long    nFlows;    /* Number of cashflows remaining */

	/* These bond fundamental values are not calculated
	   by the flow only cash flow function */
    double  oas;       /* Option-adjusted spread */
    double  value;     /* Fair value */
    double  accrued;   /* Accrued interest */
    double  optval;    /* Value of embedded options */
    double  npv;       /* Net present value of scheduled flows */

	/* The remaining elements are arrays of nFlows doubles. */
	/* The following are returned by each cash flow function. */
    long   *date;      /* Cashflow dates (yyyymmdd) */

    double *iflow;     /* Interest flows */
    double *pflow;     /* Principal flows */
    double *tflow;     /* Total cashflow */

	/* These discounted cash flow calculations are not calculated
	   by the flow only cash flow function */
    double *zero;      /* Zero-coupon rate */
    double *factor;    /* Discount factor */
    double *pv;        /* Present values of individual flows */
};


struct AKAFwdReport
{
	/* INPUTS */
    long    nMats;      /* Number of maturities in computed yield curves */
    long    nTimes;     /* Number of forward times at which to solve */
    double  *mats;      /* Maturities for which to solve (e.g., 1yr, 5yr) */
    double  *times;     /* Times at which to solve (e.g., 6mo, 1yr ...)*/
	/* OUTPUT */
    double **fwds;      /* nTimes par curves, each with nMats maturities,
			   i.e., fwds[iTime][iMat] */
};


struct AKAKRDurReport
{
    long    n;         /* Number of maturities supplied (input/output) */
    double  oas;       /* Option-adjusted spread */
    double  value;     /* Fair value */
    double  accrued;   /* Accrued interest */
    double  npv;       /* Net present value of scheduled flows */
    double  effDur;    /* Option adjusted duration in years */
    double  effCon;    /* (vup - 2*v + vdn)/(v*(dr)^2) */
    double *mats;      /* Maturities, in 30/360 years (input/output) */
    double *durs;      /* Key-rate durations */
};


struct AKAScenReport
{
    double cap0;        /* Initial capital (dirty px) per $100 face amount */
    double val0;        /* Initial clean price per $100 face amount */
    double acc0;        /* Initial accrued per $100 face amount */
                        /* NB:  cap0 = val0 + acc0 */

    double interest;    /* Interest received to horizon per $100 face */
    double principal;   /* Principal received to horizon per $100 face */
    double intOnInt;    /* Reinvestment income received to horizon per $100 */

    double cap1;        /* Final capital per $100 face amount, is:
			   val1 + acc1 + interest + principal + intOnInt */
    double val1;        /* Final clean price per $100 face amount */
    double acc1;        /* Final accrued per $100 face amount */

    double totalRet;    /* totalRet = cap1 - cap0 */

    double dur0;        /* Initial effective duration of cap0 */
    double dur1;        /* Final effective duration of cap1, but note that */
                        /* dur of interest + principal + intOnInt is zero, */
                        /* i.e. all intervening flows are cash at horizon */

    double con0;        /* Initial effective convexity of cap0 */
    double con1;        /* Final effective convexity of cap1, but note that */
                        /* con of interest + principal + intOnInt is zero, */
                        /* i.e. all intervening flows are cash at horizon */

    double oas;         /* OAS, either given or computed */

    double balance;     /* Fraction of original balance remaining */
    long   redeemed;    /* Logical OR of flags from AKAScenRedemption */
    long   rDate;       /* Date of option exercise if put or called */
};


struct AKAVolReport
{
    long    n;          /* Number of maturities supplied (input) */
    double  alpha;      /* Mean reversion (output) */
    double *mats;       /* Maturities, in 30/360 years (input) */
    double *vols;       /* Volatilities (output) */
};

struct AKAYldWorst
{
    long *dates;		/* all flows with option exercisable
				   and maturity date */
    double *yields;		/* yield on each of the above dates */
    int n;			/* number of entries in above arrays */
    int worst;			/* index in above arrays with worst yield */
};

struct AKAPrcWorst
{
    long *dates;		/* all flows with option exercisable
				   and maturity date */
    double *prices;		/* price on each of the above dates */
    int n;			/* number of entries in above arrays */
    int worst;			/* index in above arrays with worst price */
};

struct AKAAtaxYldResult {
    double principal;		/* principal, e.g., redemption at maturity */
    double tax;			/* tax on principal */
    double yield;		/* yield implied by final cash flow being
				   principal - tax */
};
	

struct AKAAtaxYld
{
	/* INPUT fields - must be filled out by user */
    double issue_price;		/* price at issue 100 is par */
    double taxrate_short;	/* short term capital gains, 10.0 = 10% */
    double taxrate_long;	/* long ... */
    double taxrate_superlong;	/* super long ... > 5yrs, issued since 1/1/01 */
    double taxrate_income;	/* income tax rate */
	/* OUTPUT fields */
    struct AKAAtaxYldResult ytm; /* after tax yield to maturity */
    struct AKAAtaxYldResult ytc; /* after tax yield to call */
    struct AKAAtaxYldResult ytp; /* after tax yield to put */
};


struct AKAAtaxBasis
{
	/* INPUT fields - must be filled out by user */
    double issue_price;		/* price at issue 100 is par */
    double taxrate_short;	/* short term capital gains, 10.0 = 10% */
    double taxrate_long;	/* long ... */
    double taxrate_superlong;	/* super long ... > 5yrs, issued since 1/1/01 */
    double taxrate_income;	/* income tax rate */
    long purchase_date;		/* date of purchase as yyyymmdd */
    double purchase_price;	/* purchase price */
	/* OUTPUT fields */
    double purchase_basis;	/* bond basis on purchase date */
    double holder_basis;	/* holder's basis on sale date */
    double sale_basis;		/* bond basis on sale date */
    double atax_price;		/* sale price adjusting for taxes */
};


struct TREESAMPLE {
    int nmats;			/* number of maturities - size of mats array*/
    int npaths;			/* number of paths to solve for */
    int ntimes;			/* number of times - size of times array */
    double *mats;		/* array of maturities to solve for */
    double *times;		/* array of times to solve for */
    struct PATH_ITEM ***paths;	/* samples: [<mat-i>][<path-i>][<time-i>] */
};
struct PATH_ITEM {
    double rate;
    double center;
};


/* ********************************************
   USEFUL CONSTANTS
   ******************************************** */
/* Define TRUE and FALSE constants, if they are not already defined. */
#ifndef TRUE
#define TRUE (1==1)
#endif
#ifndef FALSE
#define FALSE (!TRUE)
#endif


enum AKAQuoteType  /* Type of value supplied as input to valuation routines */
{
    AKA_QUOTE_OAS   = 0,
    AKA_QUOTE_PRICE = 1,
    AKA_QUOTE_YTM   = 2,
    AKA_QUOTE_YTC   = 3,
    AKA_QUOTE_YTP   = 4
};

enum AKAShiftMode  /* Shift for duration calculation */
{
    AKA_SHIFT_SPOT = 0,	   /* shift the tree, ie, shift the OAS */
    AKA_SHIFT_PAR  = 1,	   /* shift the par yield curve, compute a new tree 
			      For factor curves, a par rate curve is extracted
			      from the computed tree; this is then shifted */
    AKA_SHIFT_NONE = 2     /* do not calculate duration or convexity */
};

enum AKAVolatilityMode  /* How the volatility input is specified */
{
    AKA_VOLMODE_MEANREV = 0,
    AKA_VOLMODE_LONGVOL = 1
};

enum AKACurveType  /* Yield curve type */
{
    AKA_CURVE_FACTOR = 0,	/* discount factors */
    AKA_CURVE_PAR    = 1,	/* par yields */
    AKA_CURVE_ZERO   = 2,	/* zero coupon rates */
    AKA_CURVE_FORWARD = 3	/* forward rates - do not use for TreeFit() */
};

enum AKADaycount  /* Supported daycounts */
{
    AKA_DAYS_30_360   = 33,
    AKA_DAYS_30E_360  = 34,	/* 30/360 with European treatment of Feb. */
    AKA_DAYS_ACT_360  = 39, 
    AKA_DAYS_ACT_365  = 47,
    AKA_DAYS_ACT_ACT  = 63
};
                            
enum AKAFrequency  /* Supported frequencies */
{
    AKA_FREQ_INT_AT_MATURITY = 0, /* implies an interest at maturity bond
				     with a semi-annual frequency for BEY yields.
				     Set security yld_method to 2,
				     (AKA_YLD_SIMPLE_LAST_PERIOD)
				     for annualized simple interest yield. */
    AKA_FREQ_ANNUAL     = 1,
    AKA_FREQ_SEMIANNUAL = 2,
    AKA_FREQ_QUARTERLY  = 4,
    AKA_FREQ_MONTHLY    = 12
};

enum AKA_YIELD_METHOD {
    AKA_YLD_GLOBAL = 0,   /* use global setting, defaults to discount and can
			     be overridden with deprecated function
			     AKA_set_simple_interest_type() */
    AKA_YLD_BEY = 1, /* discount cash flows using bond equivalent yield */
    AKA_YLD_SIMPLE_LAST_PERIOD = 2, /* use simple interest if in final coupon*/
    AKA_YLD_SIMPLE_LAST_YEAR = 3, /* use simple interest if in final year */
    AKA_YLD_MUNI = 4		/* use municipal bond convention */
};


enum AKAOptionType  /* Supported call and put option types */
{
    AKA_OPTION_EUROPEAN = 0,
    AKA_OPTION_AMERICAN = 1,
    AKA_OPTION_BERMUDAN = 2	/* Deprecated; Use European and specify all call dates */
};

enum AKAAllocation  /* Designation option in sinking funds */
{
    AKA_ALLOC_PRORATA = 0,
    AKA_ALLOC_FRONT   = 1,
    AKA_ALLOC_BACK    = 2
};

enum AKAScenRedemption  /* Type of redemption returned by scenario analysis */
{
    AKA_REDEEM_NONE     = 0,
    AKA_REDEEM_CALL     = 1,
    AKA_REDEEM_PUT      = 2,
    AKA_REDEEM_SINK     = 4,
    AKA_REDEEM_MATURITY = 8
};

enum AKAScenType  /* When scenario shifts occur */
{
    AKA_SCEN_NOW     = 0,  /* Suddenly at time 0 */
    AKA_SCEN_GRADUAL = 1,  /* Linearly in time */
    AKA_SCEN_THEN    = 2  /* Suddenly at horizon */
};

enum AKAPeriodType  /* Whether dates mark beginnings or ends of step-up
		       coupon periods*/
{
    AKA_PERIOD_BEGIN = 0,
    AKA_PERIOD_END   = 1
};


/* -----------------------------------------------------------------
   Section 9. Deprecated Functions
   These routines are supported for backward compatibility
   ----------------------------------------------------------------- */

/* Sets whether to use simple interest calculations if the pvdate is
   within the final pay period or final year.  By default, this is off.
   MULTI-THREAD note: this routine is multi-thread safe.  It will
   change the setting for the invoking thread only. */
enum AKA_SIMPLE_INTEREST_TYPE {
    AKA_SI_OFF,
    AKA_SI_LAST_PERIOD,
    AKA_SI_LAST_YEAR
};

void AKA_set_simple_interest_type(enum AKA_SIMPLE_INTEREST_TYPE it);

#define AKA_set_simple_interest_yield(x) \
	AKA_set_simple_interest_type((x) ? AKA_SI_LAST_PERIOD : AKA_SI_OFF)

#define AKA_set_global_simple_interest_yield(x) \
		AKA_set_simple_interest_yield(x)
#define AKA_set_thread_simple_interest_yield(x) \
		AKA_set_simple_interest_yield(x)

#define AKACouponSpread(pvdate, hTree, bond) AKAAssetSwapSpread(100.0, pvdate, hTree, bond)

/*		  --- old style initialization ---
   Two function interface, authorize_init() must be called before calling
   AKAInit() */

#define AKATerm() AKA_shutdown_configure(0)	/* support old name */

/* Check user authorization to access library.  Returns TRUE if a valid
   key for the user was provided, otherwise FALSE.
   A valid key can be expired. This will show up when init is called */
int AKA_authorize_init(long key, char *uname);

/* Initialize with settings.  If initdata == NULL default settings are used
   returns one of AKAINIT_ERR_ defined above */
long AKAInit(AKAINITDATA *config);

/*
  The following two functions handled calculating duration outside of
  the standard bond valuation functions, AKABondVal() and
  AKABondVal2().  This was due to inefficiencies in computing duration
  shifts due to the handling of the shifted trees needed in the
  valuations.  This is no longer the case and they should not be used.
  ABABondVal() or AKABondVal2() should be used instead.
 */

/* Create two shifted trees suitable for computing duration.  The
   input is the original tree and the basis points to shift.  The
   bpShift is passed as a pointer, is always a positive number, and
   may be updated in the function.  The down-shifted and up-shifted
   trees are returned in the pointer reference parameters passed into
   the routine.  The trees are fitted by shifting the par curve
   by bpShift basis points.  If this fails, the bpShift value is
   adjusted until the trees can be successfully fitted.
   A tree will fail to fit when shifting causes
   excessively low rates.
   This routine returns 1 on success and 0 on failure.
 */
int AKATreeFitShift2(AKAHTREE tree, double *bpShift,
		     AKAHTREE *treeup, AKAHTREE *treedown);


/* Calculate the convexity and duration values assuming that the bond
   report is already filled out with the other data (price, OAS,
   accrued).  For use with AKA_SHIFT_NONE.  The hTree_up and
   hTree_down trees are assumed to be trees which were created by
   shifting the yield curve durbp basis points up and down.  These
   trees can be created by shifting the original curve and calling
   AKATreeFit() or by using AKATreeFitShift().  The price and OAS
   values in the rpt are assumed to have come by valuing the bond with
   the unshifted tree. */
long AKABondDuration(long pvdate, AKAHTREE hTree_up, AKAHTREE hTree_down,
		     double durbp, const AKABOND *bond, AKABONDREPORT *rpt);



/* Fit a tree from a set of curves */
/* ********************************************
   INTEREST RATE structure alternative form composed from known prices
   of bonds and zero coupon rates
   ******************************************** */
struct AKARun
{
    long    mode;    /* Select from AKAVolatilityMode: */
                     /* AKA_VOLMODE_MEANREV:  Supply mean reversion,
			compute lvol */
                     /* AKA_VOLMODE_LONGVOL:  Supply lvol,
			compute mean reversion */

    double  vol;     /* Short-rate volatility, 12% represented as 12.0 */
    double  lvol;    /* 30-year rate volatility, 10% represented as 10.0 */
    double  alpha;   /* Mean reversion speed, 2% represented as 2.0 */

    long      pvdate;        /* Required to price instruments, */
                             /* but tree is independent of time. */

    long      nBonds;        /* Number of bonds */
    AKABOND **bonds;         /* Optionless bond specifications */
    double   *bondPx;        /* Clean bond prices */

    long      nRates;        /* Number of zero coupon rates */
    long     *rateDaycount;  /* Appropriate daycount type */
    long     *rateMat;       /* Discount note maturities */
    double   *rate;          /* Discount rates */
};
typedef struct AKARun AKARUN;
/* -----------------------------------------------------------------
   This function is no longer supported.  It will always return a zero
   (failed) tree and generate an error message
   ----------------------------------------------------------------- */
AKAHTREE AKATreeFitRun(const AKARUN *crv, const AKASPREAD *spr);
AKARUN * AKARunAlloc(long nBonds, long nRates);
AKARUN * AKARunFree(AKARUN *run);

#ifdef __cplusplus
}
#endif

#endif
