Java Example Sources

The .java files in this directory are an example of using the AKA Library with
the java interface.  The java interface is composed of the akalib.jar file
which is in the java directory, and the akaapi java wrapper library which is
in the architecture appropriate lib directory.

To compile and run this example:

1. Compile the these java files.  For example:
	$ javac -cp  ../java/akalib.jar -d . *.java

2. Set the appropriate LD_LIBRARY_PATH (*nix) or PATH (Windows).  For example:
	$ LD_LIBRARY_PATH=../linux/lib

3. Run the java app:
	$ java -cp ./java/akalib.jar:. com.kalotay.akalib.example.AkaLibExample <KEY> <USERNAME>

	Note, under Windows the classpath separator is ;, not :.
	The key and username can be gotten from the akalib.key file.
