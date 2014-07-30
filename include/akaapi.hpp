/* -------------------------------------------------------------------------
 * Copyright (c) 2013, Andrew Kalotay Associates, Inc..  All rights reserved. *
 ------------------------------------------------------------------------- */

#ifndef _AKAAPI_HPP_
#define _AKAAPI_HPP_
/* akaapi.hpp (r. 1.22) */

namespace AndrewKalotayAssociates {
#ifdef FIX_AUTOINDENT_NEVER_DEFINED
}  /* this is here so editors/pretty-printers don't indent on above { */
#endif

/* -----------------------------------------------------------------
   Overview
   
   This file contains the class definitions for using the C++ API to
   BondOAS(tm).  This file also serves as documentation for the API.
   It is arranged in the following sections:

   1. General Notes
   2. Error Handling
   3. Initialization
   4. Interest Rate Model Definition
   5. Bond Definition
   6. Valuation
   7. Date Handling
  ----------------------------------------------------------------- */


/* -----------------------------------------------------------------
   Section 1. General Notes

   BondOAS(tm) is a multi-thread safe C++ API.  The interface is
   through a small set of classes.  These classes are described below.

   There are four primary classes:
       1. Initialization -- initializes library functions
       2. InterestRateModel -- defines an interest rate environment
       3. Bond -- defines a bond
       4. Value -- values a bond given an interest rate environment

   All error handling is managed through the Status class which is inherited
   by each other class.

   A simple program has the following steps.

   // The Initialization object must be successfully created and authorized
   Initialization init;
   if (init.Authorize(key, name) > 0)
       exit(init.Error());
   
   // Create both an interest rate model and a bond object
   Bond bond("bullet", Date(2000, 1, 1), Date(2030, 1, 1), 2.0); // 30yr, 2%
   InterestRateModel model;
   model.SetRate(1, 2.0);	// set a flat 2% rate
   model.Solve();
   if (model.Error() > 0)
       exit(model.Error());

   // Now value the bond
   Value value(bond, model, Date(2013, 7, 1));
   if (value.Error() > 0)
       exit(value.Error());
   double price = value.Price(0); // price at OAS 0

   ----------------------------------------------------------------- */


/* -----------------------------------------------------------------
   Section 2. Error Handling

   All major classes inherit from the Status class.  It supports
   retrieving status/errors/warnings about the state of an object.
   See akaerrno.h for a C enum of the AKA library error codes.
   ----------------------------------------------------------------- */
class Status {
protected:
    class StatusData *statusdata;
public:
    Status();
    ~Status();
public:					 // all are const methods
    int Error() const; // returns AKA Error Number of error (or zero, none)
    int WarningCount() const;
    int Warning(int idx) const;	// returns AKA Error Number of warning

	/* converts a numeric status code to a string */
    static const char *ErrorString(int);
	// get errors/warnings in string form
    const char *ErrorString() const { return ErrorString(Error()); };
    const char *WarningString(int idx) const {
	return ErrorString(Warning(idx));
    };

    /* Clear any error and warnings.  In most use cases there should be
       no need to call this method as each class inheriting from this
       object clears messages before each of its operations/methods. */
    void Clear();
};

// forward declaration
class Date;

/* -----------------------------------------------------------------
   Section 3. Initialization.

   The Initialization object must be successfully authorized with a
   valid key in order to use the library.  All operations will fail if
   the library is not initialized or is shutdown.  By default, the
   library is shutdown (de-initialized) when the Initialization object
   goes out of scope or is deleted.  This behavior can be inhibited by
   ShutdownAutomatically(false), see below.  If automatic shutdown is
   disabled the library can be shutdown via the static Shutdown()
   method.  The Shutdown() method should otherwise never be called.
   The behavior of multiple concurrent initializations is undefined
   and should be avoided.  Use of valid library objects subsequent to
   shutdown is also undefined and should be avoided.
   ----------------------------------------------------------------- */
class Initialization : public Status {
private:     // disallow copy, assignment
    Initialization(const Initialization &) { };
    Initialization & operator=(const Initialization &) { return *this; };
public:
	/* Initialize the library but do not check authorization.
	   All library operations will fail until authorization is done. */
    Initialization();
	/* Initialize and authorize, call the appropriate authorization
	   method, below, and set Error(). */
    Initialization(long key, const char *uname);
    Initialization(const char *keyfilename);

    ~Initialization();		// by default library is shutdown

    void ShutdownAutomatically(bool); // set auto-shutdown behavior
    static void Shutdown();	// shutdown library when not done automatically

	/* Returns error code or 0 on success. */
    int Authorize(long key, const char *uname);

