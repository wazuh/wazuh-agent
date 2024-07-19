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
import com.sleepycat.db.DatabaseType;
import com.sleepycat.db.Environment;
import db_gui.envpage.EnvConfig;

import java.io.File;

import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.ComboBox;
import javafx.scene.control.TextArea;
import javafx.stage.Stage;

import static junit.framework.Assert.assertEquals;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import org.junit.Ignore;
import org.junit.Test;
import org.testfx.framework.junit.ApplicationTest;
import db_gui.BDBState;


/**
 * Tests the various Tabs of the Database Page.
 */
public class DatabasePageTests extends ApplicationTest {
    Environment env;
    Database db;
    BDBState state;
    private static final DatabaseType[] TYPES = {
            DatabaseType.BTREE,
            DatabaseType.HASH,
            DatabaseType.RECNO,
            DatabaseType.HEAP,
            DatabaseType.QUEUE
    };
    private static final String[] TYPE_NAMES = {
            "btree",
            "hash",
            "recno",
            "heap",
            "queue"
    };

    private static final String[] PAGE_SIZES = {
            "512",
            "1024",
            "4096",
            "8192",
            "65536"
    };

    @Override
    public void start(Stage stage) throws Exception {
        state = BDBState.getBDBState();
        state.setMainStage(stage);
        FXMLLoader loader = new FXMLLoader(getClass().getResource(
                "/db_gui/dbpage/FXMLDatabasePage.fxml"));
        Parent root = loader.load();

        Scene scene = new Scene(root);

        stage.setScene(scene);
        stage.show();
    }

    @BeforeClass
    public static void setupClass() {
        System.out.println("Begin test Database Page Tests!");
    }

    /**
     * Create the environment directory, clean the directory, and start up the
     * GUI.
     *
     * @throws Exception
     */
    @Before
    public void setUp() throws Exception {
        if (db != null)
            db.close();
        db = null;
        if (env != null)
            env.close();
        env = null;
        BDBGUITestConfig.cleanEnvironment();
        BDBGUITestConfig.createEnvironmentDirectories();
    }

    /**
     * Clean the environment directory.
     *
     * @throws Exception
     */
    @After
    public void tearDown() throws Exception {
        state.closeDatabase();
        if (db != null)
            db.close();
        db = null;
        state.closeEnvironment();
        if (env != null)
            env.close();
        env = null;
        BDBGUITestConfig.cleanEnvironment();
    }

    @AfterClass
    public static void tearDownClass() {
        System.out.println("Finished test Database Page Tests!");
    }

