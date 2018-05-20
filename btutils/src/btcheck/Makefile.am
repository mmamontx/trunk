bin_SCRIPTS = btcheck
CLEANFILES = $(bin_SCRIPTS)
EXTRA_DIST = btcheck.pl

btcheck: $(srcdir)/btcheck.pl
	$(PERL) -c btcheck.pl
	rm -f btcheck
	echo "#!$(PERL)" > btcheck
	cat $(srcdir)/btcheck.pl >> btcheck
	chmod ugo+x btcheck
