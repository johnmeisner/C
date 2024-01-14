To use tlsTest:

1. If necessary, build libaeolinkPKI.so.0 by doing a "make" in the ".." directory above this. This should
   already have been done as the PKI library is part of the standard release.
2. Build the tlsTest execuatable by doing a "make" in this directory. tlsTest will be 
   installed in the ".." directory above. Untar openssl-1.1.1c.tar.gz in the ".." directory and move 
   openssl-1.1.1.c directory to tlsTest.
4. Copy tlsTest to some convenient place on the WSU-5900A.  /mnt/microsd is common.

5. To use IPv4/ethernet you will need to add a route in startup.sh which will get you to a dns server
   and the internet.  The exact nature of this will depend on your network arrangement. Some people
   change the ip address of the 5900, I don't.  I just add a route to my hub which is connected to
   the buildings network.  Add the following lines to near the end of startup.sh:

#
#---Add a route to the default gateway on the network
#
    route add default 192.168.1.1 


5a. Alternatively you might change the ip address of the 5900 to something that is part of the buildings
    network, such as 10.121.20.77 and then add a route like this:

    route add default 10.121.20.254 

    I have not tested the above but others have.

8. Connect the WSU-5900A's Ethernet port to the network.
9. Reboot the WSU-5900A.
10. Execute the tlsTest program on the WSU-5900A, with or without arguements.

Note: to use domain names you will need to add nsswitch.conf and resolv.conf to /etc of the 5900.

   Contents of /et/nsswitch.conf
       hosts: dns

   Contents of /etc/resolv.conf (for ipv4 and at denso vista)
       search densoamericas.com
       nameserver 10.121.20.34
       nameserver 10.72.58.2
       nameserver 10.72.33.46

   Alternatively you might try google's dns servers at:
       8.8.8.8
       8.8.4.4  (for ipv4)

       2001:4860:4860::8888
       2001:4860:4860::8844  (for ipv6) 

   Google's dns servers were used by OBS to run tlsTest and equivalent with ipv4 during their testing.


Last note: when running with ipv6, our code will overwrite the contents of /etc/resolv.conf, so when you
switch back to ipv4 from ipv6, you will need to reset resolv.conf for ipv4.