	/* Authorize getting key and user-name from a key file with
	   key and user-name in that order.  White space between the
	   key and the user-name is ignored, as is trailing white
	   space after the user-name.  Any read or parsing errors
	   cause initialization to fail with
	   AKA_ERROR_AUTHORIZATION.  Returns error code or 0 on success. */
    int Authorize(const char *keyfilename = "./akalib.key");

    /* Initialization inherits from Status.  Error() must be checked
       after instantiation.  No library operations will work if the
       this object has an error.  Nor will they work if this object
       is not instantiated. */

    Date Expiration() const; // when the provided key expires as a Date
    static double Version();   // library version as a number
    static const char *VersionString(); // library version as a string
};


/* -----------------------------------------------------------------
   Section 4. Interest Rate Model Definition

   InterestRateModel is the solved representation of future interest
   rates states based on an interest rate curve and volatility.
   ----------------------------------------------------------------- */
class InterestRateModel : public Status {
private:
    friend class Value;
    friend class ValueAfterTax;
    friend class InterestRateScenario;
    friend class Compatibility;
    class InterestRateModelData *data;
public:
    enum INPUTTYPE { FACTORCURVE, PARCURVE };
	// note, for Zero curves use FACTORCURVE and SetZeroRate()
    InterestRateModel(INPUTTYPE = PARCURVE);
    ~InterestRateModel();

	/* Copy Constructor

	   Using a forward period greater than zero creates a new
	   interest rate model with rates projected forward that
	   number of years. The resulting new curve will be of type
	   PARCURVE.  It will not be already solved.

	   A simple copy (no forward rate passed) of a solved model
	   creates a new copied model that is also already solved.
	   There is no cost to solving the new model. */
    InterestRateModel(const InterestRateModel &, double forward_period = 0);

    InterestRateModel & operator=(const InterestRateModel &);

	// ********* Curve Configuration
	/* all Set{X}() methods return true/false.  If the value is
	   false the set operation has been ignored.  A false value is
	   only due to bad/invalid inputs given to the method. */

	/* Set a rate on the par curve, 7% as 7.0.  Will fail if year is
	   <= 0, rate is too low, or curve is a FACTORCURVE */
    bool SetRate(double year, double rate);
	/* Set a factor on the discount curve.  Will fail if year is
	   <= 0, factor is too low, or curve is a PARCURVE */
    bool SetFactor(double year, double factor);
	/* Set a rate using the zero rate.  Will fail if year is
	   <= 0, rate is too low, or curve is not a FACTORCURVE */
    bool SetZeroRate(double year, double zerorate);

	// remove the curve point, either par or factor -- always succeeds
    void RemovePoint(double year);
    void RemoveAllPoints();

	// returns the par rate at the year, interpolated if not explicitly set,
	// a year <= 0 causes Value::BadValue to be returned.
    double GetRate(double year) const;
	// like GetRate() but returns the discount factor
    double GetFactor(double year) const;

	/* If the model input type (Factor or Par) does not match the
	   type of rate requested (GetRate() vs. GetFactor()) the
	   model must first be successfully solved using Solve().
	   Otherwise, Value::BadValue will be returned. */

	/* Par rates and discount factors are interpolated
	   differently.  Par rates are straight line interpolated
	   while discount factors are fit exponentially.  This
	   produces smooth curves in each case.  However, when
	   converting between a one curve type and another you must
	   specify points at semi-annual intervals or the different
	   interpolation approaches will result in slightly different
	   curves.  That is, if you specify new points using the
	   rates/factors gotten from a curve of a different type, get
	   and use points for each six month interval. */

	// convert between zero and factor rates
    static double ZeroToFactor(double rate, double year);
    static double FactorToZero(double discount, double year);

	// short-rate volatility, 18% represented as 18.0, initialized to zero
    bool SetVolatility(double);
    double GetVolatility() const;

	/* Interest rate model long volatility (30 year) is either
	   determined by a mean reversion factor or by being
	   set explicitly.  In the former case the short-term
	   volatility and the mean reversion together determine the
	   long-term volatility.  In the latter case the mean
	   reversion is set explicitly.  These are mutually exclusive.
	   The default behavior is to use a mean reversion of zero. */

	// mean reversion speed, 2% represented as 2.0, initialized to zero
    bool SetAlpha(double);	// forces use of mean reversion
    double GetAlpha() const;

	// explicit long volatility, 18% is represented as 18.0
    bool SetLongVolatility(double); // forces use of explicit long volatility

	/* The long volatility is either explicitly set or solved for
	   based on the mean reversion.  See note above.  If
	   set explicitly this method returns the user set value.  If
	   it must be solved for (mean reversion mode) then the
	   interest rate model must be solved (see Solve() below)
	   before the long volatility can be returned. */
    double GetLongVolatility() const;

