/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_gui.envpage;

import db_gui.envpage.BDBEnvInitializable;
import db_gui.envpage.BackupTask;
import com.sleepycat.db.BackupOptions;

import java.io.File;
import java.net.URL;
import java.util.ResourceBundle;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.CheckBox;
import javafx.scene.control.RadioButton;
import javafx.scene.control.TextField;
import javafx.stage.DirectoryChooser;

/**
 * EnvironmentBackupController implements the Backup tab of the Environment
 * page.
 */
public class EnvironmentBackupController extends BDBEnvInitializable {
    @FXML
    private TextField DataDirectoriesTextField;
    @FXML
    private TextField LogDirectoryTextField;
    @FXML
    private TextField ExtFileDirectoryTextField;
    @FXML
    private TextField BackupDirectoryTextField;
    @FXML
    private CheckBox CleanCheckBox;
    @FXML
    private RadioButton DataRadioButton;
    @FXML
    private RadioButton LogsRadioButton;
    final static private String FXMLResource =
            "/db_gui/envpage/FXMLEnvironmentBackupTab.fxml";
    private File backupHome;

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
        BackupDirectoryTextField.clear();
        DataDirectoriesTextField.clear();
        LogDirectoryTextField.clear();
        ExtFileDirectoryTextField.clear();
        CleanCheckBox.setSelected(false);
        DataRadioButton.setSelected(false);
        LogsRadioButton.setSelected(false);
        super.clearAllFields();
    }

    /**
     * Handles the Backup Directory chooser.  When a new directory
     * is chosen, it is displayed in BackupDirectoryTextField.
     *
     * @param event - Unused.
     */
    @FXML
    private void selectBackupEnvironmentHome(ActionEvent event) {
        DirectoryChooser homeDir = new DirectoryChooser();
        File home = homeDir.showDialog(bdbState.getMainStage());
        if (home != null) {
            backupHome = home;
            BackupDirectoryTextField.setText(home.getAbsolutePath());
        }
    }

    /**
     * Handles the Log directory chooser.  When a new directory
     * is chosen, it is displayed in LogDirectoryTextField.
     *
     * @param event - Unused.
     */
    @FXML
    private void selectLogDirectory(ActionEvent event) {
        selectLogDirectory(LogDirectoryTextField);
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
     * Handles the Data Directories directory chooser.  When a new directory
     * is chosen, it is appended to the text displayed in
     * DataDirectoryTextField.
     *
     * @param event - Unused.
     */
    @FXML
    private void selectDataDirectories(ActionEvent event) {
        selectDataDirectories(DataDirectoriesTextField);
    }

    /**
     * Reads the text in BackupDirectoryTextField and converts it to a directory
     * and returns its File object.
     *
     * @return The File object for the backup directory on success,
     * null on error.
     */
    private File getBackupHome() {
        String home = BackupDirectoryTextField.getText();
        if (home == null || home.isEmpty())
            return null;
        File file = new File(home);
        if (!file.exists() || !file.isDirectory())
            return null;
        return file;
    }

    /**
     * Gets the FXML file for the Backup tab, which is
     * envpage/FXMLEnvironmentBackupTab.fxml.
     *
     * @return Returns the name of the FXML file for the Backup tab.
     */
    static public String getFXMLResource() {
        return FXMLResource;
    }

    /**
     * Implements the Backup button.  It creates a new BackupTask that
     * performs the backup operation in a background thread.
     *
     * @param event - Unused.
     */
    @FXML
    private void handleBackupEnvironmentButton(ActionEvent event) {
        BackupOptions options = new BackupOptions();
        if (!getEncryptionKey())
            return;
        if (config.getHome() == null && !getEnvironmentHome()) {
            bdbState.printFeedback(
                    "Error, please select an Environment Home directory.");
            return;
        }
        if (backupHome == null) {
            backupHome = getBackupHome();
            if (backupHome == null) {
                bdbState.printFeedback(
                        "Error, please select a Backup Environment Home directory.");
                return;
            }
        }
        if (config.getDataDirs() == null &&
                !getDataDirectories(DataDirectoriesTextField))
            return;
        if (config.getExternalDir() == null &&
                !getExtFileDirectory(ExtFileDirectoryTextField))
            return;
        if (config.getLogDir() == null &&
                !getLogDirectory(LogDirectoryTextField))
            return;
        options.setClean(CleanCheckBox.isSelected());
        options.setUpdate(LogsRadioButton.isSelected());
        options.setNoLogs(DataRadioButton.isSelected());
        options.setAllowCreate(true);
        /*
         * Required because of difficulties passing relative paths through
         * the GUI, which are needed when the source environment stores the logs
         * or data in a directory other than home.  Hopefully can fix this in 
         * the future.
         */
        options.setSingleDir(true);
        bdbState.printFeedback("Opening the environment.");
        /* 
         * Disable the tabs so the user cannot change pages until the background
         * task is finished
         */
        bdbState.disable();
        BackupTask backup = new BackupTask(config, backupHome, options);
        new Thread(backup).start();
    }

}
