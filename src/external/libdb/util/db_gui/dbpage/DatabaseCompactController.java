/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.dbpage;

import java.net.URL;
import java.util.ResourceBundle;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;

/**
 * DatabaseCompactController implements the Compact tab of the Database Access
 * page.
 */
public class DatabaseCompactController extends BDBDbInitializable {
    final static private String FXMLResource =
            "/db_gui/dbpage/FXMLDatabaseCompactTab.fxml";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        super.initialize(url, rb);
    }

    /**
     * Clears all the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        super.clearAllFields();
    }

    /**
     * Implements the Compact button.  Creates a new CompactTask which compacts
     * the given database file in a separate thread.
     *
     * @param event - Unused.
     */
    @FXML
    private void handleCompactDatabaseButton(ActionEvent event) {
        if (file == null && !getDatabaseFile()) {
            bdbState.printFeedback("Please enter a database file.");
            return;
        }
        CompactTask compact = new CompactTask(file);
        bdbState.disable();
        new Thread(compact).start();
    }

}
