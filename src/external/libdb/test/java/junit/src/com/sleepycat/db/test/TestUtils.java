/*-
 * Copyright (c) 2002, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 */

/*
 * Generally useful functions :)
 */

package com.sleepycat.db.test;

import static org.junit.Assert.fail;

import com.sleepycat.db.*;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Properties;

public class TestUtils
{
    public static boolean config_loaded = false;
    public static boolean verbose_flag = false;
    public static int debug_level = 2;
 
    // should be initialized by calling loadEnvVars. Shared between all tests.
    public static String BASETEST_DBDIR   = "";
    public static File   BASETEST_DBFILE  = null; //      new File(TestUtils.BASETEST_DBDIR);
    public static String BASETEST_BACKUPDIR   = "";
    public static File   BASETEST_BACKUPFILE  = null; //      new File(TestUtils.BASETEST_BACKUPDIR);

    public static Boolean repmgrSSLDisabled = true;
    public static String repmgrCACert = null;
    public static String repmgrNodeCert = null;
    public static String repmgrNodePkey = null;
    public static String repmgrNodePkeyPassword = null;
    public static String repmgrCADir = null;
    public static int repmgrVerifyDepth = 0;
 
    public static void ERR(String a)
    {
        System.err.println("FAIL: " + a);
        fail(a);
    }

    public static void DEBUGOUT(String s)
    {
        DEBUGOUT(1, s);
    }

    public static void DEBUGOUT(int importance, String s)
    {
        if(importance > debug_level)
            System.out.println("DEBUG: " +s);
    }

    public static void VERBOSEOUT(String s)
    {
        if (verbose_flag)
            System.out.println(s);
    }

    public static void sysexit(int code)
    {
        System.exit(code);
    }

    public static void check_file_removed(String name, boolean fatal,
					   boolean force_remove_first)
    {
        File f = new File(name);
        if (force_remove_first) {
            f.delete();
        }
        if (f.exists()) {
            if (fatal)
                System.out.print("FAIL: ");
            DEBUGOUT(1, "File \"" + name + "\" still exists after check_file_removed\n");
            if (fatal)
                fail("File \"" + name + "\" still exists after check_file_removed");
        }
    }


    // remove any existing environment or database
    public static void removeall(boolean use_db, boolean remove_env, String envpath, String dbname)
    {
        {
            try {
                if (remove_env)
                    Environment.remove(new File(envpath), true, EnvironmentConfig.DEFAULT);
                if (use_db)
                    Database.remove(dbname, null, DatabaseConfig.DEFAULT);
            }
            catch (DatabaseException dbe) {
                DEBUGOUT(1, "TestUtil::removeall exception caught: " + dbe);
            }
            catch (FileNotFoundException dbe) {
                DEBUGOUT(1, "TestUtil::removeall exception caught: " + dbe);
            }
        }
        check_file_removed(dbname, false, !use_db);
        if (remove_env) {
            for (int i=0; i<8; i++) {
                String fname = envpath + "/" + "__db." + i;
                check_file_removed(fname, true, !use_db);
            }
         
            // ensure the user knows if there is junk remaining.
            // clean out spurious log.00X files
            File dir = new File(envpath);
            if(dir.isDirectory()) {
                String[] remainingfiles = dir.list();
                for(int i = 0; i < remainingfiles.length; i++) {
                    if(remainingfiles[i].startsWith("log") || remainingfiles[i].endsWith("db2") ||
                        remainingfiles[i].endsWith("log") || remainingfiles[i].startsWith("__db")) {
                        DEBUGOUT(1, "TestUtils::removeall removing: " +remainingfiles[i]);
                        check_file_removed(envpath + "/" + remainingfiles[i], false, true);
                    } else {
                        if(remainingfiles[i].indexOf("del") == -1)
                            DEBUGOUT(3, "TestUtils::removeall warning, file: " + remainingfiles[i] + " remains in directory after cleanup.");
                    }
                }
            }
        }
    }
 
    public static boolean removeDir(String dirname)
    {
        try {
            File deldir = new File(dirname);

            if (!deldir.exists()) {
                return true;
            } else if(!deldir.isDirectory()) {
                return false;
            } else {
                File[] contents = deldir.listFiles();
                for (int i = 0; i < contents.length; i++) {
                    if (contents[i].isDirectory())
                        removeDir(contents[i].toString());
                    else 
                        contents[i].delete();
                }
                deldir.delete();
            }
        } catch (Exception e) {
            TestUtils.DEBUGOUT(4, "Warning: error encountered removing directory.\n" + e);
        }
        return true;
    }
 
    static public String shownull(Object o)
    {
        if (o == null)
            return "null";
        else
            return "not null";
    }

