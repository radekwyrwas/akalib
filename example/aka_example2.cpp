/* -------------------------------------------------------------------------
 * Copyright (c) 2013, Andrew Kalotay Associates.  All rights reserved. *
   This example code is provided to users of the AKA Library.

   see usage() below
   ------------------------------------------------------------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#ifdef _MSC_VER  /* include implementation of getopts -- see end of file */
const char *optarg = NULL;
int optind = 0;
int getopt(int, char *const *, const char *);
#else
#include <unistd.h>
#endif

#include "akaapi.hpp"
#include "akaerrno.h"

using namespace AndrewKalotayAssociates;

/* forward declarations */
void usage();
bool msgs(Status *);

#define INSECS(x) ((double) (x) / CLOCKS_PER_SEC)

/* -----------------------------------------------------------------
   Purpose: start here
   Returns: 
   ----------------------------------------------------------------- */
int
main(int argc, char *argv[])
{
	/* user options */
    int c;
    int cnt;
    double rate, coupon;
    Date pvdate(2000, 01, 01);
    bool quiet = false;
    enum { FLAT, LINEAR, ASYM } ctype = FLAT;
    Date mdate(2030, 01, 01);
    double vol = 0;
    bool bullet = false;
    int days = 1;
    bool fromoas = true;	// go from oas to price
    double quote = 0;
    const char *keyfile = "./akalib.key";
    
	/* timing variables */
    clock_t start = 0;
    bool timing = false;

    while((c = getopt(argc, argv, "a:bc:d:fm:p:qT:tv:z"))!= EOF) {
	switch(c) {
	    case 'a' :
		keyfile = optarg;
		break;
	    case 'b' :
		bullet = true;
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
		    // ignoring for campatibility with C example
		break;
	    case 'm' :
		mdate = Date(optarg, Date::MDY);
		break;
	    case 'p' :
		pvdate = Date(optarg, Date::MDY);
		break;
	    case 'q' :		/* get the OAS from a quoted price*/
		fromoas = false;
		quote = 100;	/* use price of par */
		break;
	    case 't' :
		timing = true;
		quiet = true;
		break;
	    case 'v' :
		vol = atof(optarg);
		break;
	    case 'z':
		quiet = true;
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

    Initialization akareg(keyfile);
    if (akareg.Error() > 0) {
	fprintf(stderr, "ERROR: %s\n", akareg.ErrorString());
	return akareg.Error();
    }

    InterestRateModel model;
    if (!model.SetVolatility(vol))
	printf("Warning: invalid volatility '%f', using 0\n", vol);
    if (!model.SetRate(.5, rate)) {
	printf("Warning: invalid input rate '%f', using 2%%\n", rate);
	rate = 2;
	model.SetRate(.5, rate);
    }
    if (ctype == LINEAR) {
	model.SetRate(1, rate + .01);
	model.SetRate(30, rate + .3);
    }
    else if (ctype == ASYM) {
	double terms[] = { 1, 3, 5, 7, 10, 15, 30 };
	for (unsigned int i = 0; i < sizeof(terms) / sizeof(terms[0]); i++)
	    model.SetRate(terms[i], rate + 2 * (1 - 1.0 /terms[i]));
    }
    if (quiet == false) {
	printf("Making a par rate curve with %.2g volatility", vol);
	printf(", %0.0f yr = %.2f%%", 1.0, model.GetRate(1));
	printf(", %0.0f yr = %.2f%%", 30.0, model.GetRate(30));
    }

    if (timing)
	start = clock();
    model.Solve();
    if (timing) {
	start = clock() - start;
	printf("Seconds to fit the base curve = %0.2f\n", INSECS(start));
    }
    if (!msgs(&model))
	return model.Error();

	/* make the bond */
    if (quiet == false)
	printf("\nMaking a 30 year %.10g%% bond maturing on %ld",
	       coupon, mdate.Libdate());
    Date idate(mdate.YearOf() - 30, mdate.MonthOf(), mdate.DayOf());
    Bond bond("example", idate, mdate, coupon);
    if (!msgs(&bond))
	return bond.Error();

    if (!bullet) {
	Date cdate(idate.YearOf() + 5, idate.MonthOf(), idate.DayOf());
	if (quiet == false)
	    printf(" callable %ld at par", cdate.Libdate());
	if (!bond.SetCall(cdate, 100))
	    printf("failed to add call at %ld\n", cdate.Libdate());
    }
    if (quiet == false)
	printf("\n\n");

    if (quiet == false) {
	const char underline[] = "--------------------";
	const char *fmt = "%10.10s %8.8s %8.8s %8.8s %8.8s %8.8s";
	printf(fmt, "pvdate  ", fromoas ? "price" : "oas",
	       "accrued", "optval", "duration", "convex.");
	printf("\n");
	printf(fmt, underline, underline, underline,
	       underline, underline, underline);
	printf("\n");
    }

    Value value(bond, model, pvdate);
    if (!msgs(&value))
	return value.Error();

	/* loop through pvdates */
    start = clock();
    for (cnt = 0; pvdate < mdate && cnt < days; pvdate += 1, cnt++) {
	if (cnt > 0) {
	    value.Reset(bond, pvdate);
	    if (!msgs(&value))
		break;
	}
	double oas = fromoas ? quote : value.OAS(quote);
	double price = fromoas ? value.Price(quote) : quote;
	if (!msgs(&value) || oas == Value::BadValue || price == Value::BadValue)
		break;

	if (quiet == false) {
	    Duration duration = value.EffectiveDuration(oas);
	    printf("%02d/%02d/%04d %8.3f %8.3f %8.3f %8.3f %8.3f",
		   pvdate.MonthOf(), pvdate.DayOf(), pvdate.YearOf(),
		   fromoas ? price : oas,
		   value.Accrued(), value.OptionValue(oas),
		   duration.duration, duration.convexity);
	    printf("\n");
	}
    }
    if (timing)
	printf("\nSeconds to value the bond for %d pvdates = %0.2f\n",
	       cnt, INSECS(clock() - start));
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
	"\t-m <mdate> -- set bond maturity date, default 1/1/2030\n"
	"\t-p <pvdate> -- set initial pvdate to value, default bond dated date\n");
    printf(
	"\t-q -- use price of 100 as quote, default use oas of zero\n"
	"\t-T <arg> -- set tax rates as: income[,short[,long[,superlong]]]\n"
	"\t-t -- display timings\n"
	"\t-v <vol> -- set curve volatility, default zero\n"
	"\t-z -- silent mode, no output, for timing\n");
    printf("%s\n", Initialization::VersionString());
}


/* -----------------------------------------------------------------
   Purpose: print warnings and errors, if any
   Returns: true if no errors, else false
   ----------------------------------------------------------------- */
bool
msgs(Status *status)
{
    for (int i = 0; i < status->WarningCount(); i++)
	fprintf(stdout, "Warning: %2d \"%s\"\n",
		status->Warning(i), status->WarningString(i));
    if (status->Error() > 0)
    	fprintf(stdout, "%2d \"%s\"\n", status->Error(),
		status->ErrorString());

    return status->Error() == 0;
}

/* -----------------------------------------------------------------
   Purpose: try and get the key from a file
   Returns: success true/false
   ----------------------------------------------------------------- */
bool
readkey(const char *fname, long *key, char *uname, int namesize)
{
    FILE *fp;
    char linestr[200];

    bool ret = false;
    *key = 0;
    *uname = '\0';

    fp = fopen(fname, "r");
    if (fp == NULL)
	fprintf(stderr, "Error: unable to open akakey file %s\n", fname);
    else if (fgets(linestr, sizeof(linestr), fp) == NULL ||
	     (*key = atol(linestr)) == 0)
	fprintf(stderr, "Error: missing key line from akakey file %s\n",
		fname);
    else if (fgets(linestr, sizeof(linestr), fp) == NULL)
	fprintf(stderr, "Error: missing user name line from akakey file %s\n",
		fname);
    else {
	int i;
	for (i = 0; i < namesize; i++)
	    if (linestr[i] == '\0' || linestr[i] == '\n' || linestr[i] == '\r')
		break;
	    else
		uname[i] = linestr[i];
	uname[i] = '\0';
	ret = i > 0;
	if (!ret)
	fprintf(stderr, "Error: empty user name line from akakey file %s\n",
		fname);
    }
    if (fp != NULL) fclose(fp);
    return ret;
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
