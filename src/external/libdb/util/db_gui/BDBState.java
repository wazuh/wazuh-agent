/*-
 * Copyright (c) 2001, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */

package db_gui;

import com.sleepycat.db.*;
import db_gui.envpage.EnvConfig;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.logging.Level;
import java.util.logging.Logger;

import javafx.application.Platform;
import javafx.scene.control.Button;
import javafx.scene.control.Tab;
import javafx.scene.control.TextArea;
import javafx.stage.Stage;

/**
 * BDBState is a singleton that contains current BDB environment, database,
 * cursor, and transaction.  It also contains the current feedback box for
 * printing feedback to the user, and the messageQueue that buffers the messages
 * to be printed to the feedback box.
 */
public class BDBState implements ErrorHandler, FeedbackHandler, MessageHandler {
    private static final BDBState instance = new BDBState();
    private Environment env;
    private Database db;
    private Cursor cursor;
    private Transaction txn;
    private Stage mainStage;
    private TextArea feedbackBox;
    private Button disableButton, enableButton;
    private boolean feedbackBoxEnabled;
    private boolean transactional;
    private final List<Tab> tabs;
    private final List<Button> buttons;
    final private BlockingQueue<String> messageQueue;
    final private MessageBuffer messageBuffer;

    /**
     * Default constructor.
     */
    private BDBState() {
        env = null;
        db = null;
        cursor = null;
        txn = null;
        mainStage = null;
        feedbackBox = null;
        enableButton = disableButton = null;
        transactional = false;
        feedbackBoxEnabled = true;
        tabs = new LinkedList<>();
        buttons = new LinkedList<>();
        messageQueue = new LinkedBlockingQueue<>();
        messageBuffer = new MessageBuffer(messageQueue);
        Platform.runLater(new Runnable() {
	    @Override
	    public void run() {
		messageBuffer.start();
	    }
	});
    }

    /**
     * Gets the only instance of BDBState in the application.
     *
     * @return The Singleton instance of BDBState.
     */
    public static BDBState getBDBState() {
        return instance;
    }

    /**
     * Gets the stage used to display the current page of the GUI.
     *
     * @return The JavaFX main state.
     */
    public Stage getMainStage() {
        return mainStage;
    }

    /**
     * Gets the current environment.
     *
     * @return The current environment, or null if no environment is currently
     * open.
     */
    public Environment getEnv() {
        return env;
    }

    /**
     * Gets the current database.
     *
     * @return The current database, or null if no database is currently open.
     */
    public Database getDb() {
        return db;
    }

    /**
     * Gets the current cursor.
     *
     * @return Gets the current cursor, or null if no cursor is currently open.
     */
    public Cursor getCursor() {
        return cursor;
    }

    /**
     * Opens a cursor on the current database if one does not already exist.
     *
     * @return The current cursor.
     * @throws DatabaseException
     */
    public Cursor openCursor() throws DatabaseException {
        if (cursor != null)
            return cursor;
        cursor = db.openCursor(txn, CursorConfig.DEFAULT);
        return cursor;
    }

    /**
     * Gets the current transaction.
     *
     * @return The current transaction, or null if none exists.
     */
    public Transaction getTxn() {
        return txn;
    }

    /**
     * Begins a transaction.  If there is no current transaction but the
     * environment is transactional, it creates a new transaction and returns
     * it.
     *
     * @return The current transaction, or null if the environment is not
     * transactional.
     * @throws DatabaseException
     */
    public Transaction beginTxn() throws DatabaseException {
        if (txn != null || !transactional)
            return txn;
        closeCursor();
        txn = env.beginTransaction(null, TransactionConfig.DEFAULT);
        return txn;
    }

    /**
     * Gets whether the environment is transactional or not.
     *
     * @return True if the environment is transactional, otherwise false.
     */
    public boolean isTransactional() {
        return transactional;
    }

    /**
     * Sets the feedbackBox, to which output is printed.
     *
     * @param feedbackBox - The feedback box of the current GUI page.
     */
    public void setFeedbackBox(TextArea feedbackBox) {
        this.feedbackBox = feedbackBox;
    }
    
    /**
     * Sets the button to disable messages to the feedbackBox
     *
     * @param button - The disable button for the current page.
     */
    public void setDisableButton(Button button) {
        this.disableButton = button;
    }
    
    /**
     * Sets the button to enable messages to the feedbackBox
     *
     * @param button - The enable button for the current page.
     */
    public void setEnableButton(Button button) {
        this.enableButton = button;
    }
    
