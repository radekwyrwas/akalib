/**
* @(#)AkaLibExample.java
*
* Copyright (c) Andrew Kalotay Associates.
* All rights reserved.
*/

package com.kalotay.akalib.example;

import java.io.*;
import org.netbeans.lib.cvsclient.commandLine.GetOpt;
import com.kalotay.akalib.*;

public class AkaLibExample {

    public enum CurveType { FLAT, LINEAR, ASYM };

    // ***********************************
    // COMMAND LINE OPTION VARIABLES
    // ***********************************
    private static String pvdateStr = null;
    private static String mdateStr = null;
    private static boolean quiet = false;
    private static double vol = 0;
    private static boolean bullet = false;
    // set number of pvdates to value 
    private static int days = 100;
    private static boolean fromoas = true;	// go from oas to price
    private static double quote = 0;
    private static CurveType ctype = CurveType.FLAT;
    private static boolean timing = true;

    private static double rate = 7.0; // discount rate 7.0% as 7.0
    private static double coupon = rate;

    /**
	 * Runs some example valuations.
	 *
	 * @param args Command line params are <KEY> <USER>
	 */
    
    public static void main(String args[]) {

        usage();

            // Parse any command line options flags
	GetOpt g = new GetOpt(args, "bc:d:fm:p:qT:tv:z");

	int c;
	String arg;
	while ((c = g.getopt()) != g.optEOF) {
	    switch(c) {
                case 'b' :
                    bullet = true;
                    break;
                case 'c' :
                    arg = g.optArgGet();
                    if ("f".equals(arg))
                        ctype = CurveType.FLAT;
                    else if ("a".equals(arg))
                        ctype = CurveType.ASYM;
                    else if ("l".equals(arg))
                        ctype = CurveType.LINEAR;
                    System.out.println("ctype: " + ctype);
                    break;
                case 'd' :
                    days = Integer.parseInt(g.optArgGet());
                    System.out.println("days: " + days);
                    break;
                case 'f' :
			// ignoring for compatibility with C example
                    break;
                case 'm' :
                    mdateStr = g.optArgGet();
                    break;
                case 'p' :
                    pvdateStr = g.optArgGet();
                    break;
                case 'q' :		/* get the OAS from a quoted price*/
                    fromoas = false;
                    quote = 100;	/* use price of par */
                    break;
                case 't' :
                    timing = true;
                    System.out.println("timing: " + timing);
                    quiet = true;
                    break;
                case 'v' :
                    vol = Double.parseDouble(g.optArgGet());
                    System.out.println("vol: " + vol);
                    break;
                case 'z':
                    quiet = true;
                    break;
                default:
			// usage();
                    return;
	    }
	}

	int i = g.optIndexGet();
            // Get user, key and  from command line - used to init AKA library
	int key = Integer.parseInt(args[i++]);
	String uname = args[i++];

	double rate = Double.parseDouble(args[i++]); // discount rate 7.0% as 7.0
	double coupon = (i < args.length) ? Double.parseDouble(args[i++]) : rate;

	System.out.println("rate: " + rate + " coupon: " + coupon);

            // *********************************************
            // First load the native C++ libraries from system path
            // *********************************************
	    System.loadLibrary("bondoas_java_wrap");


            // *********************************************
            // Next initialize the native C++ libraries
            // *********************************************

            // Initialize the library
            System.out.println("AkaLibExample.main() - creating Initialization with key: " + key + " uname: " + uname);            
            Initialization akareg = new Initialization(key, uname);
            System.out.println("AkaLibExample.main() - created Initialization - error: " + akareg.Error());            
            if (akareg.Error() > 0) {
                printError("AkaLibExample.main()", akareg.ErrorString());
                return;
            }
           
	    System.out.println("AkaLibExample.main() - Version: " +
			       akareg.Version() 
                + " Expiration Libdate: " + akareg.Expiration().Libdate()
                + " Expiration YearOf: " + akareg.Expiration().YearOf()
                + " Expiration DayOf: " + akareg.Expiration().DayOf()
                );

            // *********************************************
            // Next perform an example valuation
            // *********************************************

            int error = exampleValuation();
            if (error > 0) 
                printError("AkaLibExample.main()", "exampleValuation() returned an error: " + error);

    }
	
	// ***********************************************************
	// EXAMPLE METHODS
	// ***********************************************************

    /** Example of how to value a bond.	*/
    
