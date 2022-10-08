## Process this file with automake to produce Makefile.in
## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

SUBDIRS = \
	cmd_proc\
	cmdn_proc\
	java_proc\
	nfc_proc\
	run_proc\
	sql_proc\
	mgr_proc\
	ok_proc\
	error_proc\
	task_proc\
	manager\
	include\
	src

lss_ldrdocdir = /opt/lss/doc

lss_ldrdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS\
	TODO\
	demo.conf\
	test_xml_library.c

EXTRA_DIST = $(lss_ldrdoc_DATA)



# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

