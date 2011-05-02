#!/usr/local/bin/perl

$numArgs = $#ARGV + 1;

if ($numArgs != 1) {
	print "usage: ./create_memory_map.pl <num. of servers>\n" ;
    exit(1) ;
}

$num_servers = $ARGV[0] ;

if($num_servers < 4) {
    print "make servers more than 3\n" ;
	exit(1) ;
}

# create configuration file
open CONF_FILE, "+>", "memory_map.conf" or die $! ;

# create random files
for ($memory_addr = 0; $memory_addr < 100; $memory_addr++) {
	$server_id = $memory_addr % $num_servers ;
	print CONF_FILE "$memory_addr $server_id\n" ;
}

close(CONF_FILE) ;