    /**
     * Sets the enabled state of the FeedbackBox
     *
     * @param enable - Whether to enable messages to be
     *   sent to the feedback box or not.
     */
    public void enableFeedbackBox(boolean enable) {
        this.feedbackBoxEnabled = enable;
        if (enableButton != null && disableButton != null) {
             enableButton.setDisable(enable);
             disableButton.setDisable(!enable);
        }
    }

    /**
     * Sets the main stage of the GUI.
     *
     * @param mainStage The mains stage as defined by the application.
     */
    public void setMainStage(Stage mainStage) {
        this.mainStage = mainStage;
    }

    /**
     * Used by the test suite to set an Environment.
     *
     * @param env - Environment to be set.
     * @throws DatabaseException
     */
    public void setEnvironment(Environment env) throws DatabaseException {
        EnvironmentConfig config = env.getConfig();
        if (this.env != null)
            closeEnvironment();
        transactional = config.getTransactional();
        this.env = env;
    }

    /**
     * Used by the test suite to set a Database.
     *
     * @param db - The Database to be set.
     */
    public void setDatabase(Database db) {
        if (this.db != null)
            closeDatabase();
        this.db = db;
    }

    /**
     * Prints output to the user.
     *
     * @param message - The message to be printed.
     */
    public void printFeedback(String message) {
        if (feedbackBoxEnabled)
            messageQueue.add(message + "\n");
    }

    /**
     * Appends text to the feedback box.  This function should only be used by
     * the MessageBuffer.
     *
     * @param message The message to be printed.
     */
    public void appendText(String message) {
        if (feedbackBox != null) {
            feedbackBox.appendText(message);
        } else
            System.err.println(message);
    }

    /**
     * Aborts the current transaction if it exists.  Also closes the current
     * cursor.
     */
    public void abortTransaction() {
        if (!transactional || txn == null)
            return;
        try {
            closeCursor();
            txn.abort();
            txn = null;
        } catch (DatabaseException ex) {
            Logger.getLogger(
                    BDBState.class.getName()).log(Level.SEVERE, null, ex);
            printFeedback("Error aborting transaction: " + ex.getMessage());
            feedbackBox.appendText("Fatal error, please restart application.");
        } finally {
            txn = null;
        }
    }

    /**
     * Add a Tab to the tabs list.  Used to hold the tabs of the current page
     * so they can be enabled and disabled when a background task is automated.
     *
     * @param tab - A tab to add to the tab list.
     */
    public void addTab(Tab tab) {
        tabs.add(tab);
    }

    public void addButton(Button button) {
        buttons.add(button);
    }

    /**
     * Clear the tabs list.
     */
    public void clearTabs() {
        tabs.clear();
    }

    /**
     * Clear the buttons list.
     */
    public void clearButtons() {
        buttons.clear();
        enableButton = null;
        disableButton = null;
    }

    /**
     * Enable the tabs of the current page so they can be accessed again.
     */
    public void enable() {
        Tab[] tabArray = new Tab[0];
        tabArray = tabs.toArray(tabArray);
        for (Tab tab : tabArray) {
            if (tab != null)
                tab.setDisable(false);
        }
        Button[] buttonArray = new Button[0];
        buttonArray = buttons.toArray(buttonArray);
        for (Button button : buttonArray) {
            if (button != null)
                button.setDisable(false);
        }
    }

    /**
     * Disable the tabs of the current page so the user cannot switch tabs.
     * This is done to protect the state of BDBState while a background Task
     * is operating on the environment.
     */
    public void disable() {
        Tab[] tabArray = new Tab[0];
        tabArray = tabs.toArray(tabArray);
        for (Tab tab : tabArray) {
            if (tab != null)
                tab.setDisable(true);
        }
        Button[] buttonArray = new Button[0];
        buttonArray = buttons.toArray(buttonArray);
        for (Button button : buttonArray) {
            if (button != null)
                button.setDisable(true);
        }
    }

    /**
     * Commits the current transaction if it exists, also closes the current
     * cursor.
     */
    public void commitTransaction() {
        if (!transactional || txn == null)
            return;
        try {
            closeCursor();
            txn.commit();
            txn = null;
        } catch (DatabaseException ex) {
            Logger.getLogger(
                    BDBState.class.getName()).log(Level.SEVERE, null, ex);
            printFeedback("Error committing transaction: " + ex.getMessage());
            feedbackBox.appendText("Fatal error, please restart application.");
        } finally {
            txn = null;
        }
    }

