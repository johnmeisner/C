
#
# Runs some basic conf_agent/conf_manager tests
#
# USAGE: Run on RSU: perl test_cfgmgr.pl
#
# NOTE: Restores changed values, so should not wipe out your configs!

use strict;

my $mypid = $$;

# Test out HTTP service port
print "Testing changing of HTTP service port\n";
my $origval = `conf_agent READ RSU_INFO WEBGUI_HTTP_SERVICE_PORT`;
chomp $origval;

$_ = `conf_agent WRITE RSU_INFO WEBGUI_HTTP_SERVICE_PORT $mypid`;
my $chk_val = `conf_agent READ RSU_INFO WEBGUI_HTTP_SERVICE_PORT`;
chomp $chk_val;

if ($chk_val == $mypid) {
    print "  Passed.\n";
} else {
    print "  FAILED: Saved value $mypid but check returned $chk_val\n";
}

# Restore original config value
$_ = `conf_agent WRITE RSU_INFO WEBGUI_HTTP_SERVICE_PORT $mypid`;

# HD note: Need to send OID's with V3 security. See ./testSuite/test_rsu_ntcip_1218.py
# Test on SNMP port
print "Testing changing of SNMP port\n";
my $origval = `conf_agent READ snmpd agentaddress`;
chomp $origval;

$_ = `conf_agent WRITE snmpd agentaddress $mypid`;
my $chk_val = `conf_agent READ snmpd agentaddress`;
chomp $chk_val;

if ($chk_val == $mypid) {
    print "  Passed.\n";
} else {
    print "  FAILED: Saved value $mypid but check returned $chk_val\n";
}

# Restore original config value
$_ = `conf_agent WRITE snmpd agentaddress $origval`;
