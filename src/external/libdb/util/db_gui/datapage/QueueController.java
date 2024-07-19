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
import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.DeadlockException;
import com.sleepycat.db.OperationStatus;

import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.ComboBox;
import javafx.scene.control.TextField;

/**
 * Implements the queue Enqueue, Dequeue, and Browse data panels of the Data
 * Access Page.
 */
public class QueueController extends BDBDataInitializable {
    @FXML
    private TextField GetKeyTextField;
    @FXML
    private TextField EnqueueTextField;
    @FXML
    private ComboBox<String> EnqueueComboBox;
    @FXML
    private TextField DequeueTextField;
    @FXML
    private ComboBox<String> DequeueComboBox;
    private DatabaseEntry currentDequeueData;
    final static private String FXMLResource =
            "/db_gui/datapage/FXMLQueue.fxml";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        EnqueueComboBox.getItems().addAll(dataFormats);
        EnqueueComboBox.getSelectionModel().selectFirst();
        DequeueComboBox.getItems().addAll(dataFormats);
        DequeueComboBox.getSelectionModel().selectFirst();
        super.initialize(url, rb);
    }

    /**
     * Clears the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        GetKeyTextField.clear();
        EnqueueComboBox.getSelectionModel().selectFirst();
        EnqueueTextField.clear();
        DequeueComboBox.getSelectionModel().selectFirst();
        DequeueTextField.clear();
        currentDequeueData = null;
        super.clearAllFields();
    }

    /**
     * Implements the Enqueue button.  Appends the data entered into the
     * EnqueueTextField onto the queue database.
     *
     * @param event - Unused.
     */
    @FXML
    private void handleEnqueueButton(ActionEvent event) {
        String dataString = EnqueueTextField.getText();
        if (dataString == null || dataString.length() == 0) {
            bdbState.printFeedback(
                    "Please enter a value in the Enqueue data field.");
            return;
        }
        DatabaseEntry data = getDatabaseEntry(dataString,
                EnqueueComboBox.getSelectionModel().getSelectedItem());
        DatabaseEntry key = new DatabaseEntry(new byte[8]);
        try {
            bdbState.closeCursor();
            bdbState.getDb().append(bdbState.getTxn(), key, data);
            // Clear the text field after a successfull insert.
            EnqueueTextField.clear();
            bdbState.printFeedback(
                    "Record successfully pushed onto the queue.");
            GetDataTextField.clear();
            currentGetData = null;
            setGetKeyDatabaseEntry(null);
        } catch (DeadlockException dex) {
            handleDeadlockException();
        } catch (DatabaseException ex) {
            Logger.getLogger(
                    QueueController.class.getName()).log(
                    Level.SEVERE, null, ex);
            bdbState.printFeedback(
                    "Error pushing record into the queue: "
                            + ex.getMessage());
        }
    }

    /**
     * Implements the Dequeue button.  Pops the first record in the queue off
     * of the queue database and displays it in the DequeueTextField.
     *
     * @param event - Unused.
     */
    @FXML
    private void handleDequeueButton(ActionEvent event) {
        DatabaseEntry data = new DatabaseEntry();
        DatabaseEntry key = new DatabaseEntry();
        try {
            bdbState.closeCursor();
            OperationStatus op = bdbState.getDb().consume(
                    bdbState.getTxn(), key, data, false);
            if (op == OperationStatus.NOTFOUND) {
                bdbState.printFeedback("No record returned, queue is empty.");
                DequeueTextField.clear();
            } else {
                bdbState.printFeedback(
                        "Record successfully popped from the queue");
                String dataString = getDatabaseEntryString(data,
                        DequeueComboBox.getSelectionModel().getSelectedItem());
                DequeueTextField.setText(dataString);
                currentDequeueData = data;
                GetDataTextField.clear();
                currentGetData = null;
                setGetKeyDatabaseEntry(null);
            }
        } catch (DatabaseException ex) {
            Logger.getLogger(
                    QueueController.class.getName()).log(
                    Level.SEVERE, null, ex);
            bdbState.printFeedback(
                    "Error popping record into the queue: "
                            + ex.getMessage());
        }
    }

    /**
     * Change how the binary data is displayed in the DequeueTextField when a
     * new type is selected in the DequeueComboBox.
     *
     * @param event - Unused.
     */
    @FXML
    protected void handleDequeueComboBox(ActionEvent event) {
        changeDataDisplay(
                currentDequeueData, DequeueTextField, DequeueComboBox);
    }

    /**
     * The Queue Data Access page does not contain a Put button (Enqueue is
     * the queue put equivalent).
     *
     * @return - null.
     */
    @Override
    public DatabaseEntry getPutKeyDatabaseEntry() {
        return null;
    }

    /**
     * The Queue Data Access page does not contain a Put button (Enqueue is
     * the queue put equivalent).
     *
     * @param key - Unused.
     */
    @Override
    public void setPutKeyDatabaseEntry(DatabaseEntry key) {
        //no-op
    }

    /**
     * Converts the data entered into a GetKeyTextField into a key DatabaseEntry
     * for a queue database, which is always an integer.
     *
     * @return - A DatabaseEntry containing a queue key.
     */
    @Override
    public DatabaseEntry getGetKeyDatabaseEntry() {
        String text = GetKeyTextField.getText();
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
     * Displays a DatabaseEntry for a key of a queue database in the
     * GetKeyTextField.  The keys for queue databases are always integers.
     *
     * @param key - DatabaseEntry containing the queue key to be displayed.
     */
    @Override
    public void setGetKeyDatabaseEntry(DatabaseEntry key) {
        String keyString;
        int recno;
        if (key == null) {
            GetKeyTextField.clear();
            return;
        }
        recno = key.getRecordNumber();
        keyString = "" + recno;
        GetKeyTextField.setText(keyString);
    }

    /**
     * Whether the GetKeyTextField has text in it or not.
     *
     * @return True if the GetKeyTextField has text in it, false otherwise.
     */
    @Override
    public boolean getGetKeySet() {
        String keyString = GetKeyTextField.getText();
        return !(keyString == null || keyString.length() == 0);
    }

    /**
     * Whether to append or put new records into the database.
     *
     * @return True, because new records are appended to queue databases.
     */
    @Override
    public boolean getAppendRecords() {
        return true;
    }

}
