/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */
package db_guitest;

import com.sleepycat.db.Database;
import com.sleepycat.db.DatabaseType;
import com.sleepycat.db.Environment;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.ComboBox;
import javafx.scene.control.TextArea;
import javafx.scene.control.TextField;
import javafx.stage.Stage;

import static junit.framework.Assert.assertEquals;

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
 * Tests the various Tabs of the Database Page.
 */
public class DataAccessPageTests extends ApplicationTest {
    private Environment env;
    private Database db;
    private BDBState state;
    private static final String[] KEYTYPES = {
            "Integer 32",
            "Float 32",
            "Integer 64",
            "Float 64"
    };
    private static final String[] KEYVALUES = {
            "1",
            "2.5",
            "3",
            "4.5"
    };
    private static final String[] DATATYPES = {
            "ASCII",
            "UTF-8",
            "UTF-16",
            "ASCII"
    };
    private static final String[] DATAVALUES = {
            "hello",
            "goodbye",
            "delete",
            "stuff"
    };
    private static final int DELETEINDEX = 2;
    private static final int[] BTREEORDER = {2, 0, 3, 1};
    private static final int[] HASHORDER = {3, 1, 2, 0};

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
        System.out.println("Begin test Data Access Page Tests!");
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
        System.out.println("Finished test Data Access Page Tests!");
    }

    /**
     * Test the BtreeHash Data Access page, using both a btree and hash
     * database, and a transactional and non-transactional environment.
     *
     * @throws Exception
     */
    @Test
    public void testBtreeHash() throws Exception {
        boolean[] transactionals = {true, false};
        DatabaseType[] types = {DatabaseType.BTREE, DatabaseType.HASH};
        for (boolean transactional : transactionals) {
            for (DatabaseType type : types) {
                DisplayAccessPageTask dapt =
                        new DisplayAccessPageTask(type, transactional);
                interact(dapt);
                clickOn("#NextButton");
                TextArea feedbackBox = lookup("#FeedbackBox").query();
                assertTrue(feedbackBox.getText().contains(
                        "End of the database has been reached,"));
                clickOn("#ClearButton");
                // Check that aborting a transaction gets rid of the data.
                if (transactional) {
                    clickOn("#TxnBeginButton");
                    /*
                     * Selecting a value from a comboBox has to be done in the
                     * JavaFX thread to avoid an exception.
                     */
                    ComboBox comboBox = lookup("#PutKeyComboBox").query();
                    ComboBoxTask task =
                            new ComboBoxTask(comboBox, KEYTYPES[0]);
                    interact(task);
                    clickOn("#PutKeyTextField").write(KEYVALUES[0]);
                    comboBox = lookup("#PutDataComboBox").query();
                    task = new ComboBoxTask(comboBox, DATATYPES[0]);
                    interact(task);
                    clickOn("#PutDataTextField").write(DATAVALUES[0]);
                    clickOn("#PutButton");
                    comboBox = lookup("#GetKeyComboBox").query();
                    task = new ComboBoxTask(comboBox, KEYTYPES[0]);
                    interact(task);
                    clickOn("#GetKeyTextField").write(KEYVALUES[0]);
                    clickOn("#GetButton");
                    TextField textField = lookup("#GetDataTextField").query();
                    assertTrue(textField.getText().contains(DATAVALUES[0]));
                    clickOn("#TxnAbortButton");
                    clickOn("#ClearButton");
                    comboBox = lookup("#GetKeyComboBox").query();
                    task = new ComboBoxTask(comboBox, KEYTYPES[0]);
                    interact(task);
                    clickOn("#GetKeyTextField").write(KEYVALUES[0]);
                    clickOn("#GetButton");
                    textField = lookup("#GetDataTextField").query();
                    assertFalse(textField.getText().contains(DATAVALUES[0]));
                }
                // Add data through the Put button
                if (transactional)
                    clickOn("#TxnBeginButton");
                for (int i = 0; i < KEYTYPES.length; i++) {
                    ComboBox comboBox = lookup("#PutKeyComboBox").query();
                    ComboBoxTask task =
                            new ComboBoxTask(comboBox, KEYTYPES[i]);
                    interact(task);
                    clickOn("#PutKeyTextField").write(KEYVALUES[i]);
                    comboBox = lookup("#PutDataComboBox").query();
                    task = new ComboBoxTask(comboBox, DATATYPES[i]);
                    interact(task);
                    clickOn("#PutDataTextField").write(DATAVALUES[i]);
                    clickOn("#PutButton");
                }
                if (transactional)
                    clickOn("#TxnCommitButton");

                int[] indexes = BTREEORDER;
                if (type == DatabaseType.HASH)
                    indexes = HASHORDER;
                // Test the Next button.
                for (int i : indexes) {
                    ComboBox comboBox = lookup("#GetKeyComboBox").query();
                    ComboBoxTask task =
                            new ComboBoxTask(comboBox, KEYTYPES[i]);
                    interact(task);
                    comboBox = lookup("#GetDataComboBox").query();
                    task = new ComboBoxTask(comboBox, DATATYPES[i]);
                    interact(task);
                    clickOn("#NextButton");
                    TextField textField = lookup("#GetKeyTextField").query();
                    assertTrue(textField.getText().contains(KEYVALUES[i]));
                    textField = lookup("#GetDataTextField").query();
                    assertTrue(textField.getText().contains(DATAVALUES[i]));
                }
                clickOn("#ClearButton");
                // Test the Previous button.
                for (int j = KEYTYPES.length - 1; j >= 0; j--) {
                    int i = indexes[j];
                    ComboBox comboBox = lookup("#GetKeyComboBox").query();
                    ComboBoxTask task =
                            new ComboBoxTask(comboBox, KEYTYPES[i]);
                    interact(task);
                    comboBox = lookup("#GetDataComboBox").query();
                    task = new ComboBoxTask(comboBox, DATATYPES[i]);
                    interact(task);
                    clickOn("#PreviousButton");
                    TextField textField = lookup("#GetKeyTextField").query();
                    assertTrue(textField.getText().contains(KEYVALUES[i]));
                    textField = lookup("#GetDataTextField").query();
                    assertTrue(textField.getText().contains(DATAVALUES[i]));
                }
                clickOn("#ClearButton");
                // Test the delete and get buttons
                ComboBox comboBox = lookup("#GetKeyComboBox").query();
                ComboBoxTask task
                        = new ComboBoxTask(comboBox, KEYTYPES[DELETEINDEX]);
                interact(task);
                comboBox = lookup("#GetDataComboBox").query();
                task = new ComboBoxTask(comboBox, DATATYPES[DELETEINDEX]);
                interact(task);
                clickOn("#GetKeyTextField").write(KEYVALUES[DELETEINDEX]);
                clickOn("#GetButton");
                TextField textField = lookup("#GetDataTextField").query();
                assertTrue(
                        textField.getText().contains(DATAVALUES[DELETEINDEX]));
                clickOn("#DeleteButton");
                clickOn("#GetKeyTextField").write(KEYVALUES[DELETEINDEX]);
                clickOn("#GetButton");
                textField = lookup("#GetDataTextField").query();
                assertFalse(
                        textField.getText().contains(DATAVALUES[DELETEINDEX]));

                // Cleanup
                state.closeDatabase();
                state.closeEnvironment();
                BDBGUITestConfig.cleanEnvironment();
            }
        }
    }

    /**
     * Test the Recno Data Access page with both a transactional and
     * non-transactional environment.
     *
     * @throws Exception
     */
    @Test
    public void testRecno() throws Exception {
        boolean[] transactionals = {true, false};
        for (boolean transactional : transactionals) {
            DisplayAccessPageTask dapt = new DisplayAccessPageTask(
                    DatabaseType.RECNO, transactional);
            interact(dapt);
            clickOn("#NextButton");
            TextArea feedbackBox = lookup("#FeedbackBox").query();
            assertTrue(feedbackBox.getText().contains(
                    "End of the database has been reached,"));
            clickOn("#ClearButton");
            // Check that aborting a transaction gets rid of the data.
            if (transactional) {
                clickOn("#TxnBeginButton");
                /*
                 * Selecting a value from a comboBox has to be done in the
                 * JavaFX thread to avoid an exception.
                 */
                clickOn("#PutKeyTextField").write("1");
                ComboBox comboBox = lookup("#PutDataComboBox").query();
                ComboBoxTask task = new ComboBoxTask(comboBox, DATATYPES[0]);
                interact(task);
                clickOn("#PutDataTextField").write(DATAVALUES[0]);
                clickOn("#PutButton");
                clickOn("#GetKeyTextField").write("1");
                clickOn("#GetButton");
                TextField textField = lookup("#GetDataTextField").query();
                assertTrue(textField.getText().contains(DATAVALUES[0]));
                clickOn("#TxnAbortButton");
                clickOn("#ClearButton");
                clickOn("#GetKeyTextField").write("1");
                clickOn("#GetButton");
                textField = lookup("#GetDataTextField").query();
                assertFalse(textField.getText().contains(DATAVALUES[0]));
            }
            // Add data through the Put button
            if (transactional) {
                clickOn("#TxnBeginButton");
            }
            for (int i = 0; i < KEYTYPES.length; i++) {
                clickOn("#PutKeyTextField").write(String.valueOf(i + 1));
                ComboBox comboBox = lookup("#PutDataComboBox").query();
                ComboBoxTask task = new ComboBoxTask(comboBox, DATATYPES[i]);
                interact(task);
                clickOn("#PutDataTextField").write(DATAVALUES[i]);
                clickOn("#PutButton");
            }
            if (transactional) {
                clickOn("#TxnCommitButton");
            }
            // Test the Next button.
            for (int i = 0; i < DATAVALUES.length; i++) {
                ComboBox comboBox = lookup("#GetDataComboBox").query();
                ComboBoxTask task = new ComboBoxTask(comboBox, DATATYPES[i]);
                interact(task);
                clickOn("#NextButton");
                TextField textField = lookup("#GetKeyTextField").query();
                assertTrue(textField.getText().contains(String.valueOf(i + 1)));
                textField = lookup("#GetDataTextField").query();
                assertTrue(textField.getText().contains(DATAVALUES[i]));
            }
            clickOn("#ClearButton");
            // Test the Previous button.
            for (int i = KEYTYPES.length - 1; i >= 0; i--) {
                ComboBox comboBox = lookup("#GetDataComboBox").query();
                ComboBoxTask task = new ComboBoxTask(comboBox, DATATYPES[i]);
                interact(task);
                clickOn("#PreviousButton");
                TextField textField = lookup("#GetKeyTextField").query();
                assertTrue(textField.getText().contains(String.valueOf(i + 1)));
                textField = lookup("#GetDataTextField").query();
                assertTrue(textField.getText().contains(DATAVALUES[i]));
            }
            clickOn("#ClearButton");
            // Test the delete and get buttons
            ComboBox comboBox = lookup("#GetDataComboBox").query();
            ComboBoxTask task =
                    new ComboBoxTask(comboBox, DATATYPES[DELETEINDEX]);
            interact(task);
            clickOn("#GetKeyTextField").write(String.valueOf(DELETEINDEX + 1));
            clickOn("#GetButton");
            TextField textField = lookup("#GetDataTextField").query();
            assertTrue(
                    textField.getText().contains(DATAVALUES[DELETEINDEX]));
            clickOn("#DeleteButton");
            clickOn("#GetKeyTextField").write(String.valueOf(DELETEINDEX + 1));
            clickOn("#GetButton");
            textField = lookup("#GetDataTextField").query();
            assertFalse(
                    textField.getText().contains(DATAVALUES[DELETEINDEX]));

            // Cleanup
            state.closeDatabase();
            state.closeEnvironment();
            BDBGUITestConfig.cleanEnvironment();
        }
    }

    /**
     * Test the Heap Data Access page with both a transactional and
     * non-transactional environment.
     *
     * @throws Exception
     */
    @Test
    public void testHeap() throws Exception {
        boolean[] transactionals = {true, false};
        for (boolean transactional : transactionals) {
            DisplayAccessPageTask dapt = new DisplayAccessPageTask(
                    DatabaseType.HEAP, transactional);
            interact(dapt);
            clickOn("#NextButton");
            TextArea feedbackBox = lookup("#FeedbackBox").query();
            assertTrue(feedbackBox.getText().contains(
                    "End of the database has been reached,"));
            clickOn("#ClearButton");
            // Check that aborting a transaction gets rid of the data.
            if (transactional) {
                clickOn("#TxnBeginButton");
                /*
                 * Selecting a value from a comboBox has to be done in the
                 * JavaFX thread to avoid an exception.
                 */
                ComboBox comboBox = lookup("#PutDataComboBox").query();
                ComboBoxTask task = new ComboBoxTask(comboBox, DATATYPES[0]);
                interact(task);
                clickOn("#PutDataTextField").write(DATAVALUES[0]);
                clickOn("#PutButton");
                clickOn("#NextButton");
                TextField textField = lookup("#GetDataTextField").query();
                assertTrue(textField.getText().contains(DATAVALUES[0]));
                clickOn("#TxnAbortButton");
                clickOn("#ClearButton");
                clickOn("#NextButton");
                textField = lookup("#GetDataTextField").query();
                assertFalse(textField.getText().contains(DATAVALUES[0]));
            }
            // Add data through the Put button
            if (transactional) {
                clickOn("#TxnBeginButton");
            }
            for (int i = 0; i < KEYTYPES.length; i++) {
                ComboBox comboBox = lookup("#PutDataComboBox").query();
                ComboBoxTask task = new ComboBoxTask(comboBox, DATATYPES[i]);
                interact(task);
                clickOn("#PutDataTextField").write(DATAVALUES[i]);
                clickOn("#PutButton");
            }
            if (transactional) {
                clickOn("#TxnCommitButton");
            }
            // Test the Next button.
            for (int i = 0; i < DATAVALUES.length; i++) {
                ComboBox comboBox = lookup("#GetDataComboBox").query();
                ComboBoxTask task = new ComboBoxTask(comboBox, DATATYPES[i]);
                interact(task);
                clickOn("#NextButton");
                TextField textField = lookup("#GetDataTextField").query();
                assertTrue(textField.getText().contains(DATAVALUES[i]));
            }
            clickOn("#ClearButton");
            // Test the Previous button.
            for (int i = KEYTYPES.length - 1; i >= 0; i--) {
                ComboBox comboBox = lookup("#GetDataComboBox").query();
                ComboBoxTask task = new ComboBoxTask(comboBox, DATATYPES[i]);
                interact(task);
                clickOn("#PreviousButton");
                TextField textField = lookup("#GetDataTextField").query();
                assertTrue(textField.getText().contains(DATAVALUES[i]));
            }
            clickOn("#ClearButton");
            // Test the delete botton
            clickOn("#NextButton");
            TextField textField = lookup("#GetDataTextField").query();
            assertTrue(
                    textField.getText().contains(DATAVALUES[0]));
            clickOn("#DeleteButton");
            clickOn("#NextButton");
            textField = lookup("#GetDataTextField").query();
            assertFalse(
                    textField.getText().contains(DATAVALUES[0]));

            // Cleanup
            state.closeDatabase();
            state.closeEnvironment();
            BDBGUITestConfig.cleanEnvironment();
        }
    }

    /**
     * Test the Queue Data Access Page with both a transactional and
     * non-transactional environment.
     *
     * @throws Exception
     */
    @Test
    public void testQueue() throws Exception {
        boolean[] transactionals = {true, false};
        for (boolean transactional : transactionals) {
            DisplayAccessPageTask dapt = new DisplayAccessPageTask(
                    DatabaseType.QUEUE, transactional);
            interact(dapt);
            clickOn("#NextButton");
            TextArea feedbackBox = lookup("#FeedbackBox").query();
            assertTrue(feedbackBox.getText().contains(
                    "End of the database has been reached,"));
            clickOn("#ClearButton");
            // Add data through the Enqueue button
            if (transactional) {
                clickOn("#TxnBeginButton");
            }
            for (int i = 0; i < KEYTYPES.length; i++) {
                ComboBox comboBox = lookup("#EnqueueComboBox").query();
                ComboBoxTask task = new ComboBoxTask(comboBox, DATATYPES[i]);
                interact(task);
                clickOn("#EnqueueTextField").write(DATAVALUES[i]);
                clickOn("#EnqueueButton");
            }
            if (transactional) {
                clickOn("#TxnCommitButton");
            }
            // Test the Next button.
            for (int i = 0; i < DATAVALUES.length; i++) {
                ComboBox comboBox = lookup("#GetDataComboBox").query();
                ComboBoxTask task = new ComboBoxTask(comboBox, DATATYPES[i]);
                interact(task);
                clickOn("#NextButton");
                TextField textField = lookup("#GetKeyTextField").query();
                assertTrue(textField.getText().contains(String.valueOf(i + 1)));
                textField = lookup("#GetDataTextField").query();
                assertTrue(textField.getText().contains(DATAVALUES[i]));
            }
            clickOn("#ClearButton");
            // Test the Previous button.
            for (int i = KEYTYPES.length - 1; i >= 0; i--) {
                ComboBox comboBox = lookup("#GetDataComboBox").query();
                ComboBoxTask task = new ComboBoxTask(comboBox, DATATYPES[i]);
                interact(task);
                clickOn("#PreviousButton");
                TextField textField = lookup("#GetKeyTextField").query();
                assertTrue(textField.getText().contains(String.valueOf(i + 1)));
                textField = lookup("#GetDataTextField").query();
                assertTrue(textField.getText().contains(DATAVALUES[i]));
            }
            clickOn("#ClearButton");
            // Test the delete and get buttons
            ComboBox comboBox = lookup("#GetDataComboBox").query();
            ComboBoxTask task =
                    new ComboBoxTask(comboBox,
                            DATATYPES[DATAVALUES.length - 1]);
            interact(task);
            clickOn("#GetKeyTextField").write(
                    String.valueOf(DATAVALUES.length));
            clickOn("#GetButton");
            TextField textField = lookup("#GetDataTextField").query();
            assertTrue(textField.getText().contains(
                    DATAVALUES[DATAVALUES.length - 1]));
            clickOn("#DeleteButton");
            clickOn("#GetKeyTextField").write(
                    String.valueOf(DATAVALUES.length));
            clickOn("#GetButton");
            textField = lookup("#GetDataTextField").query();
            assertFalse(textField.getText().contains(
                    DATAVALUES[DATAVALUES.length - 1]));

            // Test the Dequeue button
            for (int i = 0; i < DATAVALUES.length - 1; i++) {
                comboBox = lookup("#DequeueComboBox").query();
                task = new ComboBoxTask(comboBox, DATATYPES[i]);
                interact(task);
                clickOn("#DequeueButton");
                textField = lookup("#DequeueTextField").query();
                assertTrue(textField.getText().contains(DATAVALUES[i]));
            }
            clickOn("#ClearButton");
            clickOn("#NextButton");
            assertEquals(textField.getText().length(), 0);
            clickOn("#ClearButton");
            // Check that aborting a transaction gets rid of the data.
            if (transactional) {
                clickOn("#TxnBeginButton");
                comboBox = lookup("#EnqueueComboBox").query();
                task = new ComboBoxTask(comboBox, DATATYPES[0]);
                interact(task);
                clickOn("#EnqueueTextField").write(DATAVALUES[0]);
                clickOn("#EnqueueButton");
                clickOn("#NextButton");
                textField = lookup("#GetDataTextField").query();
                assertTrue(textField.getText().contains(DATAVALUES[0]));
                clickOn("#TxnAbortButton");
                clickOn("#ClearButton");
                clickOn("#NextButton");
                textField = lookup("#GetDataTextField").query();
                assertEquals(textField.getText().length(), 0);
            }
            // Cleanup
            state.closeDatabase();
            state.closeEnvironment();
            BDBGUITestConfig.cleanEnvironment();
        }
    }
}
