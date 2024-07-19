/*-
 * Copyright (c) 2011, 2020 Oracle and/or its affiliates.  All rights reserved.
 *
 * See the file LICENSE for license information.
 */

#ifndef HDBREC_H
#define	HDBREC_H

#include "htimestampxa.h"

/*
 * DB record
 */
typedef struct __HDbRec {
	long SeqNo;
	HTimestampData Ts;
	char Msg[10];
} HDbRec;
#endif
