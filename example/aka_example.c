/* -------------------------------------------------------------------------
 * Copyright (c) 2012, Andrew Kalotay Associates.  All rights reserved. *
   This example code is provided to users of the AKA Library.

   see usage() below
   ------------------------------------------------------------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#ifdef _MSC_VER  /* include implementation of getopts -- see end of file */
const char *optarg = NULL;
int optind = 0;
int getopt(int, char *const *, const char *);
#else
#include <unistd.h>
#endif

#include "akaapi.h"

/* different ways of generating the test curve based on initial input rate */
enum CTYPE {
    FLAT,
    LINEAR,
    ASYM
};

/* forward declarations */
void init(const char *);
int msgs();
AKACURVE *make_curve(double rate, double vol, enum CTYPE ctype);
AKABOND *make_bond(long, double);
AKAOPTION *make_call(long odate);
long pvdateadd(long pvdate, int adddays);
long akadatecnv(const char *date);
void usage();
static int set_taxrates(const char *taxrates);
#define INSECS(x) ((double) (x) / CLOCKS_PER_SEC)

/* -----------------------------------------------------------------
   Purpose: start here
   Returns: 
   ----------------------------------------------------------------- */
int
main(int argc, char *argv[])
{
	/* aka structures */
    AKABOND *bond = NULL;
    AKACURVE *curve = NULL;
    AKAHTREE htree = 0;
	/* user options */
    int c;
    int cnt;

    double rate, coupon;
    long pvdate = 20000101;
    int quiet = FALSE;
    enum CTYPE ctype = FLAT;
    long mdate = 20300101;
    double vol = 0;
    int bullet = FALSE;
    int days = 1;
    int valueonly = TRUE;
	/* by default get the price for an OAS of zero; "fair value" */
    enum AKAQuoteType input_qtype = AKA_QUOTE_OAS;
    double quote = 0;
    const char *keyfile = NULL;
    
	/* timing variables */
    clock_t start = 0;
    int timing = FALSE;

    while((c = getopt(argc, argv, "a:bc:d:fm:p:qT:tv:z"))!= EOF) {
	switch(c) {
	    case 'a' :
		keyfile = optarg;
		break;
	    case 'b' :
		bullet = TRUE;
		break;
	    case 'c' :
		switch(tolower(*optarg)) {
		    case 'f' :
			ctype = FLAT;
			break;
		    case 'a' :
			ctype = ASYM;
			break;
		    case 'l' :
			ctype = LINEAR;
			break;
		}
		break;
	    case 'd' :
		days = atoi(optarg);
		break;
	    case 'f' :
		valueonly = FALSE;
		break;
	    case 'm' :
		mdate = akadatecnv(optarg);
		break;
	    case 'p' :
		pvdate = akadatecnv(optarg);
		break;
	    case 'q' :		/* get the OAS from a quoted price*/
		input_qtype = AKA_QUOTE_PRICE;
		quote = 100;	/* use price of par */
		break;
	    case 'T' :
		if (set_taxrates(optarg) != 1)
		    return 1;
		break;
	    case 't' :
		timing = TRUE;
		quiet = TRUE;
		break;
	    case 'v' :
		vol = atof(optarg);
		break;
	    case 'z':
		quiet = TRUE;
		break;
	    default :
		usage();
		return 0;
		break;
	}
    }
    argc -= optind;
    argv += optind;

    if (argc == 0) {
	usage();
	return 1;
    }
    rate = atof(argv[0]);
    if (argc > 1)
	coupon = atof(argv[1]);
    else
	coupon = rate;
    if (rate < 1 || rate > 30) {
	printf("rate must be in range of 1 to 30\n");
	return 1;
    }
    if (coupon < 1 || coupon > 30) {
	printf("coupon must be in range of 1 to 30\n");
	return 1;
    }

	/* init the library */
    init(keyfile);

	/* make the curve, then the tree, then free the curve */
    curve = make_curve(rate, vol, ctype);
    if (quiet == FALSE) {
	int i;
	printf("Making a par rate curve with %.2g volatility",
	       vol);
	for (i = 0; i < curve->n; i++)
	    if (curve->time[i] == 1 || curve->time[i] == 30)
		printf(", %0.0f yr = %.2f%%",
		       curve->time[i], curve->yield[i]);
	printf("\n");
    }
    rate /= 100;		/* now make into real rate */
    if (timing)
	start = clock();
    htree = AKATreeFit(curve, NULL);
    if (timing) {
	start = clock() - start;
	printf("Seconds to fit the base curve = %0.2f\n", INSECS(start));
    }
    if (!msgs())
	return 1;
    AKACurveFree(curve);
    curve = NULL;

	/* make the bond */
    if (quiet == FALSE)
	printf("Making a 30 year %.10g%% bond maturing on %ld",
	       coupon, mdate);
    bond = make_bond(mdate, coupon);
    if (!bullet) {
	long cdate = bond->sec->ddate + 50000;
	if (quiet == FALSE)
	    printf(" callable %ld at par", cdate);
	bond->call = make_call(cdate);
    }
    if (quiet == FALSE)
	printf("\n\n");


    if (quiet == FALSE) {
	const char underline[] = "--------------------";
	const char *fmt = "%10.10s %8.8s %8.8s %8.8s %8.8s %8.8s";
	printf(fmt, "pvdate  ", input_qtype == AKA_QUOTE_OAS ? "price" : "oas",
	       "accrued", "optval", "duration", "convex.");
	printf("\n");
	printf(fmt, underline, underline, underline,
	       underline, underline, underline);
	printf("\n");

    }

	/* loop through pvdates */
    start = clock();
    for (cnt = 0; pvdate <= mdate && cnt < days;
	 pvdate = pvdateadd(pvdate, 1), cnt++) {
	AKABONDREPORT rpt;
	AKABondVal2(pvdate, input_qtype, quote, htree, bond, &rpt,
		    !valueonly, !valueonly, !valueonly);
	if (quiet == FALSE) {
	    printf("%02ld/%02ld/%04ld %8.3f %8.3f %8.3f %8.3f %8.3f",
		   (pvdate / 100) % 100, pvdate % 100, pvdate / 10000,
		   input_qtype == AKA_QUOTE_PRICE ? rpt.oas : rpt.price,
		   rpt.accrued, rpt.optval, rpt.effDur, rpt.effCon);
	    printf("\n");
	    if (!msgs())
		return 1;
	}
    }
    if (timing)
	printf("\nSeconds to value the bond for %d pvdates = %0.2f\n",
	       cnt, INSECS(clock() - start));
	
    AKABondFree(bond);
    AKATreeRelease(htree);
    AKA_shutdown();
    return 0;
}

