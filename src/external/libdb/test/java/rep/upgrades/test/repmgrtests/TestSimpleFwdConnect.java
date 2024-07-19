/*-
 * Copyright (c) 2010, 2020 Oracle and/or its affiliates.  All rights reserved.
 * 
 * See the file LICENSE for license information.
 *
 */

package repmgrtests;

import org.junit.Test;

public class TestSimpleFwdConnect extends SimpleConnectTest {
    @Test public void forwardMixed() throws Exception {
        doTest(false);
    }
}    
    
