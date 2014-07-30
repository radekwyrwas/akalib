#include <iostream>
using namespace std;

#include <stdlib.h>
#include "akaapi.hpp"
using namespace AndrewKalotayAssociates;

int
main(int argc, char *argv[])
{
    double price = (argc > 1) ? atof(argv[1]) : 100;

	// The Initialization object must be successfully created and authorized
    Initialization init;
    if (init.Authorize("./akalib.key") > 0) { // read key from a file
	cerr << "Error: " << init.ErrorString() << endl;
	return init.Error();
    }
	// Create both an interest rate model and a bond object
    Bond bond("simple", Date(2010, 1, 1), Date(2020, 1, 1), 3.5); // 10yr, 3.5%
    InterestRateModel model;
    model.SetRate(1, 2.0);	// set a flat 2% rate
    model.Solve();
    if (model.Error() > 0) {
	cerr << "Error: " << model.ErrorString() << endl;
	return model.Error();
    }
	// Now value the bond
    Value value(bond, model, Date(2013, 7, 1));
    if (value.Error() > 0) {
	cerr << "Error: " << value.ErrorString() << endl;
	return value.Error();
    }
    double oas = value.OAS(price); // oas at price

    cout << "bullet bond w/ flat curve oas: " << oas << endl;

	// more sophisticated inputs
    bond.SetCallAmerican(true);  // not necessary as this is the default
    bond.SetNoticePeriod(15); // 15 days
	// add a declining call schedule
    bond.SetCall(Date(2014,01,01), 102);
    bond.SetCall(Date(2015,01,01), 101.5);
    bond.SetCall(Date(2016,01,01), 101);
	// set a realistic rate curve
    double terms[] = {.25, .5, 1., 2, 3, 4, 5, 7, 10, 20, 30 };
    double rates[] = { 0.061, 0.111, 0.17, 0.43, 0.74, 1.115, 1.49,
		       2.03, 2.6, 3.31, 3.6 };
    for (unsigned int i = 0; i < sizeof(terms) / sizeof(terms[0]); i++)
	model.SetRate(terms[i], rates[i]);
    model.SetVolatility(7.5);
    model.Solve();
    if (model.Error() > 0) {
	cerr << "Error: " << model.ErrorString() << endl;
	return model.Error();
    }
    value.Reset(bond, model, Date(2013, 7, 1));
    if (value.Error() > 0) {
	cerr << "Error: " << value.ErrorString() << endl;
	return value.Error();
    }
    oas = value.OAS(price); // oas at price
    double optval = value.OptionValue(oas);

    cout << "option bond w/ realistic curve "
	 << "oas: " << oas << " optval: " << optval << endl;

    return 0;
}
