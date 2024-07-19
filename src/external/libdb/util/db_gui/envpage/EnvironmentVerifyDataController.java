/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import db_gui.envpage.BDBEnvInitializable;
import db_gui.envpage.VerifyDataTask;

import java.io.File;
import java.net.URL;
import java.util.List;
import java.util.ResourceBundle;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.TextField;
import javafx.stage.FileChooser;

/**
 * EnvironmentVerifyDataController implements the Verify Data tab of the
 * Environment Page.
 */
public class EnvironmentVerifyDataController extends BDBEnvInitializable {
    @FXML
    private TextField ExtFileDirectoryTextField;
    @FXML
    private TextField VerifyDatabasesTextField;
    private File[] databaseFiles;
    final static private String FXMLResource =
            "/db_gui/envpage/FXMLEnvironmentVerifyDataTab.fxml";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        databaseFiles = new File[0];
        super.initialize(url, rb);
    }

    /**
     * Clears all the fields when the Clear button is pressed.
     */
    @Override
    public void clearAllFields() {
        ExtFileDirectoryTextField.clear();
        VerifyDatabasesTextField.clear();
        databaseFiles = new File[0];
        super.clearAllFields();
    }

    /**
     * Handles the External File directory chooser.  When a new directory
     * is chosen, it is displayed in ExtFileDirectoryTextField.
     *
     * @param event - Unused.
     */
    @FXML
    private void selectExternalFileDirectory(ActionEvent event) {
        selectExternalFileDirectory(ExtFileDirectoryTextField);
    }

    /**
     * Handles the Database Files multi-file selector.  When files are selected
     * they are displayed in the VerifyDatabasesTextField.
     *
     * @param event - Unused.
     */
    @FXML
    private void selectVerifyDatabaseFiles(ActionEvent event) {
        FileChooser files = new FileChooser();
        List<File> dbFiles =
                files.showOpenMultipleDialog(bdbState.getMainStage());
        if (dbFiles != null && dbFiles.size() > 0) {
            databaseFiles = new File[0];
            databaseFiles = dbFiles.toArray(databaseFiles);
            String names = "";
            for (File dbFile : databaseFiles) {
                names = names + dbFile.getName() + " ";
            }
            VerifyDatabasesTextField.setText(names);
        }
    }

    /**
     * Reads databases to verify from the VerifyDatabasesTextField.
     *
     * @return - True on success, false on error.
     */
    private boolean getVerifyDatabases() {
        String home = VerifyDatabasesTextField.getText();
        if (home == null || home.isEmpty())
            return false;
        String[] dbs = home.split(",");
        databaseFiles = new File[dbs.length];
        int i = 0;
        for (String db : dbs) {
            db = db.trim();
            File file = new File(db);
            if (!file.isAbsolute()) {
                db = config.home + File.separator + db;
                file = new File(db);
            }
            if (!file.exists() || file.isDirectory()) {
                bdbState.printFeedback(
                        "Error, cannot find database:" + db);
                return false;
            }
            databaseFiles[i] = file;
            i++;
        }
        return true;
    }

    /**
     * Handles the Verify button.  It creates a new VerifyDataTask that verifies
     * the given database files in a background thread.
     *
     * @param event
     */
    @FXML
    private void handleVerifyEnvironmentButton(ActionEvent event) {
        if (!getEncryptionKey())
            return;
        if (config.getHome() == null && !getEnvironmentHome()) {
            bdbState.printFeedback(
                    "Error, please select an Environment Home directory.");
            return;
        }
        if (databaseFiles.length < 1 && !getVerifyDatabases()) {
            bdbState.printFeedback(
                    "Error, please select database files to verify.");
            return;
        }
        if (config.getExternalDir() == null &&
                !getExtFileDirectory(ExtFileDirectoryTextField))
            return;

        VerifyDataTask verify = new VerifyDataTask(config, databaseFiles);
        bdbState.printFeedback("Starting verifcation.");
        /* 
         * Disable the tabs so the user cannot change pages until the background
         * task is finished
         */
        bdbState.disable();
        Thread verifyThread = new Thread(verify);
        verifyThread.start();
    }

    /**
     * Gets the FXML file for the Verify Data tab, which is
     * envpage/FXMLEnvironmentVerifyDataTab.fxml.
     *
     * @return Returns the name of the FXML file for the Verify Data tab.
     */
    static public String getFXMLResource() {
        return FXMLResource;
    }
}