    public static int exampleValuation() {
            // use command line dates is passed, otherwise use defaults
	Date pvdate = pvdateStr == null ? new Date(2000, 01, 01) : new Date(pvdateStr, Date.ENTRY.MDY);
	Date mdate = mdateStr == null ? new Date(2030, 01, 01) : new Date(mdateStr, Date.ENTRY.MDY);

	System.out.println("pvdateStr: " + pvdateStr + " pvdate: " + pvdate.Libdate() + " mdate: " + mdate.Libdate());
            /* timing variables */
	long start = 0;

          
	InterestRateModel model = new InterestRateModel();
	if (!model.SetVolatility(vol))
	    System.out.format("Warning: invalid volatility '%f', using 0\n", vol);
	if (!model.SetRate(.5, rate)) {
	    System.out.format("Warning: invalid input rate '%f', using 2%%\n", rate);
	    rate = 2;
	    model.SetRate(.5, rate);
	}
	if (ctype == CurveType.LINEAR) {
	    model.SetRate(1, rate + .01);
	    model.SetRate(30, rate + .3);
	}
	else if (ctype == CurveType.ASYM) {
	    double [] terms = { 1, 3, 5, 7, 10, 15, 30 };
	    for (int i = 0; i < terms.length; i++)
		model.SetRate(terms[i], rate + 2 * (1 - 1.0 /terms[i]));
	}
	if (quiet == false) {
	    System.out.format("Making a par rate curve with %.2g volatility", vol);
	    System.out.format(", %.1f yr = %.2f%%", 1.0, model.GetRate(1));
	    System.out.format(", %.1f yr = %.2f%%", 30.0, model.GetRate(30));
	}
            
	if (timing)
	    start = System.currentTimeMillis();
	model.Solve();
	if (timing) {
	    start = System.currentTimeMillis() - start;
	    System.out.format("Seconds to fit the base curve = %.2f\n", INSECS(start));
	}
	if (!msgs(model))
	    return model.Error();

            // make the bond 
	if (quiet == false)
	    System.out.format("\nMaking a 30 year %.10g%% bond maturing on %d",
			      coupon, mdate.Libdate());
	Date idate = new Date(mdate.YearOf() - 30, mdate.MonthOf(), mdate.DayOf());
	Bond bond = new Bond("example", idate, mdate, coupon);
	if (!msgs(bond))
	    return bond.Error();
            
	if (!bullet) {
	    Date cdate = new Date(idate.YearOf() + 5, idate.MonthOf(), idate.DayOf());
	    if (quiet == false)
		System.out.format(" callable %d at par", cdate.Libdate());
	    if (!bond.SetCall(cdate, 100))
		System.out.format("failed to add call at %d\n", cdate.Libdate());
	}
	if (quiet == false)
	    System.out.format("\n\n");
            
	if (quiet == false) {
	    String underline = "--------------------";
	    String fmt = "%10.10s %8.8s %8.8s %8.8s %8.8s %8.8s";
	    System.out.format(fmt, "pvdate  ", fromoas ? "price" : "oas", "accrued", "optval", "duration", "convex.");
	    System.out.print("\n");;
	    System.out.format(fmt, underline, underline, underline, underline, underline, underline);
	    System.out.print("\n");;
	}
            
	Value value = new Value(bond, model, pvdate);
	if (!msgs(value))
	    return value.Error();
            
            // loop through pvdates 
	start = System.currentTimeMillis();
	int cnt = 0;
	for (cnt = 0; pvdate.IsLT(mdate) && cnt < days; pvdate.PlusEqual(1), cnt++) {
	    if (cnt > 0) {
		value.Reset(bond, pvdate);
		if (!msgs(value))
		    break;
	    }
	    double oas = fromoas ? quote : value.OAS(quote);
	    double price = fromoas ? value.Price(quote) : quote;
	    if (!msgs(value) || oas == Value.getBadValue() || price == Value.getBadValue())
		break;
                
	    if (quiet == false) {
		Duration duration = value.EffectiveDuration(oas);
		System.out.format("%02d/%02d/%04d %8.3f %8.3f %8.3f %8.3f %8.3f",
				  pvdate.MonthOf(), pvdate.DayOf(), pvdate.YearOf(),
				  fromoas ? price : oas,
				  value.Accrued(), value.OptionValue(oas),
				  duration.getDuration(), duration.getConvexity());
		System.out.print("\n");;
	    }
	}
	if (timing)
	    System.out.format("\nSeconds to value the bond for %d pvdates = %.2f\n",
			      cnt, INSECS(System.currentTimeMillis() - start));
        return 0;
    } // exampleValuation()


    /**
    Purpose: display usage message
    Returns: nothing
    */
    
    public static void usage() {
        System.out.print("Purpose: value a 30 year bond\n");
        System.out.print("Usage: AkaLibExample [FLAGS] <key> <username> <discount-rate> [<coupon> : defaults to discout-rate]\n");
        System.out.print(
            "\nFlags:\n"
            + "\t-b -- bond is bullet bond\n");
        System.out.print(
            "\t-c <curve-type> -- make the curve be:\n"
            + "\t\tflat -- same rate (default)\n"
            + "\t\tlinear -- grow linearly (slowly)\n"
            + "\t\tasymptotic -- grow (quickly) but level off\n");
        System.out.print(
            "\t-d <cnt> -- set number of pvdates to value (default 1)\n"
            + "\t-m <mdate> -- set bond maturity date, default 1/1/2030\n"
            + "\t-p <pvdate> -- set initial pvdate to value, default bond dated date\n");
        System.out.print(
            "\t-q -- use price of 100 as quote, default use oas of zero\n"
            + "\t-T <arg> -- set tax rates as: income[,short[,long[,superlong]]]\n"
            + "\t-t -- display timings\n"
            + "\t-v <vol> -- set curve volatility, default zero\n"
            + "\t-z -- silent mode, no output, for timing\n");
    }

    // **********************************************
    // DEBUG/TRACING ROUTINES
    // **********************************************

    static double INSECS(long millis) { return (double)(millis / 1000.0); };

    /** 
    Purpose: print warnings and errors, if any
    Returns: true if no errors, else false
    */
    public static boolean msgs(Status status) {
        for (int i = 0; i < status.WarningCount(); i++)
            System.out.format("Warning: %2d \"%s\"\n", status.Warning(i), status.WarningString(i));
        if (status.Error() > 0)
            System.out.format("%2d \"%s\"\n", status.Error(),
            status.ErrorString());

        return status.Error() == 0;
    }

    static void printError(String method, String error) {
        StringBuffer buff = new StringBuffer();
        buff.append("\n");
        buff.append("\n");
        buff.append("************************************* ERROR *************************************");
        buff.append("\n");
        buff.append("Method: ");
        buff.append(method);
        buff.append("\n");
        buff.append("Error: ");
        buff.append(error);
        buff.append("\n");
        buff.append("************************************* ERROR *************************************");
        buff.append("\n");
        buff.append("\n");
        System.out.println(buff.toString());
    }
}
