/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.dbpage;

import db_gui.dbpage.BDBDbInitializable;
import com.sleepycat.db.DatabaseConfig;
import com.sleepycat.db.DatabaseType;

import java.io.IOException;
import java.net.URL;
import java.util.ResourceBundle;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.ComboBox;
import javafx.scene.control.TextField;

/**
 * DatabaseCreateController implements the Create tab of the Database Access
 * page.
 */
public class DatabaseCreateController extends BDBDbInitializable {
    @FXML
    private ComboBox<String> PageSizeComboBox;
    @FXML
    private ComboBox<String> DatabaseTypeComboBox;
    @FXML
    private TextField DatabaseFileEditTextField;
    private DatabaseType type;
    private int pageSize;
    final static private String FXMLResource =
            "/db_gui/dbpage/FXMLDatabaseCreateTab.fxml";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        type = DatabaseType.UNKNOWN;
        pageSize = 0;
        DatabaseTypeComboBox.getItems().addAll(
                "btree",
                "hash",
                "recno",
                "heap",
                "queue"
        );
        DatabaseTypeComboBox.getSelectionModel().selectFirst();
        PageSizeComboBox.getItems().addAll(
                "512",
                "1024",
                "2048",
                "4096",
                "8192",
                "16384",
                "32768",
                "65536"
        );
        super.initialize(url, rb);
    }

    /**
     * Clears all the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        DatabaseTypeComboBox.getSelectionModel().selectFirst();
        PageSizeComboBox.getSelectionModel().clearSelection();
        DatabaseFileEditTextField.clear();
        pageSize = 0;
        type = DatabaseType.UNKNOWN;
        super.clearAllFields();
    }

    /**
     * Get the type of the new database from the DatabaseTypeComboBox.
     *
     * @return The access method of the new database.
     */
    private DatabaseType getDatabaseType() {
        String typeString =
                DatabaseTypeComboBox.getSelectionModel().getSelectedItem();
        switch (typeString) {
            case "btree":
                type = DatabaseType.BTREE;
                break;
            case "hash":
                type = DatabaseType.HASH;
                break;
            case "heap":
                type = DatabaseType.HEAP;
                break;
            case "queue":
                type = DatabaseType.QUEUE;
                break;
            case "recno":
                type = DatabaseType.RECNO;
                break;
            default:
                type = DatabaseType.UNKNOWN;
        }
        return type;
    }

    /**
     * Get the page size selected from the PageSizeComboBox.
     *
     * @return - True if the page size was successfully read, false if there was
     * an error.
     */
    private boolean getPageSize() {
        String sizeString =
                PageSizeComboBox.getSelectionModel().getSelectedItem();
        if (sizeString == null)
            return true;
        try {
            pageSize = Integer.parseInt(sizeString);
        } catch (NumberFormatException ex) {
            bdbState.printFeedback(
                    "Invalid value for the page size: " + sizeString);
            return false;
        }
        return true;
    }

    /**
     * Implements the Create button.  Creates a new database and forwards to the
     * Data Access page.
     *
     * @param event
     */
    @FXML
    private void handleCreateDatabaseButton(ActionEvent event) {
        String dbFile = DatabaseFileEditTextField.getText();
        String dbName = getDatabaseName();
        if (dbName != null && dbName.length() == 0)
            dbName = null;
        if (dbFile != null && dbFile.length() == 0)
            dbFile = null;
        if (dbFile == null && dbName == null) {
            bdbState.printFeedback(
                    "Please enter a database file or database name.");
            return;
        }
        if (getDatabaseType() == DatabaseType.UNKNOWN) {
            bdbState.printFeedback(
                    "Please select a database type.");
            return;
        }
        if (!getPageSize())
            return;

        DatabaseConfig config = new DatabaseConfig();
        config.setAllowCreate(true);
        config.setType(type);
        if (type == DatabaseType.QUEUE) {
            config.setRecordLength(64);
            config.setRecordPad(0);
        }
        if (pageSize != 0)
            config.setPageSize(pageSize);
        if ((dbFile != null && dbName != null) &&
                (type == DatabaseType.QUEUE || type == DatabaseType.HEAP)) {
            bdbState.printFeedback(
                    "Heap and queue databases do not support multiple databases per file.");
            return;
        }
        if (bdbState.openDatabase(dbFile, dbName, config)) {
            bdbState.printFeedback("Database successfully created.");
            try {
                forwardDataAccessPage(bdbState, type);
            } catch (IOException ex) {
                bdbState.printFeedback(
                        "Error forwarding to data access page: "
                                + ex.getMessage());
                bdbState.closeDatabase();
            }
        }
    }
}