/* -----------------------------------------------------------------
   Purpose: display usage messate
   Returns: nothing
   ----------------------------------------------------------------- */
void
usage()
{
    printf("Purpose: value a 30 year bond\n");
    printf("Usage: [FLAGS] <discount-rate> [<coupon> : defaults to rate]\n");
    printf(
	"\nFlags:\n"
	"\t-a key-file -- load akalib key from file, default ./akalib.key\n"
	"\t-b -- bond is bullet bond\n");
    printf(
	"\t-c <curve-type> -- make the curve be:\n"
	"\t\tflat -- same rate (default)\n"
	"\t\tlinear -- grow linearly (slowly)\n"
	"\t\tasymptotic -- grow (quickly) but level off\n");
    printf(
	"\t-d <cnt> -- set number of pvdates to value (default 1)\n"
	"\t-f -- full valuation, include duration, option value, and accrued\n"
	"\t-m <mdate> -- set bond maturity date, default 1/1/2030\n"
	"\t-p <pvdate> -- set initial pvdate to value, default bond dated date\n");
    printf(
	"\t-q -- use price of 100 as quote, default use oas of zero\n"
	"\t-T <arg> -- set tax rates as: income[,short[,long[,superlong]]]\n"
	"\t-t -- display timings\n"
	"\t-v <vol> -- set curve volatility, default zero\n"
	"\t-z -- silent mode, no output, for timing\n");
    printf("AKA library version: %.2f\n", AKA_version());
}

/* -----------------------------------------------------------------
   Purpose: set up curve
   Returns: pointer to allocated curve structure
   ----------------------------------------------------------------- */
