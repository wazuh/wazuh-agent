/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.dbpage;

import com.sleepycat.db.DatabaseException;

import java.io.File;
import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.TextField;
import javafx.stage.FileChooser;
import db_gui.BDBInitializable;

/**
 * BDBDbInitializeable is the parent class of all the controllers of the
 * Database Page.  It implements operations that are part of all or most of
 * each database tab, such as a field to enter the database file or database
 * name.
 */
public class BDBDbInitializable extends BDBInitializable {
    @FXML
    protected TextField DatabaseFileTextField;
    @FXML
    protected TextField DatabaseNameTextField;
    protected String file = null;

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL location, ResourceBundle resources) {
        //no-op
    }

    /**
     * Clears the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        if (DatabaseFileTextField != null)
            DatabaseFileTextField.clear();
        if (DatabaseNameTextField != null)
            DatabaseNameTextField.clear();
        file = null;
    }

    /**
     * Implements the file chooser that lets the user select the database file.
     * When a file is selected, it is displayed in DatabaseFileTextField.
     *
     * @param event - Unused.
     */
    @FXML
    protected void selectDatabaseFile(ActionEvent event) {
        FileChooser dbFileChooser = new FileChooser();
        File dbFile = dbFileChooser.showOpenDialog(bdbState.getMainStage());
        if (dbFile != null) {
            file = dbFile.getAbsolutePath();
            DatabaseFileTextField.setText(file);
        }
    }

    /**
     * Reads the text in DatabaseFileTextField and checks that it exists.
     *
     * @return True on success, false on error.
     */
    protected boolean getDatabaseFile() {
        String db = DatabaseFileTextField.getText();
        if (db == null || db.isEmpty())
            return false;
        File dbFile = new File(db);
        if (!dbFile.isAbsolute()) {
            try {
                db = bdbState.getEnv().getHome().getAbsolutePath()
                        + File.separator + db;
            } catch (DatabaseException ex) {
                Logger.getLogger(
                        BDBDbInitializable.class.getName()).log(
                        Level.SEVERE, null, ex);
                bdbState.printFeedback(
                        "Error getting environment home: " + ex.getMessage());
                return false;
            }
            dbFile = new File(db);
        }
        if (!dbFile.exists() || dbFile.isDirectory()) {
            bdbState.printFeedback(
                    "Database file is not a file or does not exist: " + db);
            return false;
        }
        file = db;
        return true;
    }

    /**
     * Gets the text into in the DatabaseNameTextField, which is the name of
     * the sub-database or in-memory database.
     *
     * @return The entered name of the database.
     */
    protected String getDatabaseName() {
        if (DatabaseNameTextField == null
                || (DatabaseNameTextField.getText() == null)
                || (DatabaseNameTextField.getText().length() == 0))
            return null;
        return DatabaseNameTextField.getText();
    }

}
