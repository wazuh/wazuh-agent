/*-
 * Copyright (c) 1996, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"

#ifndef lint
static const char copyright[] =
    "Copyright (c) 1996, 2020 Oracle and/or its affiliates.  All rights reserved.\n";
#endif

int main __P((int, char *[]));
void usage __P((void));

const char *progname;

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	DB *dbp;
	DB_ENV *dbenv;
	u_int32_t flags, cache;
	int ch, exitval, mflag, nflag, private;
	int quiet, ret;
	char *blob_dir, *dname, *fname, *home, *passwd;

	progname = __db_util_arg_progname(argv[0]);

	if ((ret = __db_util_version_check(progname)) != 0)
		return (ret);

	dbenv = NULL;
	dbp = NULL;
	/*
	 * If db_verify creates a private environment, use a 10MB cache
	 * so that we don't fail when verifying large databases.
	 */
	cache = 10 * MEGABYTE;
	mflag = nflag = quiet = 0;
	flags = 0;
	exitval = EXIT_SUCCESS;
	blob_dir = home = passwd = NULL;
	while ((ch = getopt(argc, argv, "b:h:mNoP:quV")) != EOF)
		switch (ch) {
		case 'b':
			blob_dir = optarg;
			break;
		case 'h':
			home = optarg;
			break;
		case 'm':
			mflag = 1;
			break;
		case 'N':
			nflag = 1;
			break;
		case 'P':
			if (__db_util_arg_password(progname, 
			    optarg, &passwd) != 0)
				goto err;
			break;
		case 'o':
			LF_SET(DB_NOORDERCHK);
			break;
		case 'q':
			quiet = 1;
			break;
		case 'u':			/* Undocumented. */
			LF_SET(DB_UNREF);
			break;
		case 'V':
			printf("%s\n", db_version(NULL, NULL, NULL));
			goto done;
		case '?':
		default:
			goto usage_err;
		}
	argc -= optind;
	argv += optind;

	if (argc <= 0)
		goto usage_err;

	/* Handle possible interruptions. */
	__db_util_siginit();

	/*
	 * Create an environment object and initialize it for error
	 * reporting.
	 */
	if ((ret = db_env_create(&dbenv, 0)) != 0) {
		fprintf(stderr,
		    "%s: db_env_create: %s\n", progname, db_strerror(ret));
		goto err;
	}

	if (!quiet) {
		dbenv->set_errfile(dbenv, stderr);
		dbenv->set_errpfx(dbenv, progname);
	} else {
		dbenv->set_errfile(dbenv, NULL);
		dbenv->set_msgfile(dbenv, NULL);
	}

	if (blob_dir != NULL &&
	    (ret = dbenv->set_blob_dir(dbenv, blob_dir)) != 0) {
		dbenv->err(dbenv, ret, "set_blob_dir");
		goto err;
	}

	if (nflag) {
		if ((ret = dbenv->set_flags(dbenv, DB_NOLOCKING, 1)) != 0) {
			dbenv->err(dbenv, ret, "set_flags: DB_NOLOCKING");
			goto err;
		}
		if ((ret = dbenv->set_flags(dbenv, DB_NOPANIC, 1)) != 0) {
			dbenv->err(dbenv, ret, "set_flags: DB_NOPANIC");
			goto err;
		}
	}

	if (passwd != NULL &&
	    (ret = dbenv->set_encrypt(dbenv, passwd, DB_ENCRYPT_AES)) != 0) {
		dbenv->err(dbenv, ret, "set_passwd");
		goto err;
	}
	/*
	 * Try to attach to an existing environment -- and its associated
	 * mpool. This lets db_verify access the most up-to-date version of
	 * the database's pages.
	 */
	if (__db_util_env_open(dbenv,
	    home, DB_INIT_MPOOL, 1, DB_INIT_MPOOL, cache, &private) != 0)
		goto err;

	/*
	 * Find out if we have a transactional environment so that we can
	 * make sure that we don't open the verify database with logging
	 * enabled.
	 */
	for (; !__db_util_interrupted() && argv[0] != NULL; ++argv) {
		if (mflag) {
			dname = argv[0];
			fname = NULL;
		} else {
			fname = argv[0];
			dname = NULL;
		}

		if ((ret = db_create(&dbp, dbenv, 0)) != 0) {
			dbenv->err(dbenv, ret, "%s: db_create", progname);
			goto err;
		}

		if (TXN_ON(dbenv->env) &&
		    (ret = dbp->set_flags(dbp, DB_TXN_NOT_DURABLE)) != 0) {
			dbenv->err(
			    dbenv, ret, "%s: db_set_flags", progname);
			goto err;
		}

		/* The verify method is a destructor. */
		ret = dbp->verify(dbp, fname, dname, NULL, flags);
		dbp = NULL;
		if (ret != 0)
			exitval = EXIT_FAILURE;
		if (!quiet)
			printf(DB_STR_A("5105", "Verification of %s %s.\n",
			    "%s %s\n"), argv[0], ret == 0 ? 
			    DB_STR_P("succeeded") : DB_STR_P("failed"));
	}

	if (0) {
usage_err:	usage();
err:		exitval = EXIT_FAILURE;
	}
done:
	if (dbp != NULL && (ret = dbp->close(dbp, 0)) != 0) {
		exitval = EXIT_FAILURE;
		dbenv->err(dbenv, ret, DB_STR("0164", "close"));
	}
	if (dbenv != NULL && (ret = dbenv->close(dbenv, 0)) != 0) {
		exitval = EXIT_FAILURE;
		fprintf(stderr,
		    "%s: dbenv->close: %s\n", progname, db_strerror(ret));
	}

	if (passwd != NULL)
		free(passwd);

	/* Resend any caught signal. */
	__db_util_sigresend();

	return (exitval);
}

void
usage()
{
	fprintf(stderr, "usage: %s %s\n", progname,
	    "[-mNoqV] [-b blob_dir] [-h home] [-P password] db_file ...");
}