	// get the input curve type -- set in constructor
    INPUTTYPE GetInputType() const;
    bool IsParCurve() const { return GetInputType() == PARCURVE; };

	/* Solve for the interest rate model from the current
	   configuration.  Solve() must be invoked successfully
	   before the InterestRateModel may be used in a Value
	   constructor.
	   Solve() returns the value of Error().  This will be
	   AKA_ERROR_NONE except if a) initialization failed, b)
	   permission is denied, c) curve is incomplete (no points),
	   d) the curve points describe an insolvable interest
	   environment.	*/
    int Solve();

	/* Discount from the time in the future expressed as fractional years,
	   returns Value::BadValue unless Solve() ran successfully */
    double Discount(double value, double fromtime, double oas = 0) const;

	/* Returns the volatility at a time.
	   Returns Value::BadValue unless Solve() ran successfully */
    double GetVolatility(double fromtime) const;

	/* Deprecated interface */
    void EnableCompatibility();	// does nothing
};


/* -----------------------------------------------------------------
   Section 5. Bond Definition

   The Bond object contains all the properties of a fixed income
   security.  It provides for a minimal initial definition and has
   Set methods in order to specialize the instrument.

   Error checking note: only the constructor can raise an error.  The
   only error conditions are failed initialization or bad dates or
   coupon as arguments to the constructor.  All the Set{} methods will
   return true/false as described below.  In either case they will not
   set/reset the Error() status.
   ----------------------------------------------------------------- */
class Bond : public Status {
private:
    friend class Value;
    friend class ValueAfterTax;
    friend class Compatibility;
    class BondData *data;
public:
    Bond(const char *name, const Date &idate, const Date &mdate, double coupon);
    ~Bond();
    Bond(const Bond &);
    Bond & operator=(const Bond &);

	// ********** methods to set bond features **********

	/* all Set{X}() methods return true/false.  If the value is
	   false the set operation has been ignored.  A false value is
	   only due to bad/invalid values given to the method.  All
	   dates must be valid dates and between the initial date and
	   the maturity date.  In addition, all operations will fail
	   if the constructor failed. The Error() status will not be
	   updated by the Set{X}() methods. */
    enum DAYCOUNT { // needed for yield and accrued interest calculation
        DC_30_360,  /* each month has 30 days and a year has 360 days,
		       applies to most corporate, municipal, and agency bonds */
	DC_30E_360, // 30/360 with European treatment of Feb.
	DC_ACT_360, /* days in month are actual but year has 360 days,
		       applies to U.S. Treasury bills */
	DC_ACT_365, /* days in month are actual but year has 365 days,
		       applies to some CDs */
	DC_ACT_ACT  /* days in month are actual and coupon period days are
		       actual, applies to U.S. Treasury notes and bonds, and
		       some European sovereign debt issues */
    };
    bool SetDaycount(DAYCOUNT); // constructor defaults to 30/360

    enum FREQUENCY {
	FREQ_INT_AT_MATURITY, /* interest pays at maturity but yield is
				 calculated on a semi-annual basis. */
	FREQ_ANNUAL,
	FREQ_SEMIANNUAL,
	FREQ_QUARTERLY,
	FREQ_MONTHLY
    };
    bool SetFrequency(FREQUENCY); // constructor defaults to semi-annual

    bool SetRedemptionPrice(double);	// ... par

	/* Note on pre-refunded bonds.  These must be characterized as
	   optionless bonds maturing on the "pre-refunded to"
	   date. They should not include calls and puts that may have
	   been part of the original bond.  The pre-funded call price
	   should be used as the redemption price. */

    bool SetIssuePrice(double);	// ... par
	/* Note, for after-tax valuation (see MuniOAS package) it
	   is necessary to set the issue price of Original Issue
	   Discount (OID) and Original Issue Premium (OIP) bonds in
	   order for valuation to be accurate. */

	/* face amount, used in sinking funds to verify schedule;
	   value of zero means trust schedule; constructor defaults
	   to zero.
	   For mortgages, this method must be used to set the mortgage
	   amount.  A nominal value of 100 will otherwise be used. */
    bool SetFaceAmount(unsigned long);
    bool SetFaceAmount(double);	// for very large numbers
    
	/* The next two methods allow specification of odd first and
	   last coupons.  They are necessary only if these coupons are
	   odd.  The last coupon is the final coupon before maturity.
	   It is not necessary to specify the last coupon if a first
	   coupon is specified and the last coupon is not longer than
	   one regular coupon period. */
    bool SetFirstCoupon(const Date &); // set first coupon
    bool SetLastCoupon(const Date &);  // set penultimate coupon

