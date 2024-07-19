/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.dbpage;

import db_gui.dbpage.BDBDbInitializable;
import com.sleepycat.db.DatabaseException;

import java.io.FileNotFoundException;
import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;

/**
 * DatabaseRemoveController implements the Remove tab of the Database Access
 * page.
 */
public class DatabaseRemoveController extends BDBDbInitializable {
    final static private String FXMLResource =
            "/db_gui/dbpage/FXMLDatabaseRemoveTab.fxml";

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
     * Implements the Remove button.  Deletes the given sub-database or
     * database file.
     *
     * @param event
     */
    @FXML
    private void handleRemoveDatabaseButton(ActionEvent event) {
        String name = "";
        String dbName = getDatabaseName();
        if (file == null && !getDatabaseFile() && dbName == null) {
            bdbState.printFeedback(
                    "Please enter a database file or database name.");
            return;
        }
        if (file != null && dbName != null)
            name = file + ":" + dbName;
        else if (file != null)
            name = file;
        else
            name = dbName;
        try {
            bdbState.printFeedback("Removing database: " + name);
            bdbState.getEnv().removeDatabase(bdbState.beginTxn(), file, dbName);
            bdbState.commitTransaction();
            bdbState.printFeedback(name + " successfully removed.");
        } catch (DatabaseException | FileNotFoundException ex) {
            Logger.getLogger(
                    DatabaseRemoveController.class.getName()).log(
                    Level.SEVERE, null, ex);
            bdbState.printFeedback(
                    "Error removing database: " + ex.getMessage());
            bdbState.abortTransaction();
        }
    }
}
