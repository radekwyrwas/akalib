/* -------------------------------------------------------------------------
 * Copyright (c) 2013, Andrew Kalotay Associates.  All rights reserved. *
 ------------------------------------------------------------------------- */

#ifndef _AKAAPI_COMPATIBILITY_HPP_
#define _AKAAPI_COMPATIBILITY_HPP_

/* akaapi_compatibility.hpp (r. 1.5) */

#include "akaapi.hpp"

extern "C" {
#include "akaapi.h"
} /* close of extern C */

namespace AndrewKalotayAssociates {
#ifdef FIX_AUTOINDENT_NEVER_DEFINED
}  /* this is here so editors/pretty-printers don't indent on above { */
#endif

/* -----------------------------------------------------------------
   Overview

   The C language API is defined in akaapi.h.  The C++ API provides
   all the functionality of the C API, however, it may be useful when
   migrating code to use to both APIs in a mixed model.  The
   compatibility layer support this approach.
   ---------------------------------------------------------------------- */

/*  allows akaapi C API functions to be intermixed with the C++ API. */
class Compatibility {
public:
	/* Provide C API interest rate tree handle so that C API
	   functions can be used.  Solve() must have been successfully
	   invoked on the InterestRateModel or else a tree handle of
	   zero (no tree) will be returned.  IMPORTANT: This method
	   should only be used to get the AKAHTREE as a temporary.
	   Using a Set{x}() method on, or deleting the InterestModel
	   object which returned the TreeHandle invalidates the
	   returned handle.  The returned TreeHandle should never be
	   released (i.e., do not call AKATreeRelease()).  */
    static AKAHTREE CApiTreeHandle(const InterestRateModel &);

	/* Provide C style structure so that C API functions can be
	   used.  This returns a constant pointer to constant memory.
	   The memory points to an AKA Library C AKABOND structure.
	   IMPORTANT: This method should only be used to get the C
	   structure as a temporary.  Using a Set{x}() method on, or
	   deleting the Bond object which returned the AKABOND
	   structure invalidates the returned structure.  The returned
	   structure should never be freed (i.e., do not call
	   AKABondFree()). */ 
    static const AKABOND* const CApiBond(const Bond &);

	/* Provide C style structure so that C API functions can be
	   used.  This returns a constant pointer to constant memory.
	   The memory points to an AKA Library C AKACURVE structure.
	   IMPORTANT: This method should only be used to get the C
	   structure as a temporary.  Using a Set{x}() method on, or
	   deleting the InterestRateModel object which returned the
	   AKACURVE structure invalidates the returned structure.  The
	   returned structure should never be freed (i.e., do not call
	   AKACurveFree()). */ 
    static const AKACURVE* const CApiCurve(const InterestRateModel &);

	/* Create an InterestRateModel from a C API AKACURVE.  The
	   resulting InterestRateModel still needs to be solved.  The
	   returned object is a new object and may be freely assigned
	   and used. */
    static InterestRateModel ModelFromCurve(const AKACURVE *);

	/* Create a C++ API Bond from a C API AKABOND.  The returned
	   object is a new object and may be freely assigned and
	   used. */
    static Bond BondFromBOND(const AKABOND *);
};

//   ----------------
} // end of namespace


 /* -----------------------------------------------------------------
    Section 1. Calling C Functions with C++ Objects

    Invoking a C API function using C++ objects is supported with
    Compatibility methods.  For example:

    Date pvdate;
    InterestRateModel model;
    Bond bond(...);
    ... // initialize the objects
    model.Solve();

    AKABONDREPORT report;    	// old style report structure

    AKABONDVAL(pvdate.Libdate(), AKA_QUOTE_PRICE, 100,
               Compatibility::CApiTreeHandle(model),
	       Compatibility::CApiBond(bond), &report);
    ---------------------------------------------------------------------- */

 /* -----------------------------------------------------------------
    Section 2. Creating C++ Objects from C Structures

    The C++ API objects can be created from already initialized
    C structures.  For example:

    AKACURVE *curve = AKACurveAlloc(10);
    AKABOND *cbond = AKABondAlloc(1, 0, 0, 0);
    ... // initialize the structures

    InterestRateModel model = Compatibility::ModelFromCurve(curve);
    AKACurveFree(curve); // curve may be freed, it is not referenced in model
    model.Solve();	 // model must be solved before it can be used

    Bond bond = Compatibility::BondFromBOND(cbond); // assign from C structure
    AKABondFree(cbond); // cbond may be freed, it is not referenced in bond

    Value value(bond, model, Date(2013, 10, 12)); // create value object
    ---------------------------------------------------------------------- */

