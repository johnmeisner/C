# Script to gather run-time source components for building on other
#  platforms.

use strict;


sub cd {
    my $dir = shift;
    chdir($dir) or die ("could not chdir to $dir: $!");
}


my $destdir;
my $windows = ($^O =~ /Win/);
my $zipfile;

if ($#ARGV == 0) {
    $destdir = $ARGV[0];
}
else {
    $destdir = "runtime_port";
}

&cd("..");
my $srcdir = `pwd`; # Run-time installation directory...
chomp($srcdir);

print `rm -rf $destdir` if (-e "${destdir}");
mkdir($destdir);
&cd($destdir);

print `cp -R ${srcdir}/rt*src .`;

mkdir("./c");
mkdir("./c/build_lib");
mkdir("./cpp");
mkdir("./cpp/build_lib");

&cd("./c");
print `cp ${srcdir}/c/*.mk .`;
print `chmod +w platform.mk`;
&cd("./build_lib");
print `cp ${srcdir}/c/build_lib/makefile .`;
&cd("../..");

&cd("./cpp");
print `cp ${srcdir}/cpp/*.mk .`;
print `chmod +w platform.mk`;
&cd("./build_lib");
print `cp ${srcdir}/cpp/build_lib/makefile .`;
&cd("../../..");

unlink($zipfile) if (-e "${zipfile}");
if ($windows) {
    $zipfile = "${destdir}.zip";
    print `zip -q -r ${zipfile} ${destdir}`;
}
else {
    $zipfile = "${destdir}.tar.gz";
    print `tar czf ${zipfile} ${destdir}`;
}

if (-e "${zipfile}") {
    print `rm -rf ${destdir}`;
}