	/* Configure the End-Of-Month (EOM) rule.  By default, EOM is
	   disabled.  If EOM is enabled the cyclical coupon payment
	   date will always fall on the end of the month.  For
	   example, assume a semiannual bond that matures on 6/30 (or
	   has a last coupon date of 6/30, if specified). It will pay
	   on 6/30 and 12/30 with EOM disabled (the default).  It will
	   pay on 6/30 and 12/31 (note, not 12/30) with EOM enabled.
	   The EOM setting is ignored if the payment schedule would
	   never fall on the end of the month.  A semiannual bond
	   paying on 6/15 will always pay on 12/15 regardless of the
	   EOM setting. */
    bool SetPayDayEndOfMonth(bool);
    bool SetPayDay(int); // supports interface like C API (see documentation)

    enum YIELD_METHOD {
	YLD_BEY = 1, /* Discount cash flows using bond equivalent
			yield.  Example for semiannual 30/360, the
			yield (y) explaining the input price is given
			by applying discount factors to each cashflow
			as 1/(1+y/2)^(days to cashflow/180) */
	YLD_SIMPLE_LAST_PERIOD, /* Default method.  If the pvdate is
				   in the last coupon period then the
				   yield is calculated on a
				   simple-interest basis, otherwise
				   BEY. */

	YLD_SIMPLE_LAST_YEAR, /* If the pvdate is within the last year
				 of a bond's life, then simple
				 interest is used for the cash flows
				 occurring after the pvdate, otherwise
				 BEY. (Used for some Canada bonds) */
	    
	YLD_MUNI /* Like simple_last_period but has some special
		    treatment if bond has odd first or last coupons.*/
    };
    bool SetYieldMethod(YIELD_METHOD); // set the yield method

	// add call/put/sink options
    bool SetCall(const Date &, double px);
    bool SetPut(const Date &, double px);
    bool SetSink(const Date &, double amount, double px = 100);
    bool SetCoupon(const Date &, double);
    bool SetXCouponDays(int);   /* Number of days bond trades ex-coupon
				   (optional; defaults to zero), see
				   notification settings, below. */

	// remove the call/put/sink/coupon for the date
    void RemoveCall(const Date &);
    void RemovePut(const Date &);
    void RemoveSink(const Date &);
    void RemoveCoupon(const Date &);

	// configure the call/put options

	/* Notice period must be >= 0 and < coupon frequency,
	   constructor defaults to 30 days, the notice is the same for
	   puts and calls.  See notification settings, below. */
    bool SetNoticePeriod(int);
    bool SetOptionDelay(int d) { return SetNoticePeriod(d); };

    bool SetCallAmerican(bool is_american); // constructor defaults to true
    bool SetPutAmerican(bool is_american); // constructor defaults to false
	/* there are no known examples of American puts but the library
	   accommodates such a case */

	/* ALLOCATION configures the designation option in sinking
	   funds.  It specifies how bonds purchased by issuer at a
	   discount to satisfy sink are allocated */
    enum ALLOCATION  {
	ALLOC_PRORATA, // on a pro-rata basis across sink schedule
	ALLOC_FRONT,   /* to the sink amounts after the pvdate, satisfying
			  as much of the amount as available */
	ALLOC_BACK     // like front, but working back from maturity
    };

	/* Configure the sinking fund options.  Allow delivery allows
	   purchase of discount bonds by issuer to meet sink, defaults
	   to false.  Acceleration allows the issuer to sink more than
	   the sink amount on a sink date.  Acceleration is a percent
	   where 100 is 100%.  100% allows issuer to sink twice the
	   amount on the schedule on a given date, 200% three times
	   the amount, The default is 0%.  Allocation is explained
	   above. */
    bool SetSinkOption(bool allow_delivery, // .. false
		       double acceleration, // .. 0%
		       ALLOCATION allocation); // ... pro-rata

	// coupon schedule dates can mark when coupon
	// begins or ends (reset at, or until)
    bool SetCouponSchedulePeriod(bool at_begin); // default is true


	/* Configure notification period behavior.  This is used for
	   both option notification and ex-coupon days. */
    enum NotifyBehavior {
	/*  Extend the end of the notification/ex-coupon period so
	    that it falls on a a business day.  This is the default
	    behavior.  Example, for a currently callable American call
	    with 30 days required notice, if the 30th day is a weekend
	    (or holiday, if holiday list supplied) then push the next
	    possible call opportunity to the next business day. */
	BusinessDaysExtend,

	/* Only count business days when determining the notification
	 * date. */
	BusinessDaysOnly
    };
	// configure the behavior, returns prior setting
    NotifyBehavior SetNotifyBehavior(NotifyBehavior);