AKACURVE *
make_curve(double rate, double vol, enum CTYPE ctype)
{
    static double terms[] = { .5, 1, 3, 5, 7, 10, 15, 30 };
    AKACURVE *curve;
    unsigned int i;

    curve = AKACurveAlloc(sizeof(terms) / sizeof(terms[0]));
    for (i = 0; i < sizeof(terms) / sizeof(terms[0]); i++) {
	curve->time[i] = terms[i];
	if (i == 0) {
	    curve->yield[0] = rate;
	    continue;
	}
	switch(ctype) {
	    case LINEAR :
		curve->yield[i] = rate + terms[i] *.01;
		break;
	    case ASYM :
		if (terms[i] < 1)
		    curve->yield[i] = rate;
		else
		    curve->yield[i] = rate + 2 * (1 - 1.0 /terms[i]);
		break;
	    case FLAT :
	    default :
		curve->yield[i] = rate;
		break;
	}
/*	printf("%0.2f -> %0.2f\n", curve->time[i], curve->yield[i]); */
    }
    curve->mode = AKA_VOLMODE_MEANREV;
    curve->type = AKA_CURVE_PAR;
    curve->alpha = 0;
    curve->vol = vol;
    return curve;
}
        
/* -----------------------------------------------------------------
   Purpose: set up bond
   Returns: pointer to allocated bond structure
   ----------------------------------------------------------------- */
AKABOND *
make_bond(long mdate, double coupon)
{
    AKABOND *bond = AKABondAlloc(0, 0, 0, 0);
    bond->sec->coupon = coupon;
    bond->sec->ddate = mdate - 300000; /* 30 year */
    bond->sec->mdate = mdate;
    bond->sec->daycount = AKA_DAYS_30_360;
    bond->sec->frequency = AKA_FREQ_SEMIANNUAL;

    return bond;
}

/* -----------------------------------------------------------------
   Purpose: 
   Returns: 
   ----------------------------------------------------------------- */
AKAOPTION *
make_call(long odate)
{
    AKAOPTION *call = AKAOptionAlloc(1);
    call->delay = 30;
    call->type = AKA_OPTION_AMERICAN;
    call->date[0] = odate;
    call->px[0] = 100;
    return call;
}

/* -----------------------------------------------------------------
   Purpose:
   Returns: true iif year is a leap year
   ----------------------------------------------------------------- */
static
int
isleap(int yr)
{
    int ret = FALSE;

    if ((yr % 4) == 0) {	/*  maybe leap */
	if (yr % 400 == 0)	/*  400 is exception to 100 exception */
	    ret = TRUE;
	else if (yr % 100 == 0)	/*  exception every 100 years */
	    ret = FALSE;
	else
	    ret = TRUE;		/*  otherwise, regular leap */
    }
    return ret;
}


/* -----------------------------------------------------------------
   Purpose: 
   Returns: last day in a given month and yr (yr is for leap years)
   ----------------------------------------------------------------- */
