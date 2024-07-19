/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.datapage;

import com.sleepycat.db.DatabaseEntry;

import java.net.URL;
import java.util.ResourceBundle;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.ComboBox;
import javafx.scene.control.TextField;

/**
 * Implements the btree and hash Put and Browse data panels of the Data Access
 * Page.
 */
public class BtreeHashController extends BDBDataInitializable {
    @FXML
    private TextField PutKeyTextField;
    @FXML
    private ComboBox<String> PutKeyComboBox;
    @FXML
    private TextField GetKeyTextField;
    @FXML
    private ComboBox<String> GetKeyComboBox;
    private DatabaseEntry currentGetKey;
    final static private String FXMLResource =
            "/db_gui/datapage/FXMLBtreeHash.fxml";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        GetKeyComboBox.getItems().addAll(dataFormats);
        GetKeyComboBox.getSelectionModel().selectFirst();
        PutKeyComboBox.getItems().addAll(dataFormats);
        PutKeyComboBox.getSelectionModel().selectFirst();
        super.initialize(url, rb);
    }

    /**
     * Clears the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        PutKeyTextField.clear();
        GetKeyComboBox.getSelectionModel().selectFirst();
        GetKeyTextField.clear();
        PutKeyComboBox.getSelectionModel().selectFirst();
        currentGetKey = null;
        super.clearAllFields();
    }

    /**
     * Change how the binary data is displayed in the GetKeyTextField when a
     * new type is selected in the GetKeyComboBox.
     *
     * @param event - Unused.
     */
    @FXML
    protected void handleGetKeyComboBox(ActionEvent event) {
        changeDataDisplay(currentGetKey, GetKeyTextField, GetKeyComboBox);
    }

    /**
     * Converts the data entered into a TextField into a key DatabaseEntry for a
     * btree or hash database.  The type of the data is stored in the ComboBox.
     *
     * @param textField - The TextField containing the data.
     * @param comboBox - The ComboBox which contains how the data should be
     * interpreted.
     * @return - The DatabaseEntry containing the data.
     */
    private DatabaseEntry getDatabaseEntry(
            TextField textField, ComboBox<String> comboBox) {
        String keyString;
        keyString = textField.getText();
        if (keyString == null || keyString.length() == 0) {
            bdbState.printFeedback("Error, please enter a Key value.");
            return null;
        }
        return getDatabaseEntry(keyString,
                comboBox.getSelectionModel().getSelectedItem());
    }

    /**
     * Displays a DatabaseEntry for a key of a btree or hash database in a
     * TextField.  The type of the key is set in the ComboBox.
     *
     * @param key - The DatabaseEntry containing the key to be displayed.
     * @param textField - The TextField in which to display the key.
     * @param comboBox - The ComboBox containing how the key is to be
     * interpreted.
     */
    private void setTextField(DatabaseEntry key,
            TextField textField, ComboBox<String> comboBox) {
        String keyString;
        if (key == null) {
            textField.clear();
            return;
        }
        keyString = getDatabaseEntryString(key,
                comboBox.getSelectionModel().getSelectedItem());
        textField.setText(keyString);
    }

    /**
     * This function converts the text entered into the PutKeyTextField
     * into a DatabaseEntry using the type selected in the PutKeyComboBox.
     *
     * @return - The DatabaseEntry for PutKeyTextField.
     */
    @Override
    public DatabaseEntry getPutKeyDatabaseEntry() {
        return getDatabaseEntry(PutKeyTextField, PutKeyComboBox);
    }

    /**
     * Displays the data in the given DatabaseEntry in the PutKeyTextField,
     * using the type selected in the PutKeyComboBox.
     *
     * @param key - The DatabaseEntry containing the key to be displayed.
     */
    @Override
    public void setPutKeyDatabaseEntry(DatabaseEntry key) {
        setTextField(key, PutKeyTextField, PutKeyComboBox);
    }

    /**
     * This function converts the text entered into the GetKeyTextField
     * into a DatabaseEntry using the type selected in the GetKeyComboBox.
     *
     * @return - The DatabaseEntry for GetKeyTextField.
     */
    @Override
    public DatabaseEntry getGetKeyDatabaseEntry() {
        return getDatabaseEntry(GetKeyTextField, GetKeyComboBox);
    }

    /**
     * Displays the data in the given DatabaseEntry in the GetKeyTextField,
     * using the type selected in the GetKeyComboBox.
     *
     * @param key - The DatabaseEntry containing the key to be displayed.
     */
    @Override
    public void setGetKeyDatabaseEntry(DatabaseEntry key) {
        currentGetKey = key;
        setTextField(key, GetKeyTextField, GetKeyComboBox);
    }

    /**
     * Checks if a value has been entered into GetKeyTextField.
     *
     * @return - True if the GetKeyTextField contains text, false otherwise.
     */
    @Override
    public boolean getGetKeySet() {
        String keyString = GetKeyTextField.getText();
        return !(keyString == null || keyString.length() == 0);
    }

    /**
     * Whether new records should be appended or put into the database.
     *
     * @return - True, since btree and hash records are put, not appended, into
     * the database.
     */
    @Override
    public boolean getAppendRecords() {
        return false;
    }
}
