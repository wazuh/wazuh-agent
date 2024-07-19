/*-
 * Copyright (c) 2010, 2020 Oracle and/or its affiliates.  All rights reserved.
 * 
 * See the file LICENSE for license information.
 *
 */

package repmgrtests;

import org.junit.Test;

public class TestSimpleRevConnect extends SimpleConnectTest {
    @Test public void reverseMixed() throws Exception {
        doTest(true);
    }
}    
    
