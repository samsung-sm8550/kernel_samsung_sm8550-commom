/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020-2025 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */

#ifdef DEFEX_KUNIT_ENABLED
#include <include/defex_config.h>
#include <kunit/mock.h>
#include <kunit/test.h>
#include <linux/version.h>
#if KERNEL_VER_GTE(5, 15, 0)
#include <linux/stdarg.h>
#else
#include <stdarg.h>
#endif

#include "feature_trusted_map/include/dtm_engine.h"
#include "dtm_engine_policytest.h"

static int test_ctx(struct dtm_context *ctx,
			     char *caller,
			     char *callee,
			     int mode,
			     const char *program,
			     ...)
{
	va_list ap;

	ctx->defex_context->process_name = caller;
	ctx->defex_context->target_name = callee;
	ctx->stdin_mode_bit = mode;
	memset(ctx->callee_argv, 0, sizeof(ctx->callee_argv));
	ctx->callee_argv[0] = program;
	ctx->program_name = program;
	va_start(ap, program);
	for (ctx->callee_argc = 1; ctx->callee_argc < DTM_MAX_ARGC; ) {
		const char *p = va_arg(ap, char *);

		if (!p)
			break;
		ctx->callee_argv[ctx->callee_argc++] = p;
	}
	va_end(ap);
	return dtm_enforce(ctx);
}

/* Source policy data, to be used instead of normal policy.
 * Text between the "#DONT_CHANGE_THIS_LINE" markers will be extracted and
 * processed by ...buildscript/build_external/defex in order to generate
 * dtm_engine_policytest.h above.
 */
#ifdef __NEVER_DEFINED__
#DONT_CHANGE_THIS_LINE, test policy starts below
[common]
/system/bin/nobody:/system/bin/toybox:CHR:*
/system/bin/nobody:/vendor/bin/toybox_vendor:CHR:*
/system/bin/sh:/system/bin/toybox:CHR:log:-t:art_apex:-p:f
/system/bin/sh:/system/bin/toybox:CHR:log:-t:art_apex:-p:i
/system/bin/sh:/system/bin/toybox:CHR:log:-t:Art_apex:-p:i
/system/bin/sh:/system/bin/toybox:CHR:log:-t:Art_apex
/system/bin/sh:/system/bin/toybox:CHR:log:-t:last_arg
/system/bin/sh:/system/bin/toybox:CHR:log:-t:install_recovery:*
/system/bin/sh:/system/bin/toybox:CHR:log:--any:*
/system/bin/sh:/system/bin/toybox:CHR:log:-t:recovery:*
/system/bin/sh:/system/bin/{alpha,bravo,charlie,delta,echo,foxtrot,golf,hotel,india,juliet,kilo}:CHR:log:-t:Art_apex
/vendor/bin/sh:/vendor/bin/toybox_vendor:CHR:log:-t:BOOT:*
# Checking stdin modes: specific programs
/system/bin/sh:/system/bin/toybox:CHR,FIFO:{grep,head}:*
# Checking stdin modes: any programs
/system/bin/sh:/system/bin/toybox2:LNK:*:*
# From An Seongjin
###/system/bin/sh:/system/bin/toybox2:{CHR,FIFO,SOCK}:*
###/system/bin/sh:/system/bin/toybox2:{CHR,FIFO,SOCK}:*:*
#/vendor/bin/sh:/vendor/bin/sh:CHR:/vendor/bin/sh:/vendor/bin/init.qti
/vendor/bin/sh:/vendor/bin/sh:CHR:/vendor/bin/sh:abc
/vendor/bin/sh:/vendor/bin/sh:CHR:/vendor/bin/sh:Abc:def/ghi/jkl
/vendor/bin/sh:/vendor/bin/sh:CHR:/vendor/bin/sh:ABC:/DEF/GHI/JKL:m/n/o
# Extended wildcards
/system/bin/sh:/system/bin/toybox:CHR:{rm,mv}:/tmp/*
/system/bin/sh:/system/bin/toybox:CHR:{rm2,mv}:-i:/tmp/*
/system/bin/sh:/system/bin/toybox:CHR:{rm3,mv}:/var/tmp*:/var/share
/apex/com.android.adbd/bin/adbd:/system/bin/sh:SOCK:/system/bin/sh:-c:"svc power*"
/system/bin/init:/vendor/bin/sh:CHR:/vendor/bin/sh:/vendor/bin/init*
# Special characters
/sys\\tem/bin/sh:/sy\\stem/bin/toy\BOX:C\HR:log:' x\: y\'\x25z\\':" blank \\n":" 'nested quote' \" "

/system/bin/sh:/system/bin/sh:CHR:/data/local/tmp/test.sh:allow:if:unchanged
/system/bin/sh:/system/bin/sh:CHR:/system/bin/sh:/data/local/tmp/test.sh:allow:if:unchanged
[debug]
# Wait for integration of new features

#DONT_CHANGE_THIS_LINE, test policy ends above */
#endif