	/* Set weekends to be business days, returns prior setting.
	   The default setting, for historical compatibility, is true.
	   Note, if weekends are business days and no holidays are set
	   (see below), notification is simply calendar days. */
    bool SetBusinessDayWeekends(bool);
	// set a day to be a non-business day, true if date is valid
    bool SetHoliday(const Date &);
	// set an array of dates to be holidays, int must contain the count
    bool SetHolidays(const Date *, int);
    bool ClearHolidays(); // remove all set holidays, true unless bond invalid

	/* Configure a bond to behave like a mortgage.

	   This configures the option and sink schedules so that the
	   bond behaves like a mortgage.  Existing sink and option
	   schedules are removed.  Step Up/Down coupon schedules are
	   also removed.  Subsequent calls to any Set methods for
	   options or sinks will return false and fail.  The exception
	   is SetNoticeDays(), see next.

	   SetMortgage() will set the frequency to Monthly unless it
	   has already been set by SetFrequency().  SetFrequency() may
	   also be used after the call to SetMortgage() to reset it.

	   Use SetFaceAmount() to set the mortgage amount.

	   SetMortgage() will always succeed */
    bool SetMortgage();

	/* Set mortgage amortization years.  This will return true if
	   SetMortgage() has been invoked and the years are greater
	   than the maturity of the bond.  Otherwise, it will return false. */
    bool SetAmortization(double years);

	/* Set mortgage refinancing fees as a percent of par for
	   "calling" the mortgage.  Fees are expressed as points, 1.5%
	   is 1.5.  This will return true if SetMortgage() has been
	   invoked.  Otherwise it will return false. */
    bool SetMortgageRefinanceCost(double);


	/* Clears mortgage settings and restores functionality of Set methods
	   for Options and Sinks. */
    void ClearMortgage();

	// return basic setup
    const char *GetName() const;
    Date GetInitialDate() const;
    Date GetMaturityDate() const;
    double GetCoupon() const;
    DAYCOUNT GetDaycount() const;
    FREQUENCY GetFrequency() const;
    double GetFaceAmount() const;
};

/* -----------------------------------------------------------------
   Section 6. Valuation

   All library valuation results are provided by the Value object.

   A Value object can be created with or without an interest rate
   model.  Some valuation methods require that an interest model
   be present.
   ----------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   The following classes define complex values returned by some methods
   of the Value class (defined below).  These classes could/should be
   nested in Value however swig which is used to generate cross
   language interfaces does not support nested classes.
   ---------------------------------------------------------------------- */

class Duration {  // used to return complex value of duration and convexity
public:
    Duration(double d, double c) : duration(d), convexity(c) {};
    double duration;
    double convexity;
	// so can assign to a double and get just duration
    operator double() { return duration; };
};

class ScenarioAnalysis {	// the result of a scenario analysis
public:
    enum REDEEMED { NOTREDEEMED, CALL, PUT, SINK, MATURITY } redeemed;
    int flowwhen;	    // index of flow when redeemed, or 0 if not
    double efficiency;	    // efficiency of redemption (call or put)
    double reinvestment;    // value at horizon of intermediary coupon and
			    // principal flows

    static const char *redeemstring(REDEEMED);
    const char *redeemstring() const { return redeemstring(redeemed); };
};

/* -----------------------------------------------------------------
   The following classes define complex inputs to Value methods
   ----------------------------------------------------------------- */
// forward declare -- see below
class SinkingFundStatus;
class InterestRateScenario;

class Quote { // used to specify input price or other pricing value
public:
    double quote;
    enum QuoteType { OAS, PRICE, YTM, YTC, YTP } quotetype;
    Quote(double q, QuoteType qt) : quote(q), quotetype(qt) {};
};

/* ----------------------------------------------------------------------
   Value provides all library valuation functionality (except for after-tax)

   Note on caching and efficiency: this class caches values.  Thus,
   calling Value::<method>() is free the second time.  The values
   cached are based on the last valid OAS.  The OAS is reset each call
   to Value::OAS() or Value::Price() and the cache is flushed.
   ---------------------------------------------------------------------- */
class Value : public Status {
protected:     // disallow copy, assignment
    Value(const Value &) { };
    Value & operator=(const Value &) { return *this; };

protected:
    class ValueData *data;

public:
	// construct a Value without an InterestRateModel, not all valuation
	// methods are supported
    Value(const Bond &, const Date &pvdate);
    Value(const Bond &, const Date &pvdate, const Date &tradedate);

	// construct a Value with an InterestRateModel
    Value(const Bond &, const InterestRateModel &, const Date &pvdate);
    Value(const Bond &, const InterestRateModel &, const Date &pvdate,
	  const Date &tradedate);


