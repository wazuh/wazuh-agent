/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_guitest;

import java.util.concurrent.Callable;

import javafx.scene.control.ComboBox;

/**
 * ComboBoxTask is used to select a value in a combo box, since TestFX has
 * issues with that, and it has to be done in the FX thread or else it throws
 * an exception.
 */
public class ComboBoxTask implements Callable<Void> {
    private final ComboBox comboBox;
    private final String item;

    public ComboBoxTask(ComboBox comboBox, String item) {
        this.comboBox = comboBox;
        this.item = item;
    }


    @Override
    public Void call() throws Exception {
        comboBox.getSelectionModel().select(item);
        return null;
    }

}
