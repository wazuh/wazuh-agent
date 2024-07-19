/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.dbpage;

import db_gui.dbpage.DatabaseRemoveController;
import db_gui.dbpage.BDBDbInitializable;
import com.sleepycat.db.DatabaseException;

import java.io.FileNotFoundException;
import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.TextField;

/**
 * DatabaseRenameController implements the Rename tab of the Database Access
 * page.
 */
public class DatabaseRenameController extends BDBDbInitializable {
    @FXML
    private TextField DatabaseNewTextField;
    final static private String FXMLResource =
            "/db_gui/dbpage/FXMLDatabaseRenameTab.fxml";

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
        DatabaseNewTextField.clear();
        super.clearAllFields();
    }

    /**
     * Reads the new name for the database from DatabaseNewTextField.
     *
     * @return - The text entered into DatabaseNewTextField.
     */
    private String getNewDatabaseName() {
        if ((DatabaseNewTextField.getText() == null)
                || (DatabaseNewTextField.getText().length() == 0))
            return null;
        return DatabaseNewTextField.getText();
    }

    /**
     * Implements the Rename button.  Renames the given database or file.
     *
     * @param event - Unused.
     */
    @FXML
    private void handleRenameDatabaseButton(ActionEvent event) {
        String name, dbName, newName, completeNewName;
        if (file == null && !getDatabaseFile() && getDatabaseName() == null) {
            bdbState.printFeedback(
                    "Please enter a database file or database name.");
            return;
        }
        newName = getNewDatabaseName();
        dbName = getDatabaseName();
        if (newName == null) {
            bdbState.printFeedback(
                    "Please enter a new database name or file.");
            return;
        }

        if (file != null && dbName != null) {
            name = file + ":" + dbName;
            completeNewName = file + ":" + newName;
        } else if (getDatabaseName() != null) {
            name = "in-memory:" + dbName;
            completeNewName = "in-memory:" + newName;
        } else {
            name = file;
            completeNewName = newName;
        }
        try {
            bdbState.printFeedback(
                    "Renaming database: " + name + " to " + completeNewName);
            bdbState.getEnv().renameDatabase(
                    bdbState.beginTxn(), file, dbName, newName);
            bdbState.commitTransaction();
            bdbState.printFeedback(
                    name + " successfully renamed to " + newName);
        } catch (DatabaseException | FileNotFoundException ex) {
            Logger.getLogger(
                    DatabaseRemoveController.class.getName()).log(
                    Level.SEVERE, null, ex);
            bdbState.printFeedback(
                    "Error renaming database: " + ex.getMessage());
            bdbState.abortTransaction();
        }
    }
}