/* -----------------------------------------------------------------
   Section 3. Mixed Model Issues

   There are two potential problems when mixing the C and C++ APIs.

   The library initialization must be done using the C++ API
   Initialization object or the C++ API will not function correctly.
   The C++ initialization will enable the C API.

   Error handling in the C API is on a global basis.  If a C function
   is used the C API error interfaces AKAError() and AKAWarnings()
   must be called immediately afterward or any errors/warnings will
   be lost or confused with subsequent operations.
   ----------------------------------------------------------------- */

/* -----------------------------------------------------------------
   Section 4. C++ API to C API Correspondence
   
   In most cases the values stored in C report structures have a
   recognizable one-to-one correspondence to methods of the Value
   object.  The method names follow C++ naming conventions and
   so vary slightly from the C structure member names.

   a) The method names are consistently title capitalized.  E.g., the
   C report structure member accrued becomes the method Accrued() and
   price becomes Price().

   b) The method names are fully instantiated.  E.g., the C report
   structure member ytc becomes the method YieldToCall() and modDur
   becomes ModifiedDuration().

   The remainder of this document describes cases where there is not
   an exact correspondence.
   ---------------------------------------------------------------------- */

/* -----------------------------------------------------------------
   Section 5. Key Rate Durations

   The key rate duration report AKAKRDURREPORT provides duration at
   specific maturities defined as input to the C key rate functions.
   The C key rate functions have two behaviors.  First, the tenting
   (see the KeyRateDuration() method documentation in akaapi.hpp) is
   from input maturity to input maturity.  Second, the durations are
   scaled after they are computed so that the sum of the durations
   equals the overall duration.  To use the C++ API and still get the
   same values as returned by the C function it is necessary to
   implement both of these behaviors.  The following code snippet is
   an example:

{
    double sumdur = 0;
    for (int i = 0; i < matcnt; i++) {
	double left = (i == 0) ? -1000 : mats[i - 1];
	double right = (i == matcnt - 1) ? 1000 : mats[i + 1];
	sumdur +=
	    results[i] = value.KeyRateDuration(oas, durbp, mats[i], 
					       left, right);
    }
    double sumratio = sumdur / value.EffectiveDuration(oas, durbp);
    for (int i = 0; i < matcnt; i++)
	results[i] /= sumratio;
}
   ---------------------------------------------------------------------- */

/* -----------------------------------------------------------------
   Section 6. Flow Reports

   All the flows for an instrument can be gotten by using the Flow()
   access methods in Value.  These can be used as is or discounted
   using the Discount() method.  The Scenario Analysis section below
   has an example of how to walk the flows.
   ---------------------------------------------------------------------- */

/* -----------------------------------------------------------------
   Section 7. Scenario Analysis

   The Value::AnalyzeScenario() provides the redemption information
   and the reinvestment income.  The remainder of the values can be
   calculated using other Value methods.  The initial price, accrued,
   duration, etc., is returned by the same Value object used for
   AnalyzeScenario().  The horizon values can be computed by creating
   a new Value object with the horizon Interest Rate Model.  The
   summed principals and coupon income at the horizon can be computed
   by walking the bond flows.  For example:

    double outstanding = 1;
    double remaining = 1; // these two handle sinking funds
    for (int f = 1; f < value.FlowCount(); f++) {
	if (value.FlowDate(f) > horizondate)
	    break;
	if (f < value.FirstFlow())  // calculate outstanding
	    outstanding -= value.Principal(f);
	else {
	    if (f == value.FirstFlow()) // keep track of remaining
		remaining = outstanding;
		// always get coupon on the flow
	    couponincome += value.Interest(f);
	    if (f == analysis.flowwhen) { // done
		    // if balance is gone, get any accrued
		couponincome += value.Accrued(f);
		    // default final price is sink or redemption
		double atprice = value.PrincipalPrice(f);
		if (analysis.redeemed == ScenarioAnalysis::CALL)
		    atprice = value.CallPrice(f);
		else if (analysis.redeemed == ScenarioAnalysis::PUT)
		    atprice = value.PutPrice(f);
		    // note, using remaining in case of sinking fund
		principalincome += atprice *
		    remaining / outstanding;
		remaining = 0;
		break;
	    }
	    else {	// just a regular flow, only for sinks
		principalincome += value.PrincipalPrice(f) *
		    value.Principal(f) / outstanding;
		remaining -= value.Principal(f);
	    }
	}
    }

    Please see additional scenario analysis example code in the
    example directory of the release.
    ---------------------------------------------------------------------- */

#endif // ifndef _AKAAPI_COMPATIBILITY_HPP_