static
int
lastday(int mn, int yr)
{
    static int _DAYSIN[] =
	{ 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int month_days;
    month_days = _DAYSIN[mn];
    
    if (mn == 2 && isleap(yr))	/*  leap year check */
	month_days += 1;
    return month_days;
}



/* -----------------------------------------------------------------
   Purpose: add days to a pvdate
   Returns: new pvdate as long
   ----------------------------------------------------------------- */
long
pvdateadd(long pvdate, int adddays)
{
    int yr, mon, day;

    if (adddays <= 0)		/* only adds */
	return pvdate;

    day = pvdate % 100;
    mon = (pvdate / 100) % 100;
    yr = pvdate / 10000;

    day += adddays;
    while (day > lastday(mon, yr)) {
	day -= lastday(mon, yr);
	mon += 1;
	if (mon > 12) {
	    mon = 1;
	    yr += 1;
	}
    }
    return yr * 10000 + mon * 100 + day;
}


/* -----------------------------------------------------------------
   Purpose: pop all the messages of the stack and put them out
   Returns: 1 if no errors else 0
   ----------------------------------------------------------------- */
int
msgs()
{
    enum AKA_ERROR_NUMBER errornumber = AKAError();
    enum AKA_ERROR_NUMBER warnings[AKA_WARNINGS_MAX];
    int wcnt = AKAWarnings(warnings);
    int i;
    for (i = 0; i < wcnt; i++)
	fprintf(stdout, "%2d \"%s\"\n", 2, AKAErrorString(warnings[i]));
    if (errornumber > AKA_ERROR_NONE)
    	fprintf(stdout, "%2d \"%s\"\n", 3, AKAErrorString(errornumber));
    return errornumber == AKA_ERROR_NONE;
}

/* -----------------------------------------------------------------
   Purpose: try and get the key from a file
   Returns: nothing -- all errors exit
   ----------------------------------------------------------------- */
void
readkey(const char *fname, long *key, char *uname, int namesize)
{
    FILE *fp;
    char linestr[200];

    *key = 0;
    memset(uname, 0, namesize);

    fp = fopen(fname, "r");
    if (fp == NULL) {
	fprintf(stderr, "Error: unable to open akakey file %s\n", fname);
	exit(1);
    }
    else if (fgets(linestr, sizeof(linestr), fp) == NULL ||
	     (*key = atol(linestr)) == 0) {
	fprintf(stderr, "Error: missing key line from akakey file %s\n",
		fname);
	fclose(fp);
	exit(1);
    }
    else if (fgets(linestr, sizeof(linestr), fp) == NULL) {
	fprintf(stderr, "Error: missing user name line from akakey file %s\n",
		fname);
	fclose(fp);
	exit(1);
    }
    else {
	fclose(fp);
	strncpy(uname, linestr, namesize - 1);
	for (namesize-- ; namesize > 0; namesize--) {
	    if (uname[namesize] != '\0') {
		if (isspace((int) uname[namesize]))
		    uname[namesize] = '\0';
		else
		    break;
	    }
	}
    }
}
/*-----------------------------------------------------------------
  Purpose: init what need to run
  -----------------------------------------------------------------*/
void
init(const char *keyfile)
{
    long key;
    char uname[100];
    AKAINITDATA config;
    AKA_initialize_get_defaults(&config);
    config.durBPNoOpts = config.durBPOpts = 30;

    if (keyfile == NULL)
	keyfile = "akalib.key";

    readkey(keyfile, &key, uname, sizeof(uname));
    if (AKA_initialize_configure(key, uname, &config) != 0) {
	fprintf(stderr, "Error: library initialization failed\n");
	exit(1);
    }
}

/* -----------------------------------------------------------------
   Purpose: make an aka date from the standard mm/dd/yyyy format
   Returns: AKADATE
   ----------------------------------------------------------------- */
long
akadatecnv(const char *date)
{
    int m, d, y;
    if (sscanf(date, "%d/%d/%d", &m, &d, &y) != 3)
	return atol(date);	/* assume is in AKA yyyymmdd format */
    else {
	if (y < 1900)
	    y += 1900;
	return y * 10000 + m * 100 + d;
    }
}

/* -----------------------------------------------------------------
   Purpose: set the default tax rates
   Returns: TRUE/FALSE
   ----------------------------------------------------------------- */
int
set_taxrates(const char *taxrates)
{
    const char *ptr = taxrates;
    char *tptr;
    double taxrate_income = strtod(ptr, &tptr);
    double taxrate_short = taxrate_income;
    double taxrate_long = taxrate_income;
    double taxrate_superlong = taxrate_income;

    if (tptr != NULL && *tptr != '\0') {
	ptr = tptr;
	if (*ptr != ',') {
	    fprintf(stderr, "Error: %s is not a valid taxrate string\n",
		    taxrates);
	    return 0;
	}
	else {
	    ptr++;
	    taxrate_short = strtod(ptr, &tptr);
	    if (tptr != NULL && *tptr != '\0') {
		ptr = tptr;
		if (*ptr != ',') {
		    fprintf(stderr, "Error: %s is not a valid taxrate string\n",
			    taxrates);
		    return 0;
		}
		else {
		    ptr++;
		    taxrate_long = strtod(ptr, &tptr);
		    if (tptr != NULL && *tptr != '\0') {
			ptr = tptr;
			if (*ptr != ',') {
			    fprintf(stderr,
				    "Error: %s is not a valid taxrate string\n",
				    taxrates);
			    return 0;
			}
			else {
			    ptr++;
			    taxrate_superlong = strtod(ptr, &tptr);
			}
		    }
		}
	    }
	}
    }
    AKADefaultTaxRate(taxrate_income, taxrate_short,
		      taxrate_long, taxrate_superlong);
    return 1;
}

#ifdef _MSC_VER
#include <string.h>

/* -----------------------------------------------------------------
   Purpose: extremely simplified version of getopt for microsoft
   Returns: 
   ----------------------------------------------------------------- */
int
getopt (int argc, char *const *argv, const char *opts)
{
    int c = EOF;
    optind += 1;
    optarg = NULL;
    if (optind < argc && argv[optind][0] == '-') {
	const char *opt = NULL;
	c = argv[optind][1];
	opt = strchr(opts, c);
	if (opt) {
	    if (opt[1] == ':') {
		if (optind < argc -1) {
		    optind++;
		    optarg = argv[optind];
		}
		else
		    c = '?';
	    }
	}
	else
	    c = '?';
    }
    return c;
}

#endif