    /**
     * Opens a database in the current environment.
     *
     * @param file The database file.
     * @param name The database name.
     * @param config The database configuration.
     * @return True if the operation was successful, otherwise false.
     */
    public boolean openDatabase(
            String file, String name, DatabaseConfig config) {
        if (db != null)
            closeDatabase();
        config.setErrorHandler(this);
        config.setMessageHandler(this);
        config.setTransactional(isTransactional());
        try {
            db = env.openDatabase(null, file, name, config);
        } catch (DatabaseException | FileNotFoundException ex) {
            Logger.getLogger(
                    BDBState.class.getName()).log(Level.SEVERE, null, ex);
            String fullName = "";
            if (file != null)
                fullName = file + " ";
            if (name != null)
                fullName = fullName + name + " ";
            printFeedback(
                    "Error opening database: " + fullName + ex.getMessage());
            return false;
        }
        return true;
    }

    /**
     * Opens an environment.
     *
     * @param envConfig - Contains various environment configurations.
     * @param txn - Whether the environment is transactional or not.
     * @param create - Whether to create the environment if it does not exist
     * or not.
     * @return True if the operation was successful, otherwise false.
     */
    public boolean openEnvironment(
            EnvConfig envConfig, boolean txn, boolean create) {
        EnvironmentConfig config = new EnvironmentConfig();
        config.setTransactional(txn);
        if (txn) {
            config.setInitializeLocking(true);
            config.setInitializeLogging(true);
        }
        config.setAllowCreate(create);
        return openEnvironment(envConfig, config);
    }

    /**
     * Opens the environment with recovery.
     *
     * @param envConfig - Contains various environment configurations.
     * @param catastrophic - Whether normal or catastrophic recovery should be
     * performed.
     * @return True if the operation was successful, otherwise false.
     */
    public boolean recoverEnvironment(
            EnvConfig envConfig, boolean catastrophic) {
        EnvironmentConfig config = new EnvironmentConfig();
        config.setTransactional(true);
        config.setInitializeLocking(true);
        config.setInitializeLogging(true);
        config.setAllowCreate(true);
        config.setRunFatalRecovery(catastrophic);
        config.setVerbose(VerboseConfig.RECOVERY, true);
        return openEnvironment(envConfig, config);
    }

    /**
     * Opens the environment.
     *
     * @param envConfig - Contains various environment configurations.
     * @param config - An EnvironmentConfig object.
     * @return True if the operation was successful, otherwise false.
     */
    public boolean openEnvironment(
            EnvConfig envConfig, EnvironmentConfig config) {
        if (env != null)
            closeEnvironment();

        if (envConfig.getEncryptionKey() != null)
            config.setEncrypted(envConfig.getEncryptionKey());
        if (envConfig.getCacheSize() != 0)
            config.setCacheSize(envConfig.getCacheSize());
        else
            config.setCacheSize(10 * 1024 * 1024);

        if (envConfig.getExternalDir() != null)
            config.setExternalFileDir(envConfig.getExternalDir());
        if (envConfig.getLogDir() != null)
            config.setLogDirectory(envConfig.getLogDir());
        File[] dataDirs = envConfig.getDataDirs();
        if (dataDirs != null) {
            for (File dataDir : dataDirs) {
                config.addDataDir(dataDir);
            }
        }
        config.setErrorHandler(this);
        config.setFeedbackHandler(this);
        config.setMessageHandler(this);
        config.setVerbose(VerboseConfig.BACKUP, true);
        config.setInitializeCache(true);
        if (config.getTransactional() && !config.getRunFatalRecovery()) {
            config.setInitializeLocking(true);
            config.setInitializeLogging(true);
            config.setRegister(true);
            config.setRunRecovery(true);
            // Recovery requires the create flag.
            config.setAllowCreate(true);
        }
        try {
            env = new Environment(envConfig.getHome(), config);
            config = env.getConfig();

            transactional = config.getTransactional();
        } catch (DatabaseException ex) {
            Logger.getLogger(BDBState.class.getName())
                    .log(Level.SEVERE, null, ex);
            feedbackBox.appendText(
                    "Error opening environment: " + ex.getMessage());
            return false;
        } catch (FileNotFoundException ex) {
            Logger.getLogger(BDBState.class.getName())
                    .log(Level.SEVERE, null, ex);
            feedbackBox.appendText(
                    "Error opening environment, environment does not exist: "
                            + envConfig.getHome());
            return false;
        }
        return true;
    }

