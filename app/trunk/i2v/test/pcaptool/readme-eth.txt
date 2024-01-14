README for Ethernet Replay Tool
++++++++++++++++++++++++++++++++++++++++

The Ethernet Replay tool has been designed to take a PCAP file
containing UDP messages and send the messages to the input IP
address and port.  The assumption is that the data is SPaT data
such as what would be provided by a Traffic Signal Controller.

The tool can be modified to support additional types.  However,
do NOT break backwards compatibility.

Compile
===========================
To generate the Ethernet Replay tool, you will run a manual
compile in your environment.  The Makefile should NOT be used;
it will not work.  Use the following to compile the code:

  gcc pcapreplayeth.c -o replay


This will generate the replay tool for your environment.


Usage
==========================
The Ethernet Replay tool is designed for your local machine.
You may find some need to use this on a unit (no clue why).
If you need this to run on a WSU, you must cross compile it.

To operate in your local environment, either supply all
mandatory arguments to the command line, or use the config
file.  The replay.conf is checked as a secondary source for
config parameters.  If all arguments are specified on the
command line, the config file is ignored.  To find out how
to use the utility, enter the command with no options or
config options.

If using the config file, the default directory that is
searched is the current directory from where the utility
is invoked.


PCAP File
==========================
The Ethernet Replay tool requires a traditional PCAP file.
This would be the type of file that tcpdump generates.
There are newer file formats (pcap-ng) that have extra
data that cannot be parsed by the tool.  If you have an
NG file (or something else unique) use Wireshak to
convert the file to the appropriate format.  Wireshark
will allow you an option to specify the format when
saving a file for any file that it successfully opens.

