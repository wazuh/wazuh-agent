/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_guitest;

import com.sleepycat.db.Database;
import com.sleepycat.db.DatabaseConfig;
import com.sleepycat.db.DatabaseEntry;
import com.sleepycat.db.DatabaseType;
import com.sleepycat.db.Environment;
import com.sleepycat.db.EnvironmentConfig;
import com.sleepycat.db.Transaction;
import com.sleepycat.db.TransactionConfig;
import db_gui.envpage.EnvConfig;

import java.io.File;

import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.RadioButton;
import javafx.scene.control.TextArea;
import javafx.stage.Stage;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import org.junit.Test;
import org.testfx.framework.junit.ApplicationTest;
import db_gui.BDBState;


/**
 * Tests the various Tabs of the Environment Page.
 */
public class EnvironmentPageTests extends ApplicationTest {
    Environment env;
    BDBState state;
    File backup;

    @Override
    public void start(Stage stage) throws Exception {
        state = BDBState.getBDBState();
        state.setMainStage(stage);
        FXMLLoader loader = new FXMLLoader(getClass().getResource(
                "/db_gui/envpage/FXMLEnvironmentPage.fxml"));
        Parent root = loader.load();

        Scene scene = new Scene(root);

        stage.setScene(scene);
        stage.show();
    }

    @BeforeClass
    public static void setupClass() {
        System.out.println("Begin test Environment Page Tests!");
    }

    /**
     * Create the environment directory, clean the directory, and start up the
     * GUI.
     *
     * @throws Exception
     */
    @Before
    public void setUp() throws Exception {
        if (env != null)
            env.close();
        env = null;
        BDBGUITestConfig.cleanEnvironment();
        BDBGUITestConfig.createEnvironmentDirectories();
        backup = new File(BDBGUITestConfig.BACKUP_DIR);
        backup.mkdirs();
        assertTrue(backup.exists());
        BDBGUITestConfig.fileRemove(BDBGUITestConfig.BACKUP_DIR);
    }

    /**
     * Clean the environment directory.
     *
     * @throws Exception
     */
    @After
    public void tearDown() throws Exception {
        state.closeEnvironment();
        if (env != null)
            env.close();
        env = null;
        BDBGUITestConfig.cleanEnvironment();
        BDBGUITestConfig.fileRemove(BDBGUITestConfig.BACKUP_DIR);
    }

    @AfterClass
    public static void tearDownClass() {
        System.out.println("Finished test Environment Page Tests!");
    }

