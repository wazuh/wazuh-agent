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
import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.DatabaseType;

import java.io.IOException;
import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.PasswordField;

/**
 * DatabaseOpenController implements the Open tab of the Database Access page.
 */
public class DatabaseOpenController extends BDBDbInitializable {
    @FXML
    private PasswordField EncryptionPasswordField;
    private String encryptionKey;
    final static private String FXMLResource =
            "/db_gui/dbpage/FXMLDatabaseOpenTab.fxml";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        encryptionKey = null;
        super.initialize(url, rb);
    }

    /**
     * Clears all the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        EncryptionPasswordField.clear();
        encryptionKey = null;
        super.clearAllFields();
    }

    /**
     * Reads the encryption key from the EncryptionPasswordField.
     *
     * @return - Always true.
     */
    private boolean getEncryptionKey() {
        String key = EncryptionPasswordField.getText();
        if (key != null && key.length() > 0)
            encryptionKey = key;
        return true;
    }

    /**
     * Implements the Open button.  Opens the given database and forwards to the
     * Data Access Page.
     *
     * @param event - Unused.
     */
    @FXML
    private void handleOpenDatabaseButton(ActionEvent event) {
        DatabaseType type = DatabaseType.UNKNOWN;
        if (!getEncryptionKey())
            return;
        if (file == null && !getDatabaseFile() && getDatabaseName() == null) {
            bdbState.printFeedback(
                    "Please enter a database file or database name.");
            return;
        }
        DatabaseConfig config = new DatabaseConfig();
        if (encryptionKey != null)
            config.setEncrypted(encryptionKey);
        config.setType(type);
        if (bdbState.openDatabase(file, getDatabaseName(), config)) {
            bdbState.printFeedback("Database successfully opened.");
            try {
                type = bdbState.getDb().getConfig().getType();
            } catch (DatabaseException ex) {
                Logger.getLogger(
                        DatabaseOpenController.class.getName()).log(
                        Level.SEVERE, null, ex);
                bdbState.printFeedback(
                        "Error getting the database type: " + ex.getMessage());
                bdbState.closeDatabase();
                return;
            }
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
