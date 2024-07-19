/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.datapage;

import db_gui.datapage.BDBDataInitializable;
import com.sleepycat.db.DatabaseEntry;

import java.net.URL;
import java.util.ResourceBundle;

import javafx.fxml.FXML;
import javafx.scene.control.TextField;

/**
 * Implements the recno Put and Browse data panels of the Data Access Page.
 */
public class RecnoController extends BDBDataInitializable {
    @FXML
    private TextField PutKeyTextField;
    @FXML
    private TextField GetKeyTextField;
    final static private String FXMLResource =
            "/db_gui/datapage/FXMLRecno.fxml";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        super.initialize(url, rb);
    }

    /**
     * Clears the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        PutKeyTextField.clear();
        GetKeyTextField.clear();
        super.clearAllFields();
    }

    /**
     * Converts the data entered into a TextField into a key DatabaseEntry for a
     * recno database, which is always an integer.
     *
     * @param textField - The TextField to be read.
     * @return - The DatabaseEntry containing the recno key created from the
     * TextField.
     */
    private DatabaseEntry getDatabaseEntry(TextField textField) {
        String text = textField.getText();
        if (text == null || text.length() == 0) {
            bdbState.printFeedback("Please enter a value for the key.");
            return null;
        }
        DatabaseEntry key = new DatabaseEntry();
        int recno;
        try {
            recno = Integer.parseInt(text);
        } catch (NumberFormatException ex) {
            bdbState.printFeedback(
                    "Invalid value entered for the key: " + text);
            return null;
        }
        key.setRecordNumber(recno);
        return key;
    }

    /**
     * Displays a DatabaseEntry for a key of a recno database in a TextField.
     * The keys for recno databases are always integers.
     *
     * @param key - The DatabaseEntry that contains the key read from the recno
     * database.
     * @param textField - The TextField in which to display the recno key.
     */
    private void setTextField(DatabaseEntry key, TextField textField) {
        String keyString;
        long recno;
        if (key == null) {
            textField.clear();
            return;
        }
        recno = key.getRecordNumber();
        keyString = "" + recno;
        textField.setText(keyString);
    }

    /**
     * This function converts the text entered into the
     * PutKeyTextField into a DatabaseEntry.
     *
     * @return The Database entry containing the recno key entered into the
     * PutKeyTextField.
     */
    @Override
    public DatabaseEntry getPutKeyDatabaseEntry() {
        return getDatabaseEntry(PutKeyTextField);
    }

    /**
     * Displays the data in the given DatabaseEntry in the PutKeyTextField.
     *
     * @param key - The DatabaseEntry containing the recno key to be displayed
     * in PutKeyTextField.
     */
    @Override
    public void setPutKeyDatabaseEntry(DatabaseEntry key) {
        setTextField(key, PutKeyTextField);
    }

    /**
     * This function converts the text entered into the
     * GetKeyTextField into a DatabaseEntry.
     *
     * @return The Database entry containing the recno key entered into the
     * GetKeyTextField.
     */
    @Override
    public DatabaseEntry getGetKeyDatabaseEntry() {
        return getDatabaseEntry(GetKeyTextField);
    }

    /**
     * Displays the data in the given DatabaseEntry in the GetKeyTextField.
     *
     * @param key - The DatabaseEntry containing the recno key to be displayed
     * in GetKeyTextField.
     */
    @Override
    public void setGetKeyDatabaseEntry(DatabaseEntry key) {
        setTextField(key, GetKeyTextField);
    }

    /**
     * Checks if a value has been entered into GetKeyTextField.
     *
     * @return - Whether the GetKeyTextField has text in it or not.
     */
    @Override
    public boolean getGetKeySet() {
        String keyString = GetKeyTextField.getText();
        return !(keyString == null || keyString.length() == 0);
    }

    /**
     * Returns whether to append or put a new record into the database.
     *
     * @return False, since recno records are put, not appended, into the
     * database.
     */
    @Override
    public boolean getAppendRecords() {
        return false;
    }
}
