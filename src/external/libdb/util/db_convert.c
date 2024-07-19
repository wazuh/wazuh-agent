/*-
 * Copyright (c) 2016, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"

#ifndef lint
static const char copyright[] =
    "Copyright (c) 2016, 2020 Oracle and/or its affiliates.  All rights reserved.\n";
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
	DB *dbp, *dbvp;
	DB_ENV *dbenv;
	u_int32_t vflag;
	int ch, exitval, lorder, ret, t_ret, verbose;
	char *home, *msgpfx, *passwd, *vopt;

	progname = __db_util_arg_progname(argv[0]);

	if ((ret = __db_util_version_check(progname)) != 0)
		return (ret);

	dbenv = NULL;
	verbose = 0;
	exitval = EXIT_SUCCESS;
	vflag = 0;
	home = msgpfx = passwd = vopt = NULL;
	lorder = __db_isbigendian() ? 4321 : 1234;
	while ((ch = getopt(argc, argv, "bh:lm:P:S:Vv")) != EOF)
		switch (ch) {
		case 'b':
			lorder = 4321;
			break;
		case 'h':
			home = optarg;
			break;
		case 'l':
			lorder = 1234;
			break;
		case 'm':
			msgpfx = optarg;
			break;
		case 'P':
			if (__db_util_arg_password(progname, 
			    optarg, &passwd) != 0)
				goto err;
			break;
		case 'S':
			vopt = optarg;
			switch (*vopt) {
			case 'o':
				vflag = DB_NOORDERCHK;
				break;
			case 'v':
				vflag = 0;
				break;
			default:
				(void)usage();
				goto err;
			}
			break;
		case 'V':
			printf("%s\n", db_version(NULL, NULL, NULL));
			goto done;
		case 'v':
			verbose = 1;
			break;
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

	if (__db_util_env_create(&dbenv, progname, passwd, msgpfx) != 0)
		goto err;

	if (__db_util_env_open(dbenv, home, 0, 1, DB_INIT_MPOOL, 0, NULL) != 0)
		goto err;

	for (; !__db_util_interrupted() && argv[0] != NULL; ++argv) {

		if (vopt != NULL && (db_create(&dbvp, dbenv, 0) != 0 
		    || dbvp->verify(dbvp, argv[0], NULL, stdout, vflag) != 0))
			goto err;

		if ((ret = db_create(&dbp, dbenv, 0)) != 0) {
			fprintf(stderr,
			    "%s: db_create: %s\n", progname, db_strerror(ret));
			goto err;
		}
		dbp->set_errfile(dbp, stderr);
		dbp->set_errpfx(dbp, progname);
		if ((ret = dbp->convert(dbp, argv[0], lorder)) != 0)
			dbp->err(dbp, ret, "DB->convert: %s", argv[0]);
		if ((t_ret = dbp->close(dbp, 0)) != 0 && ret == 0) {
			dbenv->err(dbenv, ret, "DB->close: %s", argv[0]);
			ret = t_ret;
		}
		if (ret != 0)
			goto err;
		/*
		 * People get concerned if they don't see a success message.
		 * If verbose is set, give them one.
		 */
		if (verbose)
			printf(DB_STR_A("5145",
			    "%s: %s converted successfully to %s\n",
			    "%s %s %s\n"), progname, argv[0],
			    lorder == 1234 ? "little-endian" : "big-endian");
	}

	if (0) {
usage_err:	usage();
err:		exitval = EXIT_FAILURE;
	}
done:	if (dbenv != NULL && (ret = dbenv->close(dbenv, 0)) != 0) {
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
	    "[-blVv] [-m msg_pfx] [-h home] [-P password] [-S vo] db_file ...");
}
