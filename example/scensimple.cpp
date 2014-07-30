/*
  Scenario Analysis Sample Code.

  This sample program is provided by Andrew Kalotay Associates to
  their clients.  Valid licensees of the BondOAS(tm) library may
  freely use or modify this code.
 */

#include <iostream>
using namespace std;

#include <stdlib.h>
#include "akaapi.hpp"
using namespace AndrewKalotayAssociates;

int
main(int argc, char *argv[])
{
    if (argc != 4) {
	cerr << "Usage: price pvdate horizondate" << endl;
	return 255;
    }

    double price = atof(argv[1]);
    Date pvdate(argv[2], Date::MDY);
    if (!pvdate.Good()) {
	cerr << "Error: invalid pvdate" << endl;
	return 255;
    }
    Date horizondate(argv[3], Date::MDY);
    if (!horizondate.Good()) {
	cerr << "Error: invalid horizon date" << endl;
	return 255;
    }

	// The Initialization object must be successfully created and authorized
    Initialization init;
    if (init.Authorize("./akalib.key") > 0) { // read key from a file
	cerr << "Error: " << init.ErrorString() << endl;
	return init.Error();
    }

	// some sample rates
    double terms[] = {.25, .5, 1., 2, 3, 4, 5, 7, 10, 20, 30 };
    double rates[] = { 0.061, 0.111, 0.17, 0.43, 0.74, 1.115, 1.49,
		       2.03, 2.6, 3.31, 3.6 };

	// create an initial interest rate model
    InterestRateModel initmodel;
    initmodel.SetVolatility(7.5);
    for (unsigned int i = 0; i < sizeof(terms) / sizeof(terms[0]); i++)
	initmodel.SetRate(terms[i], rates[i]);
    if (initmodel.Solve() > 0) {
	cerr << "Error: " << initmodel.ErrorString() << endl;
	return initmodel.Error();
    }

	// Create a bond object
    Bond bond("simple", Date(2010, 1, 1), Date(2020, 1, 1), 3.85); // 10yr, 3.85%
    bond.SetCallAmerican(true);  // not necessary as this is the default
    bond.SetNoticePeriod(15); // 15 days
	// add a declining call schedule
    bond.SetCall(Date(2014,01,01), 102);
    bond.SetCall(Date(2015,01,01), 101.5);
    bond.SetCall(Date(2016,01,01), 101);

	// Value the bond
    Value value(bond, initmodel, pvdate);
    if (value.Error() > 0) {
	cerr << "Error: " << value.ErrorString() << endl;
	return value.Error();
    }
	// get the OAS at that price
    double oas = value.OAS(price);
    if (value.Error() > 0) {
	cerr << "Error: " << value.ErrorString() << endl;
	return value.Error();
    }

	// create a scenario analysis object
	// first we need a horizon interest rate model
    InterestRateModel horizonmodel;
    horizonmodel.SetVolatility(7.5);
    for (unsigned int i = 0; i < sizeof(terms) / sizeof(terms[0]); i++)
	horizonmodel.SetRate(terms[i], rates[i] * .95); // down by 5%
    if (horizonmodel.Solve() > 0) {
	cerr << "Error: " << horizonmodel.ErrorString() << endl;
	return horizonmodel.Error();
    }
    
    InterestRateScenario scenario(pvdate.YearsTo(horizondate, Bond::DC_ACT_ACT),
				  horizonmodel);

    ScenarioAnalysis analysis;
    if (value.AnalyzeScenario(scenario, oas, analysis) == false) {
	cerr << "Error: scenario analysis failed " << value.ErrorString()
	     << endl;
	return value.Error();
    }

	// now print out the values
    cout.precision(6);
    cout << "Initial price/oas: " << price << " / " << oas << endl;
    cout << "Redemption: " << analysis.redeemstring();
    if (analysis.redeemed != ScenarioAnalysis::NOTREDEEMED) {
	Date rdate = value.FlowDate(analysis.flowwhen);
	cout << " on ";
	cout << rdate.MonthOf() << "/" << rdate.DayOf() << "/" << rdate.YearOf();
	cout << endl;
    }
    else {
	Value horizonvalue(bond, horizonmodel, horizondate);
	cout << endl;		// close redemption line
	cout << "Horizon price: " << horizonvalue.Price(oas) << endl;
    }
    return 0;
}
