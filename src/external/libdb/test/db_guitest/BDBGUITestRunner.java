/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_guitest;

//JUnit Suite Test

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;

import org.junit.runner.Result;
import org.junit.runner.notification.Failure;

/*@RunWith(Suite.class)

@Suite.SuiteClasses({ 
   DataAccessPageTests.class
})*/

/**
 * @author lfoutz
 */
public class BDBGUITestRunner {

    private static final Class[] ALLTESTS = {
            EnvironmentPageTests.class,
            DatabasePageTests.class,
            DataAccessPageTests.class
    };

    public static void usage() {
        System.out.println("usage: java db_guitest.BDBGUITestRunner"
                + "[test1 [test2 ..]]");
        System.exit(-1);
    }

    public static void main(String[] arg) {
        List<String> testNames = new LinkedList<>();
        List<Class> argTests = new LinkedList<>();
        Class[] tests = {};

        testNames.addAll(Arrays.asList(arg));

        //Get the tests to run
        if (!testNames.isEmpty()) {
            for (int i = 0; i < testNames.size(); i++) {
                Class testClass = null;
                try {
                    testClass = Class.forName("db_guitest." + testNames.get(i));
                } catch (ClassNotFoundException e) {
                    System.out.println(
                            "Skipping test " + testClass + ". Test not found.");
                }
                if (testClass != null) {
                    argTests.add(testClass);
                }
            }
            if (!argTests.isEmpty()) {
                tests = argTests.toArray(tests);
            } else {
                tests = ALLTESTS;
            }
        } else {
            tests = ALLTESTS;
        }

        // Run the tests
        int failureCount = 0;
        boolean success = true;
        File errorFile = new File("./errorLog.txt");
        try {
            if (errorFile.exists()) {
                errorFile.delete();
            }
            errorFile.createNewFile();
            System.setErr(new PrintStream(new FileOutputStream(errorFile)));
        } catch (IOException e) {
        }
        Result res = org.junit.runner.JUnitCore.runClasses(tests);
        if (!res.wasSuccessful()) {
            System.err.println(
                    "Number of failures are " + res.getFailureCount());
            List<Failure> testFailures = res.getFailures();
            ListIterator<Failure> iter = testFailures.listIterator();
            while (iter.hasNext()) {
                Failure fail = iter.next();
                System.err.println(fail.getDescription());
                Throwable e = fail.getException();
                if (e != null) {
                    e.printStackTrace();
                } else {
                    System.err.println(fail.getTrace());
                }
            }
            failureCount += res.getFailureCount();
            success = res.wasSuccessful();
        }
        if (success) {
            System.out.println("All tests successful.");
        } else {
            System.out.println(failureCount + " tests failed.");
        }
        System.out.println("Failures printed to " + errorFile.getName());
    }

}
