DB_GUITEST

This is the automatic test suite for testing db_gui.  It requires the following libraries:
junit.jar
testfx-core-4.0.jar
testfx-junit-4.0.jar
guava-20.0.jar

BUILDING ON UNIX/LINUX SYSTEMS

To build the test on postix systems build BDB with the following configurations:
../dist/configure --enable-java --enable-gui

Then add the test libraries mentioned above to the CLASSPATH environment variable along with db.jar and db_gui.jar, then build db_guitest with the following command:

make db_guitest

This will build db_guitest.jar. 

BUILDING ON WINDOWS 

Open up the Visual Studio solution that matches your prefered VS version.  Right click the db_java project and select "Build".  Note you may have to add the paths to the JDK include, include/win32, bin, and lib directories to the db_java project Directories property in order for db_java to build successfull.  After a successful build the jar files db.jar and db_gui.jar will be created.

Next add the test libraries mentioned above to the CLASSPATH environment variable along with db.jar and db_gui.jar, then build db_guitest with the following commands:
cd build_windows
mkdir classes.guitest
javac -g -d .\classes.guitest -classpath ".\classes.guitest;%CLASSPATH%" ..\test\db_guitest\*.java 
jar cf "db_guitest.jar" -C "classes.guitest" .

RUNNING DB_GUITEST

To run the tests add db_guitest.jar to the CLASSPATH and execute the following command:

java db_guitest.BDBGUITestRunner

Note the test will take control of your keyboard and mouse, and using your keyboard or mouse will interfer with the test.  It takes about 7 minutes to run.  Any errors will be printed out to ./errorLog.txt.