static void dtm_engine_enforce_test(struct kunit *test)
{
	struct dtm_context *ctx; /* Avoid stack overrun in some devices */
	struct defex_context *defex_ctx;
	struct cred defex_cred;
	/* Just for is_dtm_context_valid to succeed;
	 * Value does not matter as long as dtm_context->callee_argv[n] is set
	 */
	char dummy_user_arg_ptr[100];
	/* Shortcut helpers */
	#define CER1_P "/system/bin/"
	#define CEE1_P "/system/bin/"
	#define CHR DTM_FD_MODE_CHR
	#define BLK DTM_FD_MODE_BLK
	#define FIFO DTM_FD_MODE_FIFO
	#define LNK DTM_FD_MODE_LNK
	#define SOCK DTM_FD_MODE_SOCK

	defex_ctx = kzalloc(sizeof(struct defex_context), GFP_KERNEL);
	if (!defex_ctx) {
		KUNIT_FAIL(test, "Not enough memory for defex_context");
		return;
	}
	memset(&defex_cred, 0, sizeof(defex_cred));
	defex_ctx->cred = &defex_cred;
	ctx = kzalloc(sizeof(struct dtm_context), GFP_KERNEL);
	if (!ctx) {
		KUNIT_FAIL(test, "Not enough memory for dtm_context");
		goto error_dtm_ctx;
	}
	ctx->defex_context = defex_ctx;
	defex_ctx->task = current; /* Just so is_dtm_context_valid succeeds */
	memset(dummy_user_arg_ptr, 0, sizeof(dummy_user_arg_ptr));
	ctx->callee_argv_ref = dummy_user_arg_ptr;
	dtm_engine_override_data(dtm_engine_defaultpolicy_test);
	/* Pathological cases first: */
	KUNIT_EXPECT_EQ(test, dtm_enforce(0), DTM_DENY);
	/* If callee is null, DEFEX concocts a "<unknown filename>", so accept */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, 0, 0, CHR, "log", (const char *)NULL),
			DTM_ALLOW);
	/* Ditto for null caller; but if not found, must deny */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, 0, CEE1_P "toybox", CHR, "log",
				       (const char *)NULL), DTM_DENY);
	/* Ordinary cases: */
	/* callee not found, should allow */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "nobody", CEE1_P "SHELL",
				 CHR, "PROGRAM", (const char *)NULL), DTM_ALLOW);
	/* program may be "*", allow */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "nobody", CEE1_P "toybox",
				 CHR, "PROGRAM", (const char *)NULL), DTM_ALLOW);
	/* mode doesn't match, deny */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "nobody", CEE1_P "toybox",
				 DTM_FD_MODE_BLK, "PROGRAM", (const char *)NULL),
				 DTM_DENY);
	/* caller SH not found, deny */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "SH", CEE1_P "toybox",
				 CHR, "log", "-t", (const char *)NULL), DTM_DENY);
	/* callee TOYBOX not found, allow */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "TOYBOX",
				 CHR, "log", "-t", (const char *)NULL), DTM_ALLOW);
	/* program LOG not found, deny */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "LOG", "-t", (const char *)NULL), DTM_DENY);
	/* caller, callee, program, last argument mismatch, deny */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "log", "-t", "last", (const char *)NULL),
				 DTM_DENY);
	/* argument is install_recovery, not install_recover: deny */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "log", "-t", "install_recover",
				 (const char *)NULL), DTM_DENY);
	/* arguments -t, install_recovery match: allow */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "log", "-t", "install_recovery",
				 (const char *)NULL), DTM_ALLOW);
	/* anything after argument --any accepted, allow */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "log", "--any", "XXX", "YYY",
				 (const char *)NULL), DTM_ALLOW);
	/* argument -x unknown, deny */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "log", "-x", (const char *)NULL), DTM_DENY);
	/* argument -x unknown, deny */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "log", "-t", "-x",
				 (const char *)NULL), DTM_DENY);
	/* 2 arguments checked but more are required */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "log", "-t", "art_apex",
				 (const char *)NULL), DTM_DENY);
	/* argument -x unknown, deny */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "log", "-t", "art_apex", "-x",
				 (const char *)NULL),
		  DTM_DENY);
	/* all arguments check, allow */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "log", "-t", "art_apex", "-p",
				 "i", (const char *)NULL), DTM_ALLOW);
	/* argument -I unknown, deny */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "log", "-t", "art_apex", "-p", "I",
				 (const char *)NULL), DTM_DENY);
	/* all arguments checked, allow */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox", CHR,
				 "log", "-t", "Art_apex", "-p", "i",
				 (const char *)NULL),
		  DTM_ALLOW);
	/* similar (just to check binsearch) */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "delta",
				 CHR, "log", "-t", "Art_apex", (const char *)NULL),
		  DTM_ALLOW);
	/* any argument after install_recovery accepted, allow */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "log", "-t", "install_recovery", "-p", "I",
				 (const char *)NULL), DTM_ALLOW);
	/* Test stdin modes: specific programs */
	/* accept if program found and one of the accepted modes */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "grepx", "look", "for", (const char *)NULL),
		  DTM_DENY);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 FIFO, "grepx", "look", "for", (const char *)NULL),
		  DTM_DENY);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 FIFO, "grep", "look", "for", (const char *)NULL),
		  DTM_ALLOW);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 FIFO, "head", "look", "for", (const char *)NULL),
		  DTM_ALLOW);
	/* program found, wrong mode, deny */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 BLK, "grep", "look", "for", (const char *)NULL),
		  DTM_DENY);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 BLK, "head", "look", "for", (const char *)NULL),
		  DTM_DENY);
	/* Test stdin modes: any program */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox2",
				 BLK, "tail", "don't care", (const char *)NULL),
		  DTM_DENY);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox2",
				 LNK, "tail", "don't care", (const char *)NULL),
		  DTM_ALLOW);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox2",
				 LNK, "grep", "don't care", (const char *)NULL),
		  DTM_ALLOW);
	/* No rule allows SOCK, deny */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 SOCK, "grep", "don't care", (const char *)NULL),
		  DTM_DENY);
	/* Arguments with paths */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, "/vendor/bin/sh", "/vendor/bin/sh",
				 CHR, "/vendor/bin/sh", "Abc", "def/ghi/jkl",
				(const char *)NULL),
		  DTM_ALLOW);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, "/vendor/bin/sh", "/vendor/bin/sh",
				 CHR, "/vendor/bin/sh", "abc", "def/Ghi/jkl",
				 (const char *)NULL),
		  DTM_DENY); // too many args
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, "/vendor/bin/sh", "/vendor/bin/sh",
				 CHR, "/vendor/bin/sh", "Abc", "def/Ghi/jkl",
				(const char *)NULL),
		  DTM_DENY); // argv[2] mismatch
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, "/vendor/bin/sh", "/vendor/bin/sh",
				 CHR, "/vendor/bin/sh", "ABC", "/DEF/GHI/JKL",
				 (const char *)NULL),
		  DTM_DENY); // missing argv[3]
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, "/vendor/bin/sh", "/vendor/bin/sh",
				 CHR, "/vendor/bin/sh", "ABC", "/DEF/GHI/JKL", "m/n/o",
				 (const char *)NULL),
		  DTM_ALLOW);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, "/vendor/bin/sh", "/vendor/bin/sh",
				 CHR, "/vendor/bin/sh", "ABC", "DEF/GHI/JKL", "m/n/o",
				 (const char *)NULL),
		  DTM_DENY); // argv[2] mmismatch
	// Quoting and other special characters
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, "/sys\\tem/bin/sh",
				 "/sy\\stem/bin/toyBOX", CHR, "log",  " x: y'%z\\",
				 " blank \\n", " 'nested quote' \" ",
				 (const char *)NULL),
		  DTM_ALLOW);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, "/sys\\tem/bin/sh",
				 "/sy\\stem/bin/toyBOX", CHR, "log",  " x: y'%z\\",
				 " blank", " 'nested quote' \" ",
				 (const char *)NULL),
		  DTM_DENY);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, "/sys\\tem/bin/sh",
				 "/sy\\stem/bin/toyBOX", CHR, "log",  " x: y'%z\\",
				 " blank \\n", " 'nested quote' ' ",
				 (const char *)NULL),
		  DTM_DENY);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "rm", "/tmp/xxx", (const char *)NULL),
		  DTM_ALLOW);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "rm", "/proc/xxx", (const char *)NULL),
		  DTM_DENY);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "rm2", "/tmp/xxx", (const char *)NULL),
		  DTM_DENY);  /* missing penultimate argument */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "rm2", "-i", "/tmp/xxx", (const char *)NULL),
		  DTM_ALLOW);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "rm2", "-i", "/tmp", (const char *)NULL),
		  DTM_DENY);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "rm2", "-i", "/tmp/", (const char *)NULL),
		  DTM_ALLOW); /* variable part may be empty */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "rm3", "/var/tmpxxx/abc",
				"/var/sharex", (const char *)NULL),
		  DTM_DENY);  /* mismatch in last argument */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, CER1_P "sh", CEE1_P "toybox",
				 CHR, "rm3", "/var/tmpxxx/abc",
				"/var/share", (const char *)NULL),
		  DTM_ALLOW);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, "/apex/com.android.adbd/bin/adbd",
				 "/system/bin/sh", SOCK, "/system/bin/sh", "-c",
				 "svc power stayon true",
				 (const char *)NULL),
		  DTM_ALLOW);
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, "/apex/com.android.adbd/bin/adbd",
				 "/system/bin/sh", SOCK, "/system/bin/sh", "-c",
				 "svc xpower stayon true",
				 (const char *)NULL),
		  DTM_DENY); /* mismatch, "svc power" x "svc xpower" */
	KUNIT_EXPECT_EQ(test, test_ctx(ctx, "/system/bin/init", "/vendor/bin/sh",
				 CHR, "/vendor/bin/sh", "/vendor/bin/init.qti.kernel.early_debug.sh",
				 (const char *)NULL),
		  DTM_ALLOW);

	dtm_engine_override_data(NULL);
	kfree(ctx);
error_dtm_ctx:
	kfree(defex_ctx);
}

static struct kunit_case dtm_engine_test_cases[] = {
	KUNIT_CASE(dtm_engine_enforce_test),
	{},
};

static struct kunit_suite dtm_engine_test_module = {
	.name = "defex_dtm_engine_test",
	.test_cases = dtm_engine_test_cases,
};
kunit_test_suites(&dtm_engine_test_module);
#endif /* if DEFEX_KUNIT_ENABLED */