    public static void loadSSLConfig(Properties props)
    {
        try {
            String disable_ssl = props.getProperty("disable_ssl");
            DEBUGOUT(2, "disable_ssl : " + disable_ssl);

            if (disable_ssl != null && disable_ssl.length() != 0)
            {
                repmgrSSLDisabled = Boolean.parseBoolean(disable_ssl);

		if (repmgrSSLDisabled == false)
		    DEBUGOUT(3, "SSL enabled for Replication Manager Tests.");
            }
            
            String ca_cert = props.getProperty("ca_cert");
            DEBUGOUT(2, "ca_cert : " + ca_cert);

            if (ca_cert != null && ca_cert.length() != 0)
            {
                repmgrCACert = ca_cert;
            }

            String ca_dir = props.getProperty("ca_dir");
            DEBUGOUT(2, "ca_dir : " + ca_dir);

            if (ca_dir != null && ca_dir.length() != 0)
            {
                repmgrCACert = ca_dir;
            }

            String node_cert = props.getProperty("node_cert");
            DEBUGOUT(2, "node_cert : " + node_cert);

            if (node_cert != null && node_cert.length() != 0)
            {
                repmgrNodeCert = node_cert;
            }

            String node_pkey = props.getProperty("node_pkey");
            DEBUGOUT(2, "node_pkey : " + node_pkey);

            if (node_pkey != null && node_pkey.length() != 0)
            {
                repmgrNodePkey = node_pkey;
            }

            String pkey_passwd = props.getProperty("pkey_passwd");
            DEBUGOUT(2, "pkey_passwd : " + pkey_passwd);

            if (pkey_passwd != null && pkey_passwd.length() != 0)
            {
                repmgrNodePkeyPassword = pkey_passwd;
            }

            String verify_depth = props.getProperty("verify_depth");	
            DEBUGOUT(2, "verify_depth : " + verify_depth);

            if (verify_depth != null && verify_depth.length() != 0)
            {
                repmgrVerifyDepth = Integer.parseInt(verify_depth);
            }
        } catch (Exception e) {
            // expected - the config file is optional.
            DEBUGOUT(0, "loadEnvVars -- loading of default variables failed. error: " + e);
        }
    }

	/*
	 * The config file is not currently required.
	 * The only variable that can be set via the
	 * config file is the base directory for the
	 * tests to be run in. The default is "data"
	 * and will be created for the tests.
	 */
    public static void loadConfig(String envfilename)
    {
        if(config_loaded)
            return;
     
        String configname = envfilename;
        if(envfilename == null)
        {
            String OSStr = java.lang.System.getProperty("os.name");
            if((OSStr.toLowerCase()).indexOf("windows") != -1)
            {
                configname = "config_win32";
            } else {
                // assume a nix variant.
                configname = "config_nix";
            }
        }
        config_loaded = true;
        try {
            InputStream in = new FileInputStream(configname);
            DEBUGOUT(2, "Opened " + configname + " to read configuration.");
            Properties props = new Properties();
            props.load(in);
         
            String var = props.getProperty("BASETEST_DBDIR");
            if(var != null)
            { // Property seems to encase things in "";
                var = var.substring(1);
                var = var.substring(0, var.length() -2);
                BASETEST_DBDIR = var;
            }             
            DEBUGOUT(2, "BASETEST_DBDIR is: " + BASETEST_DBDIR);

            loadSSLConfig(props);

        } catch (Exception e) {
			// expected - the config file is optional.
            DEBUGOUT(0, "loadEnvVars -- loading of default variables failed. error: " + e);
        }
		if (BASETEST_DBDIR == "")
			BASETEST_DBDIR = "data";
        BASETEST_DBFILE = new File(BASETEST_DBDIR);
        if (!BASETEST_DBFILE.exists())
            BASETEST_DBFILE.mkdirs();
        BASETEST_BACKUPDIR = BASETEST_DBDIR + "_bak";
        BASETEST_BACKUPFILE = new File(BASETEST_BACKUPDIR);
        if (!BASETEST_BACKUPFILE.exists())
            BASETEST_BACKUPFILE.mkdirs();
    }
 
    public static String getDBFileName(String dbname)
    {
        DEBUGOUT(1, "getDBFileName returning: " + BASETEST_DBDIR + "/" + dbname);
        return BASETEST_DBDIR + "/" + dbname;
    }

    public static String getBackupFileName(String dbname)
    {
        DEBUGOUT(1, "getBackupFileName returning: " + BASETEST_BACKUPDIR + "/" + dbname);
        return BASETEST_BACKUPDIR + "/" + dbname;
    }
 
    public static OutputStream getErrorStream()
    {
        OutputStream retval = System.err;
        try {
            File outfile = new File(BASETEST_DBDIR + "/" + "errstream.log");
            if(outfile.exists())
            {
                outfile.delete();
                outfile.createNewFile();
            } else {
                outfile.createNewFile();
            }
            retval = new FileOutputStream(outfile);
        } catch (FileNotFoundException fnfe) {
            DEBUGOUT(3, "Unable to open error log file. " + fnfe);
        } catch (IOException ioe) {
            DEBUGOUT(3, "Unable to create error log file. " + ioe);
        }
        return retval;
    }
}
