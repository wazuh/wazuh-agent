/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.datapage;

import com.sleepycat.db.DatabaseEntry;
import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.DeadlockException;
import com.sleepycat.db.LockMode;
import com.sleepycat.db.OperationStatus;

import java.net.URL;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.ComboBox;
import javafx.scene.control.TextField;
import db_gui.BDBInitializable;

/**
 * BDBDataInitializeable is the parent class of all the controllers of the
 * Data Access Page.  It implements operations that are part of all or most of
 * each of the data access pages for the different database access methods, such
 * as translating the the Get and Put data TextFields.
 */
public abstract class BDBDataInitializable extends BDBInitializable {
    @FXML
    protected TextField GetDataTextField;
    @FXML
    protected ComboBox<String> GetDataComboBox;
    @FXML
    protected TextField PutDataTextField;
    @FXML
    protected ComboBox<String> PutDataComboBox;
    protected DatabaseEntry currentGetData;
    final static protected String[] dataFormats = {"ASCII",
            "UTF-8", "UTF-16", "Integer 32", "Float 32", "Integer 64",
            "Float 64"};

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL location, ResourceBundle resources) {
        GetDataComboBox.getItems().addAll(dataFormats);
        GetDataComboBox.getSelectionModel().selectFirst();
        // The Queue Data Access page does not have a PutDataComboBox.
        if (PutDataComboBox != null) {
            PutDataComboBox.getItems().addAll(dataFormats);
            PutDataComboBox.getSelectionModel().selectFirst();
        }
    }

    /**
     * Clears the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        GetDataTextField.clear();
        GetDataComboBox.getSelectionModel().selectFirst();
        if (PutDataTextField != null)
            PutDataTextField.clear();
        if (PutDataComboBox != null)
            PutDataComboBox.getSelectionModel().selectFirst();
        currentGetData = null;
        bdbState.closeCursor();
    }

    /**
     * Get the Put Key DatabaseEntry from the inheriting class.
     *
     * @return The Put Key DatabaseEntry.
     */
    public abstract DatabaseEntry getPutKeyDatabaseEntry();

    /**
     * Display the Put Key.
     *
     * @param key - The DatabaseEntry of the Put Key.
     */
    public abstract void setPutKeyDatabaseEntry(DatabaseEntry key);

    /**
     * Get the Get Key DatabaseEntry from the inheriting class.
     *
     * @return The Get Key DatabaseEntry.
     */
    public abstract DatabaseEntry getGetKeyDatabaseEntry();

    /**
     * Display the Get Key.
     *
     * @param key - The DatabaseEntry of the Get Key.
     */
    public abstract void setGetKeyDatabaseEntry(DatabaseEntry key);

    /**
     * Whether the Get Key TextField has text in it or not.
     *
     * @return True if the Get Key TextField contains text, false otherwise.
     */
    public abstract boolean getGetKeySet();

    /**
     * Whether to append or put new records into the database.
     *
     * @return True if to append new records, false if to put.
     */
    public abstract boolean getAppendRecords();


    /**
     * Returns the given data string as a DatabaseEntry.  The string
     * is translated into bytes based on what type is passed in.  Null
     * is returned if there is a parsing error.
     *
     * @param data - The data to be made into a DatabaseEntry.
     * @param type - How to translate the data String into binary data.
     * @return - The DatabaseEntry created from the data.
     */
    protected DatabaseEntry getDatabaseEntry(String data, String type) {
        if (data == null)
            return null;
        if (type == null)
            return new DatabaseEntry(
                    data.getBytes(StandardCharsets.US_ASCII));
        switch (type) {
            case ("ASCII"):
                return new DatabaseEntry(
                        data.getBytes(StandardCharsets.US_ASCII));
            case ("UTF-8"):
                return new DatabaseEntry(
                        data.getBytes(StandardCharsets.UTF_8));
            case ("UTF-16"):
                return new DatabaseEntry(
                        data.getBytes(StandardCharsets.UTF_16));
            case ("Integer 32"):
                int ivalue;
                try {
                    ivalue = Integer.parseInt(data);
                } catch (NumberFormatException ex) {
                    bdbState.printFeedback(
                            "Error, cannot parse value \""
                                    + data + "\" into a 32-bit integer.");
                    bdbState.printFeedback("Pleae enter another value.");
                    return null;
                }
                ByteBuffer bbuf = ByteBuffer.allocate(Integer.BYTES);
                bbuf.putInt(ivalue);
                return new DatabaseEntry(bbuf.array());
            case ("Float 32"):
                float fvalue;
                try {
                    fvalue = Float.parseFloat(data);
                } catch (NumberFormatException ex) {
                    bdbState.printFeedback(
                            "Error, cannot parse value \""
                                    + data + "\" into a 32-bit float.");
                    bdbState.printFeedback("Pleae enter another value.");
                    return null;
                }
                bbuf = ByteBuffer.allocate(Float.BYTES);
                bbuf.putFloat(fvalue);
                return new DatabaseEntry(bbuf.array());
            case ("Integer 64"):
                long lvalue;
                try {
                    lvalue = Long.parseLong(data);
                } catch (NumberFormatException ex) {
                    bdbState.printFeedback(
                            "Error, cannot parse value \""
                                    + data + "\" into a 64-bit integer.");
                    bdbState.printFeedback("Pleae enter another value.");
                    return null;
                }
                bbuf = ByteBuffer.allocate(Long.BYTES);
                bbuf.putLong(lvalue);
                return new DatabaseEntry(bbuf.array());
            case ("Float 64"):
                double dvalue;
                try {
                    dvalue = Double.parseDouble(data);
                } catch (NumberFormatException ex) {
                    bdbState.printFeedback(
                            "Error, cannot parse value \""
                                    + data + "\" into a 64-bit float.");
                    bdbState.printFeedback("Pleae enter another value.");
                    return null;
                }
                bbuf = ByteBuffer.allocate(Double.BYTES);
                bbuf.putDouble(dvalue);
                return new DatabaseEntry(bbuf.array());
            default:
                return null;
        }
    }

    /**
     * Takes a DatabaseEntry and type and returns a string that displays
     * the data based on the type.
     *
     * @param data - The DatabaseEntry to translate into a String.
     * @param type - How to translate the binary DatabaseEntry data into a
     * String.
     * @return The String into which the data was translated.
     */
    protected String getDatabaseEntryString(DatabaseEntry data, String type) {
        if (data == null
                || data.getData() == null || data.getData().length == 0)
            return "";
        switch (type) {
            case ("ASCII"):
                return new String(data.getData(), StandardCharsets.US_ASCII);
            case ("UTF-8"):
                return new String(data.getData(), StandardCharsets.UTF_8);
            case ("UTF-16"):
                return new String(data.getData(), StandardCharsets.UTF_16);
            case ("Integer 32"):
                ByteBuffer bbuf = ByteBuffer.wrap(data.getData());
                int ivalue;
                try {
                    ivalue = bbuf.getInt();
                } catch (BufferUnderflowException ex) {
                    bdbState.printFeedback(
                            "Error, cannot display value as an integer.");
                    return null;
                }
                return Integer.toString(ivalue);
            case ("Float 32"):
                float fvalue;
                bbuf = ByteBuffer.wrap(data.getData());
                try {
                    fvalue = bbuf.getFloat();
                } catch (BufferUnderflowException ex) {
                    bdbState.printFeedback(
                            "Error, cannot display value as a float.");
                    return null;
                }
                return Float.toString(fvalue);
            case ("Integer 64"):
                long lvalue;
                bbuf = ByteBuffer.wrap(data.getData());
                try {
                    lvalue = bbuf.getLong();
                } catch (BufferUnderflowException ex) {
                    bdbState.printFeedback(
                            "Error, cannot display value as a 64-bit integer.");
                    return null;
                }
                return Long.toString(lvalue);
            case ("Float 64"):
                double dvalue;
                bbuf = ByteBuffer.wrap(data.getData());
                try {
                    dvalue = bbuf.getDouble();
                } catch (BufferUnderflowException ex) {
                    bdbState.printFeedback(
                            "Error, cannot display value as a 64-bit float.");
                    return null;
                }
                return Double.toString(dvalue);
            default:
                return null;
        }
    }

    /**
     * Changes how the data in the text field is displayed.
     *
     * @param dbEntry - The DatabaseEntry containing the binary data to be
     * displayed.
     * @param textField - The TextField in which to display the data.
     * @param comboBox - The ComboBox containing how to translate the binary
     * data into a String.
     */
    protected void changeDataDisplay(DatabaseEntry dbEntry,
            TextField textField, ComboBox<String> comboBox) {
        // Do nothing if the field is empty.
        if (dbEntry == null || dbEntry.getSize() == 0)
            return;
        String type = comboBox.getSelectionModel().getSelectedItem();
        if (type == null || type.length() == 0)
            return;
        String text = getDatabaseEntryString(dbEntry, type);
        if (text != null && text.length() != 0)
            textField.setText(text);
    }

    /**
     * Changes how the data in the GetDataTextField is displayed when the type
     * selected in the GetDataComboBox is changed.
     *
     * @param event - Unused.
     */
    @FXML
    protected void handleGetDataComboBox(ActionEvent event) {
        changeDataDisplay(currentGetData, GetDataTextField, GetDataComboBox);
    }

    /**
     * Handles the Put button on the Data Access page.  Either puts or appends
     * the entered key and data depending on the access method of the database.
     * Closes the current cursor if it is opened.
     *
     * @param event - Unused.
     */
    @FXML
    protected void handlePutButton(ActionEvent event) {
        String dataString;
        DatabaseEntry key, data;

        dataString = PutDataTextField.getText();
        key = getPutKeyDatabaseEntry();
        if (key == null)
            return;
        data = getDatabaseEntry(dataString,
                PutDataComboBox.getSelectionModel().getSelectedItem());
        /* data are only null if there was a parsing error. */
        if (data == null)
            return;

        try {
            /* 
             * Close the current cursor if it exists, otherwise it could cause
             * the thread to hang.
             */
            bdbState.closeCursor();
            if (getAppendRecords())
                bdbState.getDb().append(bdbState.getTxn(), key, data);
            else
                bdbState.getDb().put(bdbState.getTxn(), key, data);
            // Clear the test fields after a successful insert.
            setPutKeyDatabaseEntry(null);
            PutDataTextField.clear();
            GetDataTextField.clear();
            currentGetData = null;
            setGetKeyDatabaseEntry(null);
            bdbState.printFeedback("Inserted record sucessful.");
        } catch (DeadlockException dex) {
            handleDeadlockException();
        } catch (DatabaseException ex) {
            Logger.getLogger(
                    BtreeHashController.class.getName()).log(
                    Level.SEVERE, null, ex);
            bdbState.printFeedback(
                    "Error putting values into the database: "
                            + ex.getMessage());
        }
    }

    /**
     * Handles the Get button on the Data Access page.  Closes the current
     * cursor if it exists.
     *
     * @param event - Unused.
     */
    @FXML
    protected void handleGetButton(ActionEvent event) {
        String dataString;
        DatabaseEntry key, data;

        key = getGetKeyDatabaseEntry();
        if (key == null)
            return;
        data = new DatabaseEntry();

        try {
            /* 
             * Close the current cursor if it exists, otherwise it could cause
             * the thread to hang.
             */
            bdbState.closeCursor();
            OperationStatus op = bdbState.getDb().get(
                    bdbState.getTxn(), key, data, LockMode.DEFAULT);
            if (op == OperationStatus.NOTFOUND)
                bdbState.printFeedback("Database does not contain that key.");
            else {
                bdbState.printFeedback(
                        "Success getting record from the database.");
                setGetKeyDatabaseEntry(key);
                currentGetData = data;
                dataString = getDatabaseEntryString(data,
                        GetDataComboBox.getSelectionModel().getSelectedItem());
                GetDataTextField.setText(dataString);
            }
        } catch (DeadlockException dex) {
            handleDeadlockException();
        } catch (DatabaseException ex) {
            Logger.getLogger(
                    BtreeHashController.class.getName()).log(
                    Level.SEVERE, null, ex);
            bdbState.printFeedback(
                    "Error getting record from the database: "
                            + ex.getMessage());
        }
    }

    /**
     * Handles the Next button on the data access page.  If the cursor exists
     * then it calls the Cursor.next function, if it does not exist then it
     * opens a new Cursor and positions it at the last know position before
     * calling next, or if the is no known last position it calls getFirst.
     *
     * @param event - Unused.
     */
    @FXML
    protected void handleNextButton(ActionEvent event) {
        String dataString;
        DatabaseEntry key, data;
        boolean getFirst = false;

        if (bdbState.getCursor() == null) {
            try {
                bdbState.openCursor();
            } catch (DatabaseException ex) {
                Logger.getLogger(
                        BtreeHashController.class.getName()).log(
                        Level.SEVERE, null, ex);
                bdbState.printFeedback(
                        "Error opening database cursor: " + ex.getMessage());
                return;
            }
            getFirst = true;
            
            /*if (getGetKeySet()) {
                key = getGetKeyDatabaseEntry();
                if (key == null)
                    key = new DatabaseEntry();
                data = new DatabaseEntry();
                
                try {
                    OperationStatus op =
                        bdbState.getCursor().getSearchKey(
                            key, data, LockMode.DEFAULT);
                    if (op == OperationStatus.NOTFOUND) {
                        bdbState.printFeedback(
                                "Unable to find current key, getting " +
                                        "the first record in the database.");
                        getFirst = true;
                    }
                } catch (DeadlockException dex) {
                    handleDeadlockException();
                    bdbState.printFeedback("Please retry Next operation.");
                    return;
                } catch (DatabaseException ex) {
                    Logger.getLogger(
                            BtreeHashController.class.getName()).log(
                                    Level.SEVERE, null, ex);
                    bdbState.printFeedback(
                            "Error getting next record: " + ex.getMessage());
                    return;
                }
            } else
                getFirst = true;*/
        }
        key = new DatabaseEntry();
        data = new DatabaseEntry();
        try {
            OperationStatus op;
            if (getFirst)
                op = bdbState.getCursor().getFirst(key, data, LockMode.DEFAULT);
            else
                op = bdbState.getCursor().getNext(key, data, LockMode.DEFAULT);
            if (op == OperationStatus.NOTFOUND) {
                bdbState.printFeedback(
                        "End of the database has been reached, "
                                + "there is no next record.");
                setGetKeyDatabaseEntry(null);
                GetDataTextField.clear();
            } else {
                bdbState.printFeedback(
                        "Success getting the next record from the database.");
                currentGetData = data;
                setGetKeyDatabaseEntry(key);
                dataString = getDatabaseEntryString(data,
                        GetDataComboBox.getSelectionModel().getSelectedItem());
                GetDataTextField.setText(dataString);
            }
        } catch (DatabaseException ex) {
            Logger.getLogger(
                    BtreeHashController.class.getName()).log(
                    Level.SEVERE, null, ex);
            bdbState.printFeedback(
                    "Error getting record from the database: "
                            + ex.getMessage());
        }
    }

    /**
     * Handles the Previous button on the data access page.  If the cursor
     * exists then it calls the Cursor.previous function, if it does not exist
     * then it opens a new Cursor and positions it at the last know position
     * before calling previous, or if the is no known last position it calls
     * getLast.
     *
     * @param event - Unused.
     */
    @FXML
    protected void handlePreviousButton(ActionEvent event) {
        String dataString;
        DatabaseEntry key, data;
        boolean getLast = false;

        if (bdbState.getCursor() == null) {
            try {
                bdbState.openCursor();
            } catch (DatabaseException ex) {
                Logger.getLogger(
                        BtreeHashController.class.getName()).log(
                        Level.SEVERE, null, ex);
                bdbState.printFeedback(
                        "Error opening database cursor: " + ex.getMessage());
                return;
            }
            getLast = true;
            /*if (getGetKeySet()) {
                key = getGetKeyDatabaseEntry();
                if (key == null)
                    key = new DatabaseEntry();
                data = new DatabaseEntry();

                try {
                    OperationStatus op =
                        bdbState.getCursor().getSearchKey(
                            key, data, LockMode.DEFAULT);
                    if (op == OperationStatus.NOTFOUND) {
                        bdbState.printFeedback(
                                "Unable to find current key, getting " +
                                        "the last record in the database.");
                        getLast = true;
                    }
                } catch (DeadlockException dex) {
                    handleDeadlockException();
                    bdbState.printFeedback("Please retry Previous operation.");
                    return;
                } catch (DatabaseException ex) {
                    Logger.getLogger(
                            BtreeHashController.class.getName()).log(
                                    Level.SEVERE, null, ex);
                    bdbState.printFeedback(
                            "Error getting previous record: "
                                    + ex.getMessage());
                    return;
                }
            } else
                getLast = true;*/
        }
        key = new DatabaseEntry();
        data = new DatabaseEntry();
        try {
            OperationStatus op;
            if (getLast)
                op = bdbState.getCursor().getLast(key, data, LockMode.DEFAULT);
            else
                op = bdbState.getCursor().getPrev(key, data, LockMode.DEFAULT);
            if (op == OperationStatus.NOTFOUND) {
                bdbState.printFeedback(
                        "Beginning of the database has been reached, "
                                + "there is no previous record.");
                setGetKeyDatabaseEntry(null);
                GetDataTextField.clear();
            } else {
                currentGetData = data;
                bdbState.printFeedback(
                        "Success getting the previous record from the database.");
                setGetKeyDatabaseEntry(key);
                dataString = getDatabaseEntryString(data,
                        GetDataComboBox.getSelectionModel().getSelectedItem());
                GetDataTextField.setText(dataString);
            }
        } catch (DatabaseException ex) {
            Logger.getLogger(
                    BtreeHashController.class.getName()).log(
                    Level.SEVERE, null, ex);
            bdbState.printFeedback(
                    "Error getting record from the database: "
                            + ex.getMessage());
        }
    }

    /**
     * Handles the Delete button.  Deletes the current entry that was either
     * entered by the user, or set by the cursor.
     *
     * @param event - Unused.
     */
    @FXML
    protected void handleDeleteButton(ActionEvent event) {
        DatabaseEntry key;

        if (!getGetKeySet()) {
            bdbState.printFeedback(
                    "Error, please enter a Key value or start a cursor.");
            return;
        }
        key = getGetKeyDatabaseEntry();
        /* key is only null if there was a parsing error. */
        if (key == null)
            return;

        try {
            bdbState.closeCursor();
            OperationStatus op =
                    bdbState.getDb().delete(bdbState.getTxn(), key);
            if (op == OperationStatus.SUCCESS) {
                bdbState.printFeedback("Success deleting record.");
            } else if (op == OperationStatus.NOTFOUND) {
                bdbState.printFeedback(
                        "No such record exists in the database.");
            }
            setGetKeyDatabaseEntry(null);
            GetDataTextField.clear();
        } catch (DeadlockException dex) {
            handleDeadlockException();
        } catch (DatabaseException ex) {
            Logger.getLogger(
                    BtreeHashController.class.getName()).log(
                    Level.SEVERE, null, ex);
            bdbState.printFeedback(
                    "Error deleting record from database: "
                            + ex.getMessage());
        }
    }
}