	// construct a Value with an InterestRateModel and additional sinking
	// fund information
    Value(const Bond &, const InterestRateModel &, const SinkingFundStatus &,
	  const Date &pvdate);
    Value(const Bond &, const InterestRateModel &, const SinkingFundStatus &,
	  const Date &pvdate, const Date &tradedate);

    /* ********* Tradedate and Settlement Days

       Each constructor comes in two flavors: with and without a
       tradedate.  Specifying a tradedate affects the option notice
       period and ex-coupon days in effect at the pvdate.  The days
       are counted from the tradedate rather than the pvdate.  The
       tradedate must be less than the pvdate and no more than 31 days
       before the pvdate.  Invalid tradedates cause a warning and are
       then ignored.  A tradedate equal to the pvdate is the same as
       no tradedate and is silently accepted.  Note, pvdate is
       synonymous with settlement date. */

    ~Value();			// destructor

    /* Use the Error() method inherited from the Status object to
       check the state after the last operation.  This must be checked
       after construction and reset().  Warnings() should also be
       checked.  Most subsequent operations will succeed if the
       construction succeeded.  Operations after initialization may
       also produce warnings. */

    /* Like the constructors, returns Error() for convenience. */
    int Reset(const Bond &, const Date &pvdate);
    int Reset(const Bond &, const Date &pvdate, const Date &tradedate);
    int Reset(const Bond &, const InterestRateModel &, const Date &pvdate);
    int Reset(const Bond &, const InterestRateModel &, const Date &pvdate,
	      const Date &tradedate);
    int Reset(const Bond &, const InterestRateModel &,
	      const class SinkingFundStatus &,
	      const Date &pvdate);
    int Reset(const Bond &, const InterestRateModel &,
	      const class SinkingFundStatus &,
	      const Date &pvdate, const Date &tradedate);

	// ********* constants
    static const double BadValue; /* returned by all valuation routines
				     if the valuation failed.  Error()
				     should be checked for an explanation */

	// ********* Bond fundamentals, do not require an interest rate model

	/* returns number of coupon/option/principal flows, 0 indicates
	   a bond configuration error, last flow is always maturity */
    int FlowCount() const;

    enum FLOWTYPE {  // a flow will be one or more the following
	INTEREST = 1,		// coupon payment
	PRINCIPAL = 2,		// maturity or sinking fund date
	CALLOPTION = 4,	// call option date (and subsequent flows if American)
	PUTOPTION = 8,	// put ...
	PSEUDO_REGULAR = 16	/* for irregular coupons this indicates
				   the date of where the regular coupon would
				   fall, it is not an actual interest payment */
    };
    int FlowType(int) const; // returns bitwise-or'd flags of one or more
				//     of the above types
    bool FlowType(int idx, FLOWTYPE t) const { return (FlowType(idx) & t) != 0; };
    int FirstFlow() const;	// index of first flow after pvdate
    Date FlowDate(int) const;	// date of flow
    double YearsTo(int) const;	// year fractions to flow from pvdate
    double Interest(int) const;	// interest payment on flow
    double Principal(int) const; /* principal payment on flow,
				    normally 100% (1) on maturity, 0 otherwise,
				    but sinking funds will have principal
				    flows as a % of the face value
				    (e.g., 10% = .1). */
    double PrincipalPrice(int) const; /* price at which principal flow occurs,
					 0 if not a principal flow,
					 normally 100, but can be sink price
					 or redemption value other than 100. */
    double Accrued(int) const; // accrued on flow, 0 for interest flows
    double CallPrice(int) const; // call price at flow or BadValue
    double PutPrice(int) const; // put price at flow or BadValue
    const char *BondName() const; // name of bond

	// ********* PV Date based values, do not require an interest rate model
    double Accrued() const;	// accrued at pvdate
    int AccruedDays() const;	// accrued days on pvdate

    double AccruedOn(const Date &);

	// ********* Yields, do not require an interest rate model
    bool SetYieldMethod(Bond::YIELD_METHOD); // set the yield method, cached
					     //    yield values will be flushed
    Bond::YIELD_METHOD GetYieldMethod() const; /* get current yield method,
						  default is how Bond was set */

    double YieldToCall(double price);
    double YieldToPut(double price);
    double YieldToMaturity(double price);

	/* given a price, return the worst yield at that price,
	   the second form sets the index of the worst flow,
	   if also_to_sink is true, sinking fund principal flows
	   are included as candidates */ 
    double YieldToWorst(double price, bool also_to_sink = false);
    double YieldToWorst(double price, int &flowindex,
			bool also_to_sink = false);
    double WeightedAverageMaturity(double price);
    Duration ModifiedDuration(double px);