    /**
     * Tests that the Database Create Tab works with minimal configurations.
     *
     * @throws Exception
     */
    @Test
    public void testCreateDbWithoutConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, false, true);
        boolean[] inmemories = {true, false};
        for (boolean inmemory : inmemories) {
            for (int i = 0; i < TYPES.length; i++) {
                clickOn("#DatabaseCreateTab");
                if (inmemory)
                    clickOn("#DatabaseNameTextField").write(TYPE_NAMES[i]);
                else
                    clickOn("#DatabaseFileEditTextField").write(TYPE_NAMES[i]);
                /*
                 * Selecting a value from a comboBox has to be done in the
                 * JavaFX thread to avoid an exception.
                 */
                ComboBox comboBox = lookup("#DatabaseTypeComboBox").query();
                ComboBoxTask task = new ComboBoxTask(comboBox, TYPE_NAMES[i]);
                interact(task);
                clickOn("#CreateButton");
                TextArea feedbackBox = lookup("#FeedbackBox").query();
                assertTrue(feedbackBox.getText().contains(
                        "Database successfully created."));
                File dbFile = new File(BDBGUITestConfig.ENV_DIR
                        + File.separator + TYPE_NAMES[i]);
                if (inmemory)
                    assertFalse(dbFile.exists());
                else
                    assertTrue(dbFile.exists());
                assertEquals(state.getDb().getConfig().getType(), TYPES[i]);
                clickOn("#DatabasePageButton");
            }
        }
    }

    /**
     * Tests the Database Create Tab configurations.
     *
     * @throws Exception
     */
    @Test
    public void testCreateDbWithConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, false, true);
        for (int i = 0; i < TYPES.length; i++) {
            clickOn("#DatabaseCreateTab");
            clickOn("#DatabaseFileEditTextField").write(TYPE_NAMES[i]);
            // Heaps and Queues do not support subdatabases.
            if (TYPES[i] != DatabaseType.HEAP && TYPES[i] != DatabaseType.QUEUE)
                clickOn("#DatabaseNameTextField").write(TYPE_NAMES[i]);
            /*
             * Selecting a value from a comboBox has to be done in the JavaFX
             * thread to avoid an exception.
             */
            ComboBox typeComboBox = lookup("#DatabaseTypeComboBox").query();
            ComboBoxTask task = new ComboBoxTask(typeComboBox, TYPE_NAMES[i]);
            interact(task);
            ComboBox pageComboBox = lookup("#PageSizeComboBox").query();
            task = new ComboBoxTask(pageComboBox, PAGE_SIZES[i]);
            interact(task);
            clickOn("#CreateButton");
            TextArea feedbackBox = lookup("#FeedbackBox").query();
            assertTrue(feedbackBox.getText().contains(
                    "Database successfully created."));
            assertEquals(state.getDb().getConfig().getType(), TYPES[i]);
            clickOn("#DatabasePageButton");
        }
    }

    /**
     * Tests the Database Create Tab errors.
     *
     * @throws Exception
     */
    @Test
    public void testCreateDbErrors() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, false, true);
        // Test no database file or name.
        clickOn("#DatabaseCreateTab");
        clickOn("#CreateButton");
        TextArea feedbackBox = lookup("#dbFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Please enter a database file or database name."));
        clickOn("#ClearButton");
        // Test heap and queue databases do not support sub databases.
        clickOn("#DatabaseFileEditTextField").write(
                BDBGUITestConfig.DATABASE_NAME1);
        clickOn("#DatabaseNameTextField").write("heap");
        ComboBox typeComboBox = lookup("#DatabaseTypeComboBox").query();
        ComboBoxTask task = new ComboBoxTask(typeComboBox, "heap");
        interact(task);
        clickOn("#CreateButton");
        feedbackBox = lookup("#dbFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Heap and queue databases do not support multiple databases per file"));
        clickOn("#ClearButton");
        clickOn("#DatabaseFileEditTextField").write(
                BDBGUITestConfig.DATABASE_NAME1);
        clickOn("#DatabaseNameTextField").write("queue");
        typeComboBox = lookup("#DatabaseTypeComboBox").query();
        task = new ComboBoxTask(typeComboBox, "queue");
        interact(task);
        clickOn("#CreateButton");
        feedbackBox = lookup("#dbFeedbackBox").query();
        assertFalse(feedbackBox.getText().contains(
                "Database successfully created."));
        File dbFile = new File(BDBGUITestConfig.ENV_DIR
                + File.separator + BDBGUITestConfig.DATABASE_NAME1);
        assertFalse(dbFile.exists());
    }

    /**
     * Tests that the Database Open Tab works with minimal configurations.
     *
     * @throws Exception
     */
    @Test
    public void testOpenDbWithoutConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, false, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setAllowCreate(true);
        for (int i = 0; i < TYPES.length; i++) {
            String subDatabaseName = null;
            if (TYPES[i] != DatabaseType.HEAP && TYPES[i] != DatabaseType.QUEUE)
                subDatabaseName = TYPE_NAMES[i];
            dConfig.setType(TYPES[i]);
            state.openDatabase(TYPE_NAMES[i], subDatabaseName, dConfig);
            state.closeDatabase();
            clickOn("#DatabaseOpenTab");
            clickOn("#DatabaseFileTextField").write(TYPE_NAMES[i]);
            if (TYPES[i] != DatabaseType.HEAP && TYPES[i] != DatabaseType.QUEUE)
                clickOn("#DatabaseNameTextField").write(TYPE_NAMES[i]);
            clickOn("#OpenButton");
            TextArea feedbackBox = lookup("#FeedbackBox").query();
            assertTrue(feedbackBox.getText().contains(
                    "Database successfully opened."));
            File dbFile = new File(BDBGUITestConfig.ENV_DIR
                    + File.separator + TYPE_NAMES[i]);
            assertTrue(dbFile.exists());
            assertEquals(state.getDb().getConfig().getType(), TYPES[i]);
            clickOn("#DatabasePageButton");
        }
    }

    /**
     * Tests the Database Open Tab configurations.
     *
     * @throws Exception
     */
    @Test
    public void testOpenDbWithConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        eConfig.setEncryptionKey(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setEncrypted(BDBGUITestConfig.DB_ENCRYPTION_PASSWORD);
        dConfig.setAllowCreate(true);
        for (int i = 0; i < TYPES.length; i++) {
            String subDatabaseName = null;
            if (TYPES[i] != DatabaseType.HEAP && TYPES[i] != DatabaseType.QUEUE)
                subDatabaseName = TYPE_NAMES[i];
            dConfig.setType(TYPES[i]);
            state.openDatabase(TYPE_NAMES[i], subDatabaseName, dConfig);
            state.closeDatabase();
            clickOn("#DatabaseOpenTab");
            clickOn("#DatabaseFileTextField").write(TYPE_NAMES[i]);
            if (TYPES[i] != DatabaseType.HEAP && TYPES[i] != DatabaseType.QUEUE)
                clickOn("#DatabaseNameTextField").write(TYPE_NAMES[i]);
            clickOn("#EncryptionPasswordField").write(
                    BDBGUITestConfig.DB_ENCRYPTION_PASSWORD);
            clickOn("#OpenButton");
            TextArea feedbackBox = lookup("#FeedbackBox").query();
            assertTrue(feedbackBox.getText().contains(
                    "Database successfully opened."));
            File dbFile = new File(BDBGUITestConfig.ENV_DIR
                    + File.separator + TYPE_NAMES[i]);
            assertTrue(dbFile.exists());
            assertEquals(state.getDb().getConfig().getType(), TYPES[i]);
            clickOn("#DatabasePageButton");
        }
    }

    /**
     * Tests the Database Open Tab errors.
     *
     * @throws Exception
     */
    @Test
    public void testOpenDbErrors() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        eConfig.setEncryptionKey(BDBGUITestConfig.ENCRYPTION_PASSWORD);
        state.openEnvironment(eConfig, true, true);
        // Do not enter a database file or name
        clickOn("#DatabaseOpenTab");
        clickOn("#OpenButton");
        TextArea feedbackBox = lookup("#dbFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Please enter a database file or database name."));
        clickOn("#ClearButton");
        // Open when database does not exist.
        clickOn("#DatabaseFileTextField").write(
                BDBGUITestConfig.DATABASE_NAME1);
        clickOn("#OpenButton");
        feedbackBox = lookup("#dbFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Database file is not a file or does not exist"));
        assertNull(state.getDb());
    }

    /**
     * Tests that the Database Compact Tab works with minimal configurations.
     *
     * @throws Exception
     */
    @Test
    public void testCompactDbWithoutConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setAllowCreate(true);
        for (int i = 0; i < TYPES.length; i++) {
            dConfig.setType(TYPES[i]);
            state.openDatabase(TYPE_NAMES[i], null, dConfig);
            state.closeDatabase();
            clickOn("#DatabaseCompactTab");
            clickOn("#DatabaseFileTextField").write(TYPE_NAMES[i]);
            clickOn("#CompactButton");
            /*
             * The compact process takes place as a background thread, so sleep
             * to wait for the results.
             */
            Thread.sleep(1000);
            TextArea feedbackBox = lookup("#dbFeedbackBox").query();
            if (TYPES[i] != DatabaseType.HEAP
                    && TYPES[i] != DatabaseType.QUEUE) {
                File dbFile = new File(BDBGUITestConfig.ENV_DIR
                        + File.separator + TYPE_NAMES[i]);
                assertTrue(feedbackBox.getText().contains(
                        "Success compacting database: "
                                + dbFile.getAbsolutePath()));
            } else {
                assertTrue(feedbackBox.getText().contains(
                        "Error, cannot compact Heap and Queue databases."));
            }
            clickOn("#ClearButton");
        }
    }

    /**
     * Tests the Database Compact Tab configurations.
     *
     * @throws Exception
     */
    @Ignore
    public void testCompactDbWithConfigs() throws Exception {
        // No non-required configs to test.
    }

    /**
     * Tests the Database Compact Tab errors.
     *
     * @throws Exception
     */
    @Test
    public void testCompactDbErrors() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        // Do not enter a database file
        clickOn("#DatabaseCompactTab");
        clickOn("#CompactButton");
        TextArea feedbackBox = lookup("#dbFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Please enter a database file."));
        clickOn("#ClearButton");
    }

    /**
     * Tests that the Database Remove Tab works with minimal configurations.
     *
     * @throws Exception
     */
    @Test
    public void testRemoveDbWithoutConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        dConfig.setAllowCreate(true);
        for (int i = 0; i < TYPES.length; i++) {
            String subDatabaseName = null;
            String name = TYPE_NAMES[i];
            if (TYPES[i] != DatabaseType.HEAP
                    && TYPES[i] != DatabaseType.QUEUE) {
                subDatabaseName = TYPE_NAMES[i];
                name += ":" + TYPE_NAMES[i];
            }
            File file = new File(
                    BDBGUITestConfig.ENV_DIR + File.separator + name);
            dConfig.setType(TYPES[i]);
            state.openDatabase(TYPE_NAMES[i], subDatabaseName, dConfig);
            state.closeDatabase();
            clickOn("#DatabaseRemoveTab");
            clickOn("#DatabaseFileTextField").write(TYPE_NAMES[i]);
            if (TYPES[i] != DatabaseType.HEAP && TYPES[i] != DatabaseType.QUEUE)
                clickOn("#DatabaseNameTextField").write(TYPE_NAMES[i]);
            clickOn("#RemoveButton");
            TextArea feedbackBox = lookup("#dbFeedbackBox").query();
            assertTrue(feedbackBox.getText().contains(
                    file.getAbsolutePath() + " successfully removed."));
            File dbFile = new File(BDBGUITestConfig.ENV_DIR
                    + File.separator + TYPE_NAMES[i]);
            if (TYPES[i] != DatabaseType.HEAP && TYPES[i] != DatabaseType.QUEUE)
                assertTrue(dbFile.exists());
            else
                assertFalse(dbFile.exists());
            assertNull(state.getDb());
            clickOn("#ClearButton");
        }
    }

    /**
     * Tests the Database Remove Tab configurations.
     *
     * @throws Exception
     */
    @Ignore
    public void testRemoveDbWithConfigs() throws Exception {
        // No non-required configurations to test
    }

    /**
     * Tests the Database Remove Tab errors.
     *
     * @throws Exception
     */
    @Test
    public void testRemoveDbErrors() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        // Do not enter a database file
        clickOn("#DatabaseRemoveTab");
        clickOn("#RemoveButton");
        TextArea feedbackBox = lookup("#dbFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Please enter a database file or database name."));
        clickOn("#ClearButton");
    }

    /**
     * Tests the Database Rename Tab works with minimal configurations.
     *
     * @throws Exception
     */
    @Test
    public void testRenameDbWithoutConfigs() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        DatabaseConfig dConfig = new DatabaseConfig();
        for (int i = 0; i < TYPES.length; i++) {
            dConfig.setAllowCreate(true);
            String subDatabaseName = null;
            String name = TYPE_NAMES[i];
            if (TYPES[i] != DatabaseType.HEAP
                    && TYPES[i] != DatabaseType.QUEUE) {
                subDatabaseName = TYPE_NAMES[i];
                name += ":" + TYPE_NAMES[i];
            }
            File file = new File(
                    BDBGUITestConfig.ENV_DIR + File.separator + name);
            dConfig.setType(TYPES[i]);
            state.openDatabase(TYPE_NAMES[i], subDatabaseName, dConfig);
            state.closeDatabase();
            clickOn("#DatabaseRenameTab");
            clickOn("#DatabaseFileTextField").write(TYPE_NAMES[i]);
            if (TYPES[i] != DatabaseType.HEAP && TYPES[i] != DatabaseType.QUEUE)
                clickOn("#DatabaseNameTextField").write(TYPE_NAMES[i]);
            clickOn("#DatabaseNewTextField").write(TYPE_NAMES[i] + "2");
            clickOn("#RenameButton");
            TextArea feedbackBox = lookup("#dbFeedbackBox").query();
            assertTrue(feedbackBox.getText().contains(
                    file.getAbsolutePath() + " successfully renamed to "
                            + TYPE_NAMES[i] + "2"));
            File dbFile = new File(BDBGUITestConfig.ENV_DIR
                    + File.separator + TYPE_NAMES[i]);
            if (TYPES[i] != DatabaseType.HEAP && TYPES[i] != DatabaseType.QUEUE)
                assertTrue(dbFile.exists());
            else
                assertFalse(dbFile.exists());
            dConfig.setAllowCreate(false);
            dConfig.setType(DatabaseType.UNKNOWN);
            if (TYPES[i] != DatabaseType.HEAP && TYPES[i] != DatabaseType.QUEUE)
                db = state.getEnv().openDatabase(null,
                        TYPE_NAMES[i], subDatabaseName + "2", dConfig);
            else
                db = state.getEnv().openDatabase(null,
                        TYPE_NAMES[i] + "2", subDatabaseName, dConfig);
            assertNotNull(db);
            db.close();
            db = null;
            clickOn("#ClearButton");
        }
    }

    /**
     * Tests the Database Rename Tab configurations.
     *
     * @throws Exception
     */
    @Ignore
    public void testRenameDbWithConfigs() throws Exception {
        // No non-required configurations to test
    }

    /**
     * Tests the Database Rename Tab errors.
     *
     * @throws Exception
     */
    @Test
    public void testRenameDbErrors() throws Exception {
        File home = new File(BDBGUITestConfig.ENV_DIR);
        EnvConfig eConfig = new EnvConfig();
        eConfig.setHome(home);
        state.openEnvironment(eConfig, true, true);
        // Do not enter a database file
        clickOn("#DatabaseRenameTab");
        clickOn("#RenameButton");
        TextArea feedbackBox = lookup("#dbFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Please enter a database file or database name."));
        clickOn("#ClearButton");
        // Rename a database that does not exist
        clickOn("#DatabaseFileTextField").write(
                BDBGUITestConfig.DATABASE_NAME1);
        clickOn("#DatabaseNewTextField").write(BDBGUITestConfig.DATABASE_NAME2);
        clickOn("#RenameButton");
        feedbackBox = lookup("#dbFeedbackBox").query();
        assertTrue(feedbackBox.getText().contains(
                "Database file is not a file or does not exist"));
    }
}