    /**
     * Closes the current environment, and any open database, transaction, and
     * cursor.
     */
    public void closeEnvironment() {
        try {
            closeDatabase();
            if (env != null)
                env.close();
        } catch (DatabaseException ex) {
            Logger.getLogger(BDBState.class.getName())
                    .log(Level.SEVERE, null, ex);
            feedbackBox.appendText("Fatal error, please restart application.");
        } finally {
            env = null;
            transactional = false;
        }
    }

    /**
     * Closes the current database, and any open transaction or cursor.
     */
    public void closeDatabase() {
        try {
            closeTxn();
            if (db != null)
                db.close();
        } catch (DatabaseException ex) {
            Logger.getLogger(BDBState.class.getName())
                    .log(Level.SEVERE, null, ex);
            feedbackBox.appendText("Fatal error, please restart application.");
            try {
                env.close();
            } catch (Exception exe) {
            }
            env = null;
        } finally {
            db = null;
        }
    }

    /**
     * Commits the current transaction, and closes any open cursor.
     */
    public void closeTxn() {
        try {
            closeCursor();
            if (txn != null)
                txn.commit();
        } catch (DatabaseException ex) {
            Logger.getLogger(BDBState.class.getName())
                    .log(Level.SEVERE, null, ex);
            feedbackBox.appendText("Fatal error, please restart application.");
            try {
                db.close();
                env.close();
            } catch (Exception exe) {
            }
            db = null;
            env = null;
        } finally {
            txn = null;
        }
    }

    /**
     * Closes the current cursor.
     */
    public void closeCursor() {
        try {
            if (cursor != null)
                cursor.close();
        } catch (DatabaseException ex) {
            Logger.getLogger(BDBState.class.getName())
                    .log(Level.SEVERE, null, ex);
            feedbackBox.appendText("Fatal error, please restart application.");
            try {
                txn.abort();
                db.close();
                env.close();
            } catch (Exception exe) {
            }
            txn = null;
            db = null;
            env = null;
        } finally {
            cursor = null;
        }
    }

    /**
     * Feeds internal BDB error messages to the GUI.
     *
     * @param environment
     * @param errpfx
     * @param msg
     */
    @Override
    public void error(Environment environment, String errpfx, String msg) {
        String message;
        if (errpfx != null)
            message = errpfx + ": " + msg + "\n";
        else
            message = msg;
        printFeedback(message);
    }

    /**
     * Feeds internal BDB messages to the GUI.
     *
     * @param environment
     * @param msgpfx
     * @param msg
     */
    @Override
    public void message(Environment environment, String msgpfx, String msg) {
        String message;
        if (msgpfx != null)
            message = msgpfx + ": " + msg + "\n";
        else
            message = msg;
        printFeedback(message);
    }

    /**
     * Feeds internal BDB recovery messages to the GUI.
     *
     * @param environment
     * @param percent
     */
    @Override
    public void recoveryFeedback(Environment environment, int percent) {
        String message = "Recovery process " + percent + "% completed.";
        printFeedback(message);
    }

    /**
     * Feeds internal BDB upgrade messages to the GUI.
     *
     * @param database
     * @param percent
     */
    @Override
    public void upgradeFeedback(Database database, int percent) {
        String name = "";
        String file = "";
        try {
            if (database != null) {
                file = database.getDatabaseFile();
                name = database.getDatabaseName();
            }
        } catch (DatabaseException ex) {
            Logger.getLogger(
                    BDBState.class.getName()).log(Level.SEVERE, null, ex);
        }
        String message = "Upgrade process " + percent +
                "% completed for database " + file + ":" + name + ".";
        printFeedback(message);
    }

    /**
     * Feeds internal BDB verify messages to the GUI.
     *
     * @param database
     * @param percent
     */
    @Override
    public void verifyFeedback(Database database, int percent) {
        String name = "";
        String file = "";
        try {
            if (database != null) {
                file = database.getDatabaseFile();
                name = database.getDatabaseName();
            }
        } catch (DatabaseException ex) {
            Logger.getLogger(
                    BDBState.class.getName()).log(Level.SEVERE, null, ex);
        }
        String message = "Verify process " + percent +
                "% completed for database " + file + ":" + name + ".";
        printFeedback(message);
    }
}