	// Calculate yields to a particular flow -- will return BadValue
	// unless the flow has an option or principal payment.
	// Note, this result is not cached.
    double YieldToFlow(int, double price, bool as_cashflow_yield = false);
	// given a price and a yield, compute modified duration to a flow,
	// like YieldToFlow(), flow must have an option or principal payment
    Duration ModifiedDurationToFlow(int, double price, double yield);

	// convert yields to a price, can also use Price(Quote) method
    double ytm2price(double);
    double ytc2price(double);
    double ytp2price(double);

	// ********* Complex Valuations, require an interest rate model
    double Price(double oas);
    double Price(Quote);
    double OAS(double price);
    double OAS(Quote);

    double OptionValue(double oas);
    double OptionValue(Quote);

	/* Calculate price to a particular flow given a yield to that
	   flow -- will return BadValue unless the flow has an option
	   or principal payment.  Note, this result is not cached. */
    double PriceToFlow(int, double yield, bool as_cashflow_yield = false);

	/* given a yield, return the worst price at that yield,
	   the second form sets the index of the worst flow,
	   if also_to_sink is true, sinking fund principal flows
	   are included as candidates */ 
    double PriceToWorst(double yield, bool also_to_sinks = false);
    double PriceToWorst(double yield, int &flowindex,
			bool also_to_sinks = false);

	// effective duration and convexity
    Duration EffectiveDuration(double oas, double durationbp = 30);
    Duration EffectiveDuration(Quote, double durationbp = 30);
	// one sided durations
    double UpDuration(double oas, double durationbp = 30);
    double DownDuration(double oas, double durationbp = 30);
    double UpDuration(Quote, double durationbp = 30);
    double DownDuration(Quote, double durationbp = 30);

	/* Key rate duration measures the sensitivity of the bond to a
	   shift in a specified year's yield.  Key rate years will be
	   rounded to semi-annual.  The key rate is not suddenly
	   shifted, rather rates around it are "tented" with the key
	   year at the center.  The tenting process keeps rates
	   unchanged at anchor points (years) on either side of the
	   key rate and interpolates the rates between the anchor
	   points and the key rate.  The different flavors of
	   KeyRateDuration(), below, control the anchor points. */

	// anchor at years +/- sqrt(year) rounded to semi-annual
    Duration KeyRateDuration(double oas, double durationbp, double year);
    Duration KeyRateDuration(Quote, double durationbp, double year);

	// anchor at years +- width, rounded to semi-annual
    Duration KeyRateDuration(double oas, double durationbp, double year,
			     double width);
    Duration KeyRateDuration(Quote, double durationbp, double year,
			     double width);

	// anchor at specified years, rounded to semi-annual
    Duration KeyRateDuration(double oas, double durationbp, double year,
			     double leftanchoryear, double rightanchoryear);
    Duration KeyRateDuration(Quote, double durationbp, double year,
			     double leftanchoryear, double rightanchoryear);
			     
	// see InterestRateModel::Discount(), GetRate(), GetFactor()
    double Discount(double value, double fromtime, double oas = 0) const;
    double GetRate(double year) const;
    double GetFactor(double year) const;

	// scenario Analysis, if false, Error() has been set
    bool AnalyzeScenario(const InterestRateScenario &, double oas,
			 ScenarioAnalysis &);
    bool AnalyzeScenario(const InterestRateScenario &, Quote,
			 ScenarioAnalysis &);

	/* Solve for a single fixed spread such that if the same
	   spread is added to each coupon in the coupon schedule of
	   the bond, then the clean price of the bond, computed using
	   the tree with zero OAS, is equal to clean_price provided as
	   input. */
    double AssetSwapSpread(double price);

	/* Compute the I-Spread.  This is the difference between the
	   YTM of a bond at the provided price, and the yield on the
	   weighted-average-maturity-matched point on the provided
	   yield curve. The yield will be straight-line interpolated
	   between points on the provided curve. Industry refers to
	   this spread as I-Spread if swap curve provided and G-Spread
	   if government curve (treasuries) provided. */
    double ISpread(double price);
};


/* ----------------------------------------------------------------------
   support classes for Value
   ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   Sinking fund bonds have additional market information that determines
   pricing.  This information is only required/meaningful for sinking
   fund bonds and is optional even then.
   Note: doubles are used to accommodate very large numbers
   ---------------------------------------------------------------------- */
