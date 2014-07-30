using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using AndrewKalotayAssociates;

namespace AkaLibExample
{
    class AkaLibExample
    {
        public enum CurveType
        {
            FLAT,
            LINEAR,
            ASYM
        }

        private static bool quiet = false;
        private static double vol = 0;
        private static bool bullet = false;
        // set number of pvdates to value 
        private static int days = 100;
        private static bool fromoas = true; // go from oas to price
        private static double quote = 0;
        private static CurveType ctype = CurveType.FLAT;
        private static bool timing = true;

        private static double rate = 7.0; // discount rate 7.0% as 7.0
        private static double coupon = rate;

        static void Main(string[] args)
        {
            usage();
            if (args.Length < 3)
            {
                Console.WriteLine("Need minimum of 3 arguments");
                return;
            }
            Console.WriteLine("AkaLibExample.Main() starting...");
            int i = 0;
            // int key = 23028892;
            // string uname = "Demo expires 3/1/2020";

            // Get user, key and  from command line - used to init AKA library
            int key = Convert.ToInt32(args[i++]);
            string uname = args[i++];

            double rate = Convert.ToDouble(args[i++]); // discount rate 7.0% as 7.0
            double coupon = (i < args.Length) ? Convert.ToDouble(args[i++]) : rate;

            Console.WriteLine("rate: " + rate + " coupon: " + coupon);

            // Initialize the library
            Console.WriteLine("AkaLibExample.main() - creating Initialization with key: " + key + " uname: " + uname);
            Initialization akareg = new Initialization(key, uname);
            Console.WriteLine("AkaLibExample.main() - created Initialization - error: " + akareg.Error());
            if (akareg.Error() > 0)
            {
                Console.WriteLine("AkaLibExample.main() - Initialization failed with error: " + akareg.ErrorString());
                return;
            }

            Console.WriteLine("AkaLibExample.main() - Version: " + Initialization.Version() + " Expiration Libdate: " + akareg.Expiration().Libdate() + " Expiration YearOf: " + akareg.Expiration().YearOf() + " MonthOf: " + akareg.Expiration().MonthOf() + " DayOf: " + akareg.Expiration().DayOf());
            int error = exampleValuation();
            if (error > 0)
            {
                printError("AkaLibExample.main()", "exampleValuation() returned an error: " + error);
            }

            Console.WriteLine("AkaLibExample.Main() finished...");
        }

        // ***********************************************************
        // EXAMPLE METHODS
        // ***********************************************************

        /// <summary>
        /// Example of how to value a bond. </summary>

