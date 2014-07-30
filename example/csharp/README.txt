Instructions For Compiling C# Example

The following directory structure is assumed:

akalib\win32\lib
akalib\win64\lib
akalib\csharp
akalib\example
akalib\example\csharp

Please refer to akalib\csharp\csharp.compile.README for how to compile and use
use the C# API.

To compile the example here:

1. Add the AkaLibExample.cs example file to your project.
2. Add the Andrew Kalotay Associates C# api to your project,
   a) add the source files in akalib\csharp
	- or -
   b) add the dll you compiled from those sources

To run the example, you must have the the architecture appropriate akalib
library directory in your path, i.e., the full path to akalib\win64\lib.

Run the resulting application:

AkaLibExample.exe <KEY> <USERNAME> <YIELDRATE> <COUPON>

Note, <KEY> and <USERNAME> can be gotten from the akalib.key file.
<YIELDRATE> and <COUPON> are user inputs.  Examples are 2.2 and 4.5,
respectively.
