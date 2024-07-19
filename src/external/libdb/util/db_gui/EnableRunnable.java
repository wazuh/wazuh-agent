/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui;

/**
 * Used by background Tasks to re-enable tabs.  The tabs have to be re-enabled
 * in the JavaFX thread, which is why a Runnable class has to be used to do it.
 */
public class EnableRunnable implements Runnable {

    @Override
    public void run() {
        BDBState state = BDBState.getBDBState();
        state.enable();
    }

}