        public static int exampleValuation()
		{
            Date pvdate = new Date(2000, 1, 1);
            Date mdate = new Date(2030, 1, 1);

			Console.WriteLine("pvdate: " + pvdate.Libdate() + " mdate: " + mdate.Libdate());
			/* timing variables */
			long start = 0;


			InterestRateModel model = new InterestRateModel();
			if (!model.SetVolatility(vol))
			{
				Console.Write("Warning: invalid volatility '{0:F}', using 0\n", vol);
			}
			if (!model.SetRate(.5, rate))
			{
				Console.Write("Warning: invalid input rate '{0:F}', using 2%\n", rate);
				rate = 2;
				model.SetRate(.5, rate);
			}
			if (ctype == CurveType.LINEAR)
			{
				model.SetRate(1, rate + .01);
				model.SetRate(30, rate + .3);
			}
			else if (ctype == CurveType.ASYM)
			{
				double[] terms = new double[] {1, 3, 5, 7, 10, 15, 30};
				for (int i = 0; i < terms.Length; i++)
				{
					model.SetRate(terms[i], rate + 2 * (1 - 1.0 / terms[i]));
				}
			}
			if (quiet == false)
			{
				Console.Write("Making a par rate curve with {0:g2} volatility", vol);
				Console.Write(", {0:F1} yr = {1:F2}%", 1.0, model.GetRate(1));
				Console.Write(", {0:F1} yr = {1:F2}%", 30.0, model.GetRate(30));
			}

			if (timing)
			{
				start = CurrentUnixTimeMillis();
			}
			model.Solve();
			if (timing)
			{
				start = CurrentUnixTimeMillis() - start;
				Console.Write("Seconds to fit the base curve = {0:F2}\n", INSECS(start));
			}
			if (!msgs(model))
			{
				return model.Error();
			}

			// make the bond 
			if (quiet == false)
			{
				Console.Write("\nMaking a 30 year {0:g10}% bond maturing on {1:D}", coupon, mdate.Libdate());
			}
			Date idate = new Date(mdate.YearOf() - 30, mdate.MonthOf(), mdate.DayOf());
			Bond bond = new Bond("example", idate, mdate, coupon);
			if (!msgs(bond))
			{
				return bond.Error();
			}

			if (!bullet)
			{
				Date cdate = new Date(idate.YearOf() + 5, idate.MonthOf(), idate.DayOf());
				if (quiet == false)
				{
					Console.Write(" callable {0:D} at par", cdate.Libdate());
				}
				if (!bond.SetCall(cdate, 100))
				{
					Console.Write("failed to add call at {0:D}\n", cdate.Libdate());
				}
			}
			if (quiet == false)
			{
				Console.Write("\n\n");
			}

			if (quiet == false)
			{
				string underline = "--------------------";
				string fmt = "%10.10s %8.8s %8.8s %8.8s %8.8s %8.8s";
				Console.Write(fmt, "pvdate  ", fromoas ? "price" : "oas", "accrued", "optval", "duration", "convex.");
				Console.Write("\n");
				Console.Write(fmt, underline, underline, underline, underline, underline, underline);
				Console.Write("\n");
			}

			Value value = new Value(bond, model, pvdate);
			if (!msgs(value))
			{
				return value.Error();
			}

			// loop through pvdates 
			start = CurrentUnixTimeMillis();
			int cnt = 0;
			for (cnt = 0; pvdate.IsLT(mdate) && cnt < days; pvdate.PlusEqual(1), cnt++)
			{
				if (cnt > 0)
				{
					value.Reset(bond, pvdate);
					if (!msgs(value))
					{
						break;
					}
				}
				double oas = fromoas ? quote : value.Oas(quote);
				double price = fromoas ? value.Price(quote) : quote;
				if (!msgs(value) || oas == Value.BadValue || price == Value.BadValue)
				{
					break;
				}

				if (quiet == false)
				{
					Duration duration = value.EffectiveDuration(oas);
					Console.Write("{0:D2}/{1:D2}/{2:D4} {3,8:F3} {4,8:F3} {5,8:F3} {6,8:F3} {7,8:F3}", pvdate.MonthOf(), pvdate.DayOf(), pvdate.YearOf(), fromoas ? price : oas, value.Accrued(), value.OptionValue(oas), duration.duration, duration.convexity);
					Console.Write("\n");
				}
			}
			if (timing)
			{
				Console.Write("\nSeconds to value the bond for {0:D} pvdates = {1:F2}\n", cnt, INSECS(CurrentUnixTimeMillis() - start));
			}
			return 0;
		} // exampleValuation()


        /// <summary>
        /// Purpose: display usage message
        /// Returns: nothing
        /// </summary>

        public static void usage()
        {
            Console.Write("Purpose: value a 30 year bond\n");
            Console.Write("Usage: AkaLibExample <key> <username> <discount-rate> [<coupon> : defaults to discout-rate]\n");
        }

        // **********************************************
        // DEBUG/TRACING ROUTINES
        // **********************************************

        internal static double INSECS(long millis)
        {
            return (double)(millis / 1000.0);
        }

        /// <summary>
        /// Purpose: print warnings and errors, if any
        /// Returns: true if no errors, else false
        /// </summary>
        public static bool msgs(Status status)
        {
            for (int i = 0; i < status.WarningCount(); i++)
            {
                Console.Write("Warning: {0,2:D} \"{1}\"\n", status.Warning(i), status.WarningString(i));
            }
            if (status.Error() > 0)
            {
                Console.Write("{0,2:D} \"{1}\"\n", status.Error(), status.ErrorString());
            }

            return status.Error() == 0;
        }

        internal static void printError(string method, string error)
        {
            StringBuilder buff = new StringBuilder();
            buff.Append("\n");
            buff.Append("\n");
            buff.Append("************************************* ERROR *************************************");
            buff.Append("\n");
            buff.Append("Method: ");
            buff.Append(method);
            buff.Append("\n");
            buff.Append("Error: ");
            buff.Append(error);
            buff.Append("\n");
            buff.Append("************************************* ERROR *************************************");
            buff.Append("\n");
            buff.Append("\n");
            Console.WriteLine(buff.ToString());
        }

        private static readonly System.DateTime Jan1st1970 = new System.DateTime(1970, 1, 1, 0, 0, 0, System.DateTimeKind.Utc);

        internal static long CurrentUnixTimeMillis()
        {
            return (long)(System.DateTime.UtcNow - Jan1st1970).TotalMilliseconds;
        }
    }
}
