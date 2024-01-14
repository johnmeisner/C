# generate dependency rules file (c_rules.mk) 
# usage: perl mkdepend.pl

$g_cfile = 0;

sub checkDepends {
    local $srcdir = shift;
    local $mtime;
    
    if (-e "c_rules.mk") {
        $mtime = (stat("c_rules.mk"))[9];
        my $timestamp = localtime($mtime);
        # print "c_rules.mk mod time = $timestamp\n";
    }
    else { return 1; }

    opendir (SRCDIR, "$srcdir") ||
        die ("unable to open this directory");

    while ($filename = readdir (SRCDIR)) {
        # print "$filename\n";
        if ($filename =~ /\.[ch]$/) {
            my $mtime2 = (stat("$srcdir/$filename"))[9];
            # print ("$filename : $mtime2\n");
            if ($mtime2 > $mtime) {
                print "$srcdir/$filename was modified after c_rules.mk\n";
                return 1;
            }
        }
    }

    closedir (SRCDIR);

    return 0;
}

exit 0 if (&checkDepends('.') == 0 && 
           &checkDepends('./include') == 0 &&
           &checkDepends('./include/libxml') == 0);

# loop through all cpp files in the source directory

if (-e "c_rules.mk") {
    `rm -f c_rules.mk~`;
    `mv c_rules.mk c_rules.mk~`;
}
open (OUTFILE, ">c_rules.mk") || die ("could not open c_rules.mk for output");
print OUTFILE "# C to object file compilation rules\n";

$cccmd = "\t\$(CC) \$(CFLAGS) \$(LIBXML2DEFS) -c -I\$(LIBXML2INC) \$(OBJOUT)";

&addRules ("../libxml2src/*.c", $cccmd);

# add compile command for last rule
if ($g_cfile) {
    print OUTFILE "$cccmd $g_cfile\n";
    $g_cfile = 0;
}

close (OUTFILE);


sub addRules {
    local $sources = shift;
    local $cccmd = shift;
    local $extline = 0;
    local @rules = (`gcc $sources -E -MM -I. -I../libxml2src/include`);

    foreach $line (@rules) {
        # if line starts with a non-whitespace character, it is the 
        # start of a new rule ..
        if ($line =~ /^\w/) {
            # change .o to $(OBJ)
            $line =~ s/\.o/\$(OBJ)/;

            # add compile command
            if ($g_cfile) {
                print OUTFILE "$cccmd $g_cfile\n";
            }
            # add a newline 
            print OUTFILE "\n";

            # get C source file from rule
            $line =~ /^\w+\$\(OBJ\)\:\s+([\w\.\/]+)/;
            $g_cfile = $1;
            $g_cfile =~ s/\.\.\/libxml2src/\$\(LIBXML2SRCDIR\)/g;
            $line = '$(OBJDIR)$(PS)' . $line;

            # it is possible that if target file name is long, source file 
            # name will be on next line..
            $extline = 1 if (!$g_cfile);
        }
        elsif ($extline) {
            $line =~ /^\s+([\w\.\/]+)/;
            $g_cfile = $1;
            $g_cfile =~ s/\.\.\/libxml2src/\$\(LIBXML2SRCDIR\)/g;
            $extline = 0;
        }

        # change source file paths to use macro names
        $line =~ s/\.\.\/libxml2src\/include/\$\(LIBXML2INC\)/g;
        $line =~ s/\.\.\/libxml2src/\$\(LIBXML2SRCDIR\)/g;

        print OUTFILE $line;
    }
}