class SinkingFundStatus {
public:
    double outstanding;    /* Outstanding amount at pvdate.  This may be
			    necessary for accurate sinking valuation
			    if an incomplete sinking fund schedule is
			    provided or there has been acceleration.
			    If not provided, the outstanding amount
			    will be the sum of the remaining schedule
			    sinks, i.e., there has been no acceleration. */
    double accumulation; /* Amount in dollars held by accumulators, if
			    not provided, will be zero. */
public:
    SinkingFundStatus(unsigned long os, unsigned long ac) : outstanding(os), accumulation(ac) {};
    SinkingFundStatus(double os, double ac) : outstanding(os), accumulation(ac) {};
	// construct so uses default behavior
    SinkingFundStatus() : outstanding(-1), accumulation(0) {};
};

 /* ----------------------------------------------------------------------
    Define an interest rate scenario for analysis.  An interest rate
    scenario requires at minimum one interest rate model.  The base
    Value class interest rate model is transformed over time into the
    models in the scenario.  The bond is valued at each of its flows
    in order to determine whether it is redeemed.
 ---------------------------------------------------------------------- */
class InterestRateScenario {
protected:     // disallow copy, assignment
    InterestRateScenario(const InterestRateScenario &) { };
    InterestRateScenario & operator=(const InterestRateScenario &) {
	return *this;
    };
protected:
    friend class Value;
    class InterestRateScenarioData *data;

public:
    enum SCENTYPE {   // When scenario shifts occur
	NOW,  /* Suddenly at time 0 */
	GRADUAL,  /* Linearly in time, this can be slow  */
	THEN  /* Suddenly at horizon */
    };

	// create with at least one interest rate environment at
	// years in the future, years does not have to be a whole number
    InterestRateScenario(double years, const InterestRateModel &horizonmodel,
			 SCENTYPE = NOW);
    ~InterestRateScenario();

	/* add an interest rate environment between the start and the
	   horizon to fine tune how interest rates transition over
	   time.  Will return false if InterestRateModel is unsolved */
    bool AddTransition(double years, const InterestRateModel &);

	// efficiency threshold for option exercise, default is 100%
	// must be > 0 and <= 100
    bool SetEfficiencyThreshold(double);

	/* Scenario analysis is normally based on input yield curves
	   and the input OAS. */
    void SetReinvestStandard();	// default behavior

	/* The initial OAS may not be appropriate for subsequent curves
	   in the scenario.  The user can suppress using the OAS after
	   the start date. */
    void SetReinvestAtZeroOAS();

	/* Finally, the reinvestment can be done at a fixed rate.  In this
	   case option decisions are still made using the OAS */
    void SetReinvestAtFixed(double);
};

/* -----------------------------------------------------------------
   Section 6. Date Handling

   This object handles validation and construction of a date
   representation from a variety of forms.  It also provides simple
   date math.  The constructor will always succeed, however invalid
   input will result in the invalid date of zero.

   Note, the C library API expresses dates as 8-digit longs in the
   form yyyymmdd.  This object also supports translation from/to
   that form.
   ----------------------------------------------------------------- */
class Date {
private:
    long _data;
public:
	// order of pieces passed to constructor
    enum ENTRY { YMD, /* sortable */ MDY, /* American */ DMY /* European */ };
    Date(int, int, int, ENTRY = YMD ); // construct from pieces

    /* construct a date from a string ordered via entry and with the
       pieces separated by sep.  If sep is not found assumes (for
       historical reasons) that string is yyyymmdd and ignores ENTRY. */
    Date(const char *, ENTRY, char sep = '/');

    Date(long = 0);		// construct a date from a long

	// construct a date by adding fractional years according to daycount
	// to the give Date.  Years can be negative.
    Date(const Date &, double yrs, Bond::DAYCOUNT);

    bool Good() const;		// whether date is valid

	// C API date format, yyyymmdd as a long
    long Libdate() const { return _data; };

    int DayOf() const { return ((int) (_data) % 100); };
    int MonthOf() const { return (((int) (_data) / 100) % 100); };
    int YearOf() const { return ((int) (_data) / 10000); };

	// returns time from date to other (i.e. other - this) in
	// fractional years according to the daycount
    double YearsTo(const Date &other, Bond::DAYCOUNT) const;
	// compute calendar days between dates
    int DaysTo(const Date &other) const;

    Date & operator +=(int);
    Date & operator -=(int days) { return *this += -days; };

    bool operator <(const Date &other) const {
	return Libdate() < other.Libdate();};
    bool operator <= (const Date &other) const {
	return Libdate() <= other.Libdate();};
    bool operator == (const Date &other) const {
	return Libdate() == other.Libdate();};
    bool operator >= (const Date &other) const {
	return Libdate() >= other.Libdate();};
    bool operator > (const Date &other) const {
	return Libdate() > other.Libdate();};
};

//   ----------------
} // end of namespace
#endif