    /**
     * Tests that the Environment Create Tab works with minimal configurations.
     *
     * @throws Exception
     */
    @Test
    public void testCreateEnvWithoutConfigs() throws Exception {
        File reg = new File(BDBGUITestConfig.REGION_FILE);
        clickOn("#EnvironmentCreateTab");
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#CreateButton");
        Thread.sleep(1000);
        assertTrue("Region file exists. ", reg.exists());
        TextArea feedbackBox = lookup("#dbFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Environment successfully created."));
        env = new Environment(
                new File(BDBGUITestConfig.ENV_DIR), EnvironmentConfig.DEFAULT);
        EnvironmentConfig config = env.getConfig();
        assertFalse(
                "Environment is not transactional.", config.getTransactional());
        clickOn("#EnvironmentPageButton");
    }

    /**
     * Tests that the Environment Create Tab works with all configurations.
     *
     * @throws Exception
     */
    @Test
    public void testCreateEnvWithConfigs() throws Exception {
        File reg = new File(BDBGUITestConfig.REGION_FILE);
        clickOn("#EnvironmentCreateTab");
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#TransactionalCheckBox");
        clickOn("#EncryptionPasswordField")
                .write(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        clickOn("#CacheSizeTextField")
                .write(BDBGUITestConfig.CACHE_SIZE_STRING);
        clickOn("#CreateButton");
        Thread.sleep(1000);
        assertTrue("Region file exists. ", reg.exists());
        TextArea feedbackBox = lookup("#dbFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Environment successfully created."));
        EnvironmentConfig config = new EnvironmentConfig();
        config.setEncrypted(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        env = new Environment(
                new File(BDBGUITestConfig.ENV_DIR), config);
        config = env.getConfig();
        assertTrue("Environment is transactional.", config.getTransactional());
        assertTrue("Cache size set.",
                config.getCacheSize() >= BDBGUITestConfig.CACHE_SIZE);
        clickOn("#EnvironmentPageButton");
    }

    /**
     * Tests that the Environment Create Tab returns errors when appropriate.
     *
     * @throws Exception
     */
    @Test
    public void testCreateEnvErrors() throws Exception {
        File reg = new File(BDBGUITestConfig.REGION_FILE);
        // Test failure to enter an environment.
        clickOn("#EnvironmentCreateTab");
        clickOn("#CreateButton");
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Error, please select an Environment Home directory."));
        // Test invalid cache size value
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#CacheSizeTextField").write("invalid value");
        clickOn("#CreateButton");
        assertTrue(feedbackBox.getText().contains(
                "Invalid value for cache size:"));
        assertFalse("Region file does not exists. ", reg.exists());
    }

    /**
     * Tests the Environment Open tab with the bare minimum configurations.
     *
     * @throws Exception
     */
    @Test
    public void testOpenEnvWithoutConfigs() throws Exception {
        File reg = new File(BDBGUITestConfig.REGION_FILE);
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, false, true);
        assertTrue("Region file does exists.", reg.exists());
        state.closeEnvironment();
        clickOn("#EnvironmentOpenTab");
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#OpenButton");
        Thread.sleep(1000);
        TextArea feedbackBox = lookup("#dbFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Environment successfully opened."));
        clickOn("#EnvironmentPageButton");
    }

    /**
     * Tests all the configurations of the Environment Open tab.
     *
     * @throws Exception
     */
    @Test
    public void testOpenEnvWithConfigs() throws Exception {
        File reg = new File(BDBGUITestConfig.REGION_FILE);
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setEncryptionKey(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        eConfig.setCacheSize(BDBGUITestConfig.CACHE_SIZE);
        eConfig.setLogDir(new File(BDBGUITestConfig.LOG_DIR));
        eConfig.setExternalDir(new File(BDBGUITestConfig.EXT_FILE_DIR));
        eConfig.addDataDir(new File(BDBGUITestConfig.DATA1_DIR));
        eConfig.addDataDir(new File(BDBGUITestConfig.DATA2_DIR));
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        assertTrue("Region file does exists.", reg.exists());
        state.closeEnvironment();
        clickOn("#EnvironmentOpenTab");
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#EncryptionPasswordField")
                .write(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        clickOn("#CacheSizeTextField")
                .write(BDBGUITestConfig.CACHE_SIZE_STRING);
        clickOn("#DataDirectoriesTextField")
                .write(BDBGUITestConfig.DATA1_DIR
                        + ", " + BDBGUITestConfig.DATA2_DIR);
        clickOn("#LogDirectoryTextField")
                .write(BDBGUITestConfig.LOG_DIR);
        clickOn("#ExtFileDirectoryTextField")
                .write(BDBGUITestConfig.EXT_FILE_DIR);
        clickOn("#OpenButton");
        Thread.sleep(1000);
        TextArea feedbackBox = lookup("#dbFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Environment successfully opened."));
        EnvironmentConfig config = new EnvironmentConfig();
        config.setEncrypted(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        env = new Environment(
                new File(BDBGUITestConfig.ENV_DIR), config);
        config = env.getConfig();
        assertTrue("Environment is transactional.", config.getTransactional());
        assertTrue("Cache size set.",
                config.getCacheSize() >= BDBGUITestConfig.CACHE_SIZE);
        clickOn("#EnvironmentPageButton");
    }

    /**
     * Tests the Environment Open tab error conditions.
     *
     * @throws Exception
     */
    @Test
    public void testOpenEnvErrors() throws Exception {
        File reg = new File(BDBGUITestConfig.REGION_FILE);
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, false, true);
        assertTrue("Region file does exists.", reg.exists());
        state.closeEnvironment();
        // Open without an environment home.
        clickOn("#EnvironmentOpenTab");
        clickOn("#OpenButton");
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Error, please select an Environment Home directory."));
        // Invalid cache size value
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#CacheSizeTextField").write("invalid value");
        clickOn("#OpenButton");
        assertTrue(feedbackBox.getText().contains(
                "Invalid value for cache size:"));
        clickOn("#ClearButton");
        // Invalid log directory
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#LogDirectoryTextField").write("NOTEXIST");
        clickOn("#OpenButton");
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find directory:"));
        clickOn("#ClearButton");
        // Invalid external file directory
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#ExtFileDirectoryTextField").write("NOTEXIST");
        clickOn("#OpenButton");
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find directory:"));
        clickOn("#ClearButton");
        // Invalid data directory
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#DataDirectoriesTextField").write("NOTEXIST");
        clickOn("#OpenButton");
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find directory:"));
        clickOn("#ClearButton");
    }

    /**
     * Test the Environment Backup Tab with the bare minimum of configurations.
     *
     * @throws Exception
     */
    @Test
    public void testBackupEnvWithoutConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        clickOn("#EnvironmentBackupTab");
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#BackupDirectoryTextField")
                .write(BDBGUITestConfig.BACKUP_DIR);
        clickOn("#BackupButton");
        /*
         * The backup process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(3000);
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Environment backup successful."));
        File db1 = new File(BDBGUITestConfig.BACKUP_DIR
                + File.separator + BDBGUITestConfig.DATABASE_NAME1);
        File db2 = new File(BDBGUITestConfig.BACKUP_DIR
                + File.separator + BDBGUITestConfig.DATABASE_NAME2);
        File logs = new File(BDBGUITestConfig.BACKUP_DIR
                + File.separator + BDBGUITestConfig.LOG_FILE);
        assertTrue(db1.exists());
        assertTrue(db2.exists());
        assertTrue(logs.exists());
    }

    /**
     * Test the configurations of the Environment Backup tab.
     *
     * @throws Exception
     */
    @Test
    public void testBackupEnvWithConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setEncryptionKey(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        eConfig.setCacheSize(BDBGUITestConfig.CACHE_SIZE);
        eConfig.setLogDir(new File(BDBGUITestConfig.LOG_DIR));
        eConfig.setExternalDir(new File(BDBGUITestConfig.EXT_FILE_DIR));
        eConfig.addDataDir(new File(BDBGUITestConfig.DATA1_DIR));
        eConfig.addDataDir(new File(BDBGUITestConfig.DATA2_DIR));
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        clickOn("#EnvironmentBackupTab");
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#BackupDirectoryTextField")
                .write(BDBGUITestConfig.BACKUP_DIR);
        clickOn("#EncryptionPasswordField")
                .write(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        clickOn("#DataDirectoriesTextField")
                .write(BDBGUITestConfig.DATA1_DIR
                        + ", " + BDBGUITestConfig.DATA2_DIR);
        clickOn("#LogDirectoryTextField")
                .write(BDBGUITestConfig.LOG_DIR);
        clickOn("#ExtFileDirectoryTextField")
                .write(BDBGUITestConfig.EXT_FILE_DIR);
        clickOn("#CleanCheckBox");
        clickOn("#LogsRadioButton");
        clickOn("#BackupButton");
        /*
         * The backup process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(3000);
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Environment backup successful."));
        assertFalse(feedbackBox.getText().contains(
                BDBGUITestConfig.DATABASE_NAME2));
        File db1 = new File(BDBGUITestConfig.BACKUP_DIR
                + File.separator + BDBGUITestConfig.DATABASE_NAME1);
        File db2 = new File(BDBGUITestConfig.BACKUP_DIR
                + File.separator + BDBGUITestConfig.DATABASE_NAME2);
        File logs = new File(BDBGUITestConfig.BACKUP_DIR
                + File.separator + BDBGUITestConfig.LOG_FILE);
        assertFalse(db1.exists());
        assertFalse(db2.exists());
        assertTrue(logs.exists());
    }

    /**
     * Test error condition in the Environment Backup Tab.
     *
     * @throws Exception
     */
    @Test
    public void testBackupEnvErrors() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, false, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        // No environment directory
        clickOn("#EnvironmentBackupTab");
        clickOn("#BackupDirectoryTextField")
                .write(BDBGUITestConfig.BACKUP_DIR);
        clickOn("#BackupButton");
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Error, please select an Environment Home directory."));
        clickOn("#ClearButton");
        // No backup directory
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#BackupButton");
        assertTrue(feedbackBox.getText().contains(
                "Error, please select a Backup Environment Home directory."));
        clickOn("#ClearButton");
        // Invalid log directory
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#BackupDirectoryTextField")
                .write(BDBGUITestConfig.BACKUP_DIR);
        clickOn("#LogDirectoryTextField").write("NOTEXIST");
        clickOn("#BackupButton");
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find directory:"));
        clickOn("#ClearButton");
        // Invalid external file directory
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#BackupDirectoryTextField")
                .write(BDBGUITestConfig.BACKUP_DIR);
        clickOn("#ExtFileDirectoryTextField").write("NOTEXIST");
        clickOn("#BackupButton");
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find directory:"));
        clickOn("#ClearButton");
        // Invalid data directory
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#BackupDirectoryTextField")
                .write(BDBGUITestConfig.BACKUP_DIR);
        clickOn("#DataDirectoriesTextField").write("NOTEXIST");
        clickOn("#BackupButton");
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find directory:"));
        clickOn("#ClearButton");
        // Test the radio buttons.
        clickOn("#LogsRadioButton");
        clickOn("#DataRadioButton");
        RadioButton logsRadio = lookup("#LogsRadioButton").query();
        assertFalse(logsRadio.isSelected());
        RadioButton dataRadio = lookup("#DataRadioButton").query();
        assertTrue(dataRadio.isSelected());
        clickOn("#ClearButton");
        // Check that backup fails on a non-transactional environment.
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#BackupDirectoryTextField")
                .write(BDBGUITestConfig.BACKUP_DIR);
        clickOn("#BackupButton");
        Thread.sleep(1000);
        assertFalse(feedbackBox.getText().contains(
                "Environment backup successful."));
        // Check that the backup never happened.
        File db1 = new File(BDBGUITestConfig.BACKUP_DIR
                + File.separator + BDBGUITestConfig.DATABASE_NAME1);
        File db2 = new File(BDBGUITestConfig.BACKUP_DIR
                + File.separator + BDBGUITestConfig.DATABASE_NAME2);
        File logs = new File(BDBGUITestConfig.BACKUP_DIR
                + File.separator + BDBGUITestConfig.LOG_FILE);
        assertFalse(db1.exists());
        assertFalse(db2.exists());
        assertFalse(logs.exists());
    }

    /**
     * Test the Recovery Tab with the bare minimum of configurations.
     *
     * @throws Exception
     */
    @Test
    public void testRecoveryEnvWithoutConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        clickOn("#EnvironmentRecoveryTab");
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#RecoveryButton");
        /*
         * The recovery process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(2000);
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Environment recovery successful."));
    }

    /**
     * Test the Recovery Tab configurations.
     *
     * @throws Exception
     */
    @Test
    public void testRecoveryEnvWithConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setEncryptionKey(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        eConfig.setLogDir(new File(BDBGUITestConfig.LOG_DIR));
        eConfig.setExternalDir(new File(BDBGUITestConfig.EXT_FILE_DIR));
        eConfig.addDataDir(new File(BDBGUITestConfig.DATA1_DIR));
        eConfig.addDataDir(new File(BDBGUITestConfig.DATA2_DIR));
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        clickOn("#EnvironmentRecoveryTab");
        clickOn("#DirectoryTextField")
                .write(BDBGUITestConfig.ENV_DIR);
        clickOn("#EncryptionPasswordField")
                .write(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        clickOn("#DataDirectoriesTextField")
                .write(BDBGUITestConfig.DATA1_DIR
                        + ", " + BDBGUITestConfig.DATA2_DIR);
        clickOn("#LogDirectoryTextField")
                .write(BDBGUITestConfig.LOG_DIR);
        clickOn("#ExtFileDirectoryTextField")
                .write(BDBGUITestConfig.EXT_FILE_DIR);
        clickOn("#CatastrophicCheckBox");
        clickOn("#RecoveryButton");
        /*
         * The recovery process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(2000);
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Environment recovery successful."));
    }

    /**
     * Test the Recovery Tab error conditions.
     *
     * @throws Exception
     */
    @Test
    public void testRecoveryEnvErrors() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setEncryptionKey(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        eConfig.setLogDir(new File(BDBGUITestConfig.LOG_DIR));
        eConfig.setExternalDir(new File(BDBGUITestConfig.EXT_FILE_DIR));
        eConfig.addDataDir(new File(BDBGUITestConfig.DATA1_DIR));
        eConfig.addDataDir(new File(BDBGUITestConfig.DATA2_DIR));
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        // Open without a home dirctory
        clickOn("#EnvironmentRecoveryTab");
        clickOn("#RecoveryButton");
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Error, please select an Environment Home directory."));
        // Invalid log directory
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#LogDirectoryTextField").write("NOTEXIST");
        clickOn("#RecoveryButton");
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find directory:"));
        clickOn("#ClearButton");
        // Invalid external file directory
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#ExtFileDirectoryTextField").write("NOTEXIST");
        clickOn("#RecoveryButton");
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find directory:"));
        clickOn("#ClearButton");
        // Invalid data directory
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#DataDirectoriesTextField").write("NOTEXIST");
        clickOn("#RecoveryButton");
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find directory:"));
        clickOn("#ClearButton");
        // Recovery fails without an encryption key for encrypted environment.
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#RecoveryButton");
        Thread.sleep(1000);
        assertTrue(feedbackBox.getText().contains(
                "Encrypted environment: no encryption key supplied"));
    }

    /**
     * Test Remove Logs Tab with minimal configurations.
     *
     * @throws Exception
     */
    @Test
    public void testRemoveLogsEnvWithoutConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        File log1 = new File(
                BDBGUITestConfig.ENV_DIR + File.separator + "log.0000000001");
        File log2 = new File(
                BDBGUITestConfig.ENV_DIR + File.separator + "log.0000000002");
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        // Keep adding records until we create another log file.
        String dataString = "asdfasdiajfldjf;afd";
        Database db = state.getDb();
        DatabaseEntry key = new DatabaseEntry(dataString.getBytes());
        DatabaseEntry data = new DatabaseEntry(dataString.getBytes());
        while (!log2.exists()) {
            Transaction txn = state.getEnv().beginTransaction(
                    null, TransactionConfig.DEFAULT);
            for (int i = 0; i < 1000; i++) {
                db.put(txn, key, data);
            }
            txn.commit();
        }
        state.closeDatabase();
        state.closeEnvironment();
        clickOn("#EnvironmentRemoveLogsTab");
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#RemoveLogsButton");
        /*
         * The recovery process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(2000);
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Old logs successfully removed."));
        assertTrue(log2.exists());
        assertFalse(log1.exists());
    }

    /**
     * Test Remove Logs Tab with all configurations.
     *
     * @throws Exception
     */
    @Test
    public void testRemoveLogsEnvWithConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        File log1 = new File(BDBGUITestConfig.ENV_DIR
                + File.separator + BDBGUITestConfig.LOG_DIR
                + File.separator + "log.0000000001");
        File log2 = new File(BDBGUITestConfig.ENV_DIR
                + File.separator + BDBGUITestConfig.LOG_DIR
                + File.separator + "log.0000000002");
        EnvConfig eConfig = new EnvConfig();
        eConfig.setEncryptionKey(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        eConfig.setLogDir(new File(BDBGUITestConfig.LOG_DIR));
        eConfig.setExternalDir(new File(BDBGUITestConfig.EXT_FILE_DIR));
        eConfig.addDataDir(new File(BDBGUITestConfig.DATA1_DIR));
        eConfig.addDataDir(new File(BDBGUITestConfig.DATA2_DIR));
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        // Keep adding records until we create another log file.
        String dataString = "asdfasdiajfldjf;afd";
        Database db = state.getDb();
        DatabaseEntry key = new DatabaseEntry(dataString.getBytes());
        DatabaseEntry data = new DatabaseEntry(dataString.getBytes());
        while (!log2.exists()) {
            Transaction txn = state.getEnv().beginTransaction(
                    null, TransactionConfig.DEFAULT);
            for (int i = 0; i < 1000; i++) {
                db.put(txn, key, data);
            }
            txn.commit();
        }
        state.closeDatabase();
        state.closeEnvironment();
        clickOn("#EnvironmentRemoveLogsTab");
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#EncryptionPasswordField")
                .write(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        clickOn("#DataDirectoriesTextField")
                .write(BDBGUITestConfig.DATA1_DIR
                        + ", " + BDBGUITestConfig.DATA2_DIR);
        clickOn("#LogDirectoryTextField").write(BDBGUITestConfig.LOG_DIR);
        clickOn("#ExtFileDirectoryTextField")
                .write(BDBGUITestConfig.EXT_FILE_DIR);
        clickOn("#RemoveLogsButton");
        /*
         * The recovery process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(2000);
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Old logs successfully removed."));
        assertTrue(log2.exists());
        assertFalse(log1.exists());
    }

    /**
     * Test error conditions in the Remove Logs Tab.
     *
     * @throws Exception
     */
    @Test
    public void testRemoveLogsEnvErrors() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        File log1 = new File(BDBGUITestConfig.ENV_DIR
                + File.separator + BDBGUITestConfig.LOG_DIR
                + File.separator + "log.0000000001");
        File log2 = new File(BDBGUITestConfig.ENV_DIR
                + File.separator + BDBGUITestConfig.LOG_DIR
                + File.separator + "log.0000000002");
        EnvConfig eConfig = new EnvConfig();
        eConfig.setLogDir(new File(BDBGUITestConfig.LOG_DIR));
        eConfig.setExternalDir(new File(BDBGUITestConfig.EXT_FILE_DIR));
        eConfig.addDataDir(new File(BDBGUITestConfig.DATA1_DIR));
        eConfig.addDataDir(new File(BDBGUITestConfig.DATA2_DIR));
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        // Keep adding records until we create another log file.
        String dataString = "asdfasdiajfldjf;afd";
        Database db = state.getDb();
        DatabaseEntry key = new DatabaseEntry(dataString.getBytes());
        DatabaseEntry data = new DatabaseEntry(dataString.getBytes());
        while (!log2.exists()) {
            Transaction txn = state.getEnv().beginTransaction(
                    null, TransactionConfig.DEFAULT);
            for (int i = 0; i < 1000; i++) {
                db.put(txn, key, data);
            }
            txn.commit();
        }
        state.closeDatabase();
        state.closeEnvironment();
        clickOn("#EnvironmentRemoveLogsTab");
        clickOn("#RemoveLogsButton");
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Error, please select an Environment Home directory."));
        // Invalid log directory
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#LogDirectoryTextField").write("NOTEXIST");
        clickOn("#RemoveLogsButton");
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find directory:"));
        clickOn("#ClearButton");
        // Invalid external file directory
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#ExtFileDirectoryTextField").write("NOTEXIST");
        clickOn("#RemoveLogsButton");
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find directory:"));
        clickOn("#ClearButton");
        // Invalid data directory
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#DataDirectoriesTextField").write("NOTEXIST");
        clickOn("#RemoveLogsButton");
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find directory:"));
        clickOn("#ClearButton");

        // Test that the process fails the the log directory is not included
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#RemoveLogsButton");
        /*
         * The recovery process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(2000);
        feedbackBox = lookup("#environmentFeedbackBox").query();
        assertFalse(feedbackBox.getText().contains("DB_RUNRECOVERY."));
        assertTrue(log2.exists());
        assertTrue(log1.exists());
    }

    /**
     * Test the Upgrade Tab with the bare minimum of configurations.
     *
     * @throws Exception
     */
    @Test
    public void testUpgradeEnvWithoutConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        clickOn("#EnvironmentUpgradeTab");
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#UpgradeDatabasesTextField").write(
                BDBGUITestConfig.DATABASE_NAME1
                        + ", " + BDBGUITestConfig.DATABASE_NAME2);
        clickOn("#UpgradeButton");
        /*
         * The upgrade process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(2000);
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertFalse(feedbackBox.getText().contains("Error upgrading database"));
    }

    /**
     * Test the Upgrade Tab configurations.
     *
     * @throws Exception
     */
    @Test
    public void testUpgradeEnvWithConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        eConfig.setEncryptionKey(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        clickOn("#EnvironmentUpgradeTab");
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#UpgradeDatabasesTextField").write(
                BDBGUITestConfig.DATABASE_NAME1
                        + ", " + BDBGUITestConfig.DATABASE_NAME2);
        clickOn("#EncryptionPasswordField")
                .write(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        clickOn("#UpgradeButton");
        /*
         * The upgrade process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(2000);
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertFalse(feedbackBox.getText().contains("Error upgrading database"));
    }

    /**
     * Test the Upgrade Tab errors.
     *
     * @throws Exception
     */
    @Test
    public void testUpgradeEnvErrors() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        // Test no home directory
        clickOn("#EnvironmentUpgradeTab");
        clickOn("#UpgradeDatabasesTextField").write(
                BDBGUITestConfig.DATABASE_NAME1
                        + ", " + BDBGUITestConfig.DATABASE_NAME2);
        clickOn("#UpgradeButton");
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Error, please select an Environment Home directory."));
        clickOn("#ClearButton");
        // Test no databases to upgrade.
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#UpgradeButton");
        feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Error, please select database files to upgrade."));
    }

    /**
     * Test the Verify Data Tab with the bare minimum of configurations.
     *
     * @throws Exception
     */
    @Test
    public void testVerifyDataEnvWithoutConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, false, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        clickOn("#EnvironmentVerifyDataTab");
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#VerifyDatabasesTextField").write(
                BDBGUITestConfig.DATABASE_NAME1
                        + ", " + BDBGUITestConfig.DATABASE_NAME2);
        clickOn("#VerifyDataButton");
        /*
         * The upgrade process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(2000);
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Success validating database"));
    }

    /**
     * Test the Verify Data Tab configurations.
     *
     * @throws Exception
     */
    @Test
    public void testVerifyDataEnvWithConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        eConfig.setEncryptionKey(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        eConfig.setExternalDir(new File(BDBGUITestConfig.EXT_FILE_DIR));
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        clickOn("#EnvironmentVerifyDataTab");
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#ExtFileDirectoryTextField")
                .write(BDBGUITestConfig.EXT_FILE_DIR);
        clickOn("#VerifyDatabasesTextField").write(
                BDBGUITestConfig.DATABASE_NAME1
                        + ", " + BDBGUITestConfig.DATABASE_NAME2);
        clickOn("#EncryptionPasswordField")
                .write(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        clickOn("#VerifyDataButton");
        /*
         * The upgrade process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(2000);
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Success validating database"));
    }

    /**
     * Test the Verify Data Tab errors.
     *
     * @throws Exception
     */
    @Test
    public void testVerifyDataErrors() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        eConfig.setEncryptionKey(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        eConfig.setExternalDir(new File(BDBGUITestConfig.EXT_FILE_DIR));
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        // Test without environment directory
        clickOn("#EnvironmentVerifyDataTab");
        clickOn("#VerifyDatabasesTextField").write(
                BDBGUITestConfig.DATABASE_NAME1
                        + ", " + BDBGUITestConfig.DATABASE_NAME2);
        clickOn("#VerifyDataButton");
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Error, please select an Environment Home directory."));
        clickOn("#ClearButton");
        // Test without databases to verify
        clickOn("#EnvironmentVerifyDataTab");
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#VerifyDataButton");
        feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Error, please select an Environment Home directory."));
        clickOn("#ClearButton");
        // Test with invalid external files directory
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#VerifyDatabasesTextField").write(
                BDBGUITestConfig.DATABASE_NAME1
                        + ", " + BDBGUITestConfig.DATABASE_NAME2);
        clickOn("#ExtFileDirectoryTextField").write("NOTEXIST");
        clickOn("#VerifyDataButton");
        feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find directory:"));
        clickOn("#ClearButton");
        // Test without entering an enryption password on an encrypted database
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#ExtFileDirectoryTextField")
                .write(BDBGUITestConfig.EXT_FILE_DIR);
        clickOn("#VerifyDatabasesTextField").write(
                BDBGUITestConfig.DATABASE_NAME1
                        + ", " + BDBGUITestConfig.DATABASE_NAME2);
        clickOn("#VerifyDataButton");
        /*
         * The upgrade process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(2000);
        feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Encrypted environment: no encryption key supplied"));
    }

    /**
     * Test the Verify Log Tab with the bare minimum of configurations.
     *
     * @throws Exception
     */
    @Test
    public void testVerifyLogEnvWithoutConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        clickOn("#EnvironmentVerifyLogTab");
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#VerifyLogButton");
        /*
         * The upgrade process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(2000);
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Log verification succeeded."));
    }

    /**
     * Test the Verify Log Tab configurations.
     *
     * @throws Exception
     */
    @Test
    public void testVerifyLogEnvWithConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setEncryptionKey(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        eConfig.setCacheSize(BDBGUITestConfig.CACHE_SIZE);
        eConfig.setLogDir(new File(BDBGUITestConfig.LOG_DIR));
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        clickOn("#EnvironmentVerifyLogTab");
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#LogDirectoryTextField").write(BDBGUITestConfig.LOG_DIR);
        clickOn("#TempDirectoryTextField").write(BDBGUITestConfig.BACKUP_DIR);
        clickOn("#EncryptionPasswordField")
                .write(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        clickOn("#CacheSizeTextField")
                .write(BDBGUITestConfig.CACHE_SIZE_STRING);
        clickOn("#VerifyLogButton");
        /*
         * The upgrade process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(2000);
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Log verification succeeded."));
    }

    /**
     * Test the Verify Log Tab errors.
     *
     * @throws Exception
     */
    @Test
    public void testVerifyLogErrors() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setEncryptionKey(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        eConfig.setCacheSize(BDBGUITestConfig.CACHE_SIZE);
        eConfig.setLogDir(new File(BDBGUITestConfig.LOG_DIR));
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setType(DatabaseType.BTREE);
        dConfig.setAllowCreate(true);
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME1, null, dConfig);
        state.closeDatabase();
        state.openDatabase(BDBGUITestConfig.DATABASE_NAME2, null, dConfig);
        state.closeDatabase();
        state.closeEnvironment();
        // No environment directory.
        clickOn("#EnvironmentVerifyLogTab");
        clickOn("#VerifyLogButton");
        TextArea feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Error, please select an Environment Home directory."));
        clickOn("#ClearButton");
        // Invalid temporary directory.
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#TempDirectoryTextField").write("NOEXIST");
        clickOn("#VerifyLogButton");
        feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find the temporary home directory."));
        clickOn("#ClearButton");
        // Invalid log directory
        clickOn("#LogDirectoryTextField").write("NOEXIST");
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#VerifyLogButton");
        feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Error, cannot find directory:"));
        clickOn("#ClearButton");
        // Invalid cache size value
        clickOn("#CacheSizeTextField").write("abcde");
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#VerifyLogButton");
        feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Invalid value for cache size:"));
        clickOn("#ClearButton");
        // Open encrypted environmetn without encryption.
        clickOn("#DirectoryTextField").write(BDBGUITestConfig.ENV_DIR);
        clickOn("#LogDirectoryTextField").write(BDBGUITestConfig.LOG_DIR);
        clickOn("#VerifyLogButton");
        /*
         * The upgrade process takes place as a background thread, so sleep to
         * wait for the results.
         */
        Thread.sleep(2000);
        feedbackBox = lookup("#environmentFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Encrypted environment: no encryption key supplied"));
    }
}
