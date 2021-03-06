#ifndef _AKAERRNO_H_
#define _AKAERRNO_H_
/* akaerrno.h (r. 1.7) */

/* ERROR NUMBERS */
enum AKA_ERROR_NUMBER {
	/* ** INITIALIZATION ERRORS ** */
    AKA_ERROR_NONE,		/* no error */
    AKA_ERROR_AUTHORIZATION,	/* authorization did not run successfully */
    AKA_ERROR_MEMORY,		/* memory allocation error -- should not occur */
    AKA_ERROR_EXPIREDKEY,	/* authorization key has expired */
    AKA_ERROR_UNINITIALIZED,	/* not initialized or shutdown */

	/* ** FEATURE PERMISSION ERRORS ** */
    AKA_ERROR_PERMISSION_LATTICE, /* not authorized to use lattice functions */
    AKA_ERROR_PERMISSION_ADVANCED_LATTICE, /* not authorized to use advanced lattice functions */

    AKA_ERROR_PERMISSION_SCENARIOS, /* not authorized to use scenario analysis functions */
    AKA_ERROR_PERMISSION_ATAX,	/* not authorized to use after-tax functions */

	/* ** VALUATION ERRORS ** */
    AKA_ERROR_COMPUTE_COUPON,	/* failed to compute coupon spread */
    AKA_ERROR_COMPUTE_KEYRATES,	/* failed to compute key rate durations */
    AKA_ERROR_COMPUTE_OAS,	/* failed to compute OAS from price */
    AKA_ERROR_COMPUTE_PRICE,	/* failed to compute price from OAS */
    AKA_ERROR_COMPUTE_YIELD_ATAX, /* failed to compute after-tax yield */

    AKA_ERROR_TREEFIT,		/* failed to construct interest rate model */

    AKA_ERROR_INTERNAL,		/* internal library failure -- this should never occur */

	/* ** INPUT ERRORS ** */
    AKA_ERROR_TREESAMPLE,	/* invalid treesample structure as input */
    AKA_ERROR_HTREE,		/* invalid interest rate model as input */

    AKA_ERROR_COUPON,		/* invalid coupon as input, must be > 0 */
    AKA_ERROR_QUOTETYPE,	/* invalid quote type as input */
    AKA_ERROR_OAS,		/* invalid OAS as input */
    AKA_ERROR_PRICE,		/* invalid price as input */
    AKA_ERROR_YIELD,		/* invalid yield as input */

    AKA_ERROR_MATURED,		/* pvdate is after maturity */
    AKA_ERROR_RETIRED,		/* deprecated, will not occur*/


    AKA_ERROR_CURVE,		/* invalid curve as input */
    AKA_ERROR_DATE,		/* invalid date input */
    AKA_ERROR_DURSHIFT,		/* invalid duration shift basis points as input, must be >= 1 and < 300, must also not be so high that is causes very negative rates */
    AKA_ERROR_FACEAMOUNT,	/* invalid face amount as input*/
    AKA_ERROR_FCDATE,		/* invalid first coupon date as input */
    AKA_ERROR_IDATE,		/* invalid initial date as input */
    AKA_ERROR_LCDATE,		/* invalid last coupon date as input */
    AKA_ERROR_MATURITY,		/* invalid maturity as input */
    AKA_ERROR_OPTIONDATE,	/* invalid option date as input */
    AKA_ERROR_OPTIONPRICE,	/* invalid option price as input */
    AKA_ERROR_PURCHASEDATE,	/* invalid purchase date as input */
    AKA_ERROR_PVDATE,		/* invalid Pvdate as input */
    AKA_ERROR_SALEDATE,		/* invalid sale date as input */
    AKA_ERROR_SINKDATE,		/* invalid sink date as input */
    AKA_ERROR_SINKPRICE,	/* invalid sink price as input */

	/* ** RECOVERABLE INPUT ISSUES **  */
    AKA_WARN_SINK_ALLOCATION,	/* invalid sinking fund allocation, using prorata */
    AKA_WARN_SINK_UNDESIGNATED,	/* sink allocated amount could not be fully designated */
    AKA_WARN_FREQUENCY,		/* invalid coupon frequency as input, using semi-annual */
    AKA_WARN_DAYCOUNT,		/* invalid day count, using 30/360 */
    AKA_WARN_OPTIONDELAY,	/* option delay less than zero or greater than bond frequency, adjusting to 30 days or bond frequency */
    AKA_WARN_OUTSTANDING_HIGH,	/* input amount outstanding is greater than remaining flows, reducing amount outstanding accordingly */
    AKA_WARN_PAYDAY,		/* invalid payday, using none */
    AKA_WARN_SINKSUM_HIGH,	/* sum of sinking fund schedule exceeds input face amount, trusting schedule */
    AKA_WARN_SINKSUM_LOW,	/* sum of sinking fund schedule is less than input face amount, adding remainder to final flow */
    AKA_WARN_TRADEDATE,		/* invalid trade date in packed date, ignoring trade date */
    AKA_WARN_YIELDMETHOD,	/* invalid yield method, using BEY */

	/* ** RECOVERABLE VALUATION ISSUES ** */
    AKA_WARN_IMPERFECT_COUPON,	/* imperfect fit of coupon spread */
    AKA_WARN_IMPERFECT_OAS,	/* imperfect fit of OAS to price */


    AKA_WARN_TOOMANY,		/* too many warnings */

    AKA_WARN_AMORTIZATION,	/* amortization period before maturity */
    AKA_WARN_SINK_TOO_SOON,     /* sink schedule retires bond before maturity, either schedule, maturity, or face amount are wrong */
    AKA_WARN_OUTSTANDING_LOW,	/* input amount outstanding is less than 0.1% of face, disregarding input and trusting schedule */
    AKA_WARN_LCDATE_NONCYCLICAL /* last regular coupon date not cyclical with first coupon date, ignoring last coupon date */
};

/* maximum number of warnings that will be generated by any operation */
#define AKA_WARNINGS_MAX 12

#endif
