/* config.h
 * VersionServ configuration file
 * Copyright (c) 2002 Alien Internet Services
 */

#ifndef __CONFIG_H_
# define __CONFIG_H_

// Debugging?
# define DEBUG

// Info about us
# define MY_SERVERNAME			"version.mothership.alien.intranet"
# define MY_SERVERDESC			"Client Version Tester"
# define MY_USERNICK			"Statistics"
# define MY_USERDESC			"Version Statistics Gathering Thing"
# define MY_USERMODES			"+dkRi"
# define MY_USERNAME			"statuser"
# define MY_USERHOST			"stathost"
# define MY_USERVWHOST			"statvwhost"

// Server info to connect to
# define CONNECT_SERVER			0x0100007F // localhost
# define CONNECT_PORT			6667
# define CONNECT_PASSWORD		"secretword"

// Ping stuff..
# define PING_TIME			120 // 2 mins
# define TIMEOUT			300 // 5 mins

// Time between checkpoints
//# define CHECKPOINT_TIME		1800 // 30 mins
# define CHECKPOINT_TIME		43200 // 12 hours

// Funny clone test thing -- This is the number of clones
//# define BURST_CLONE_TEST		50 // disable this to disable clones
//# define BURST_CLONE_PREFIX		"Clone-"
//# define BURST_CLONE_MODES		"+d"
//# define BURST_CLONE_CHAN_PREFIX	"#CloneChan-" // enable channel joins
//# define BURST_CLONE_FAT_CHAN		"#Clones" // one fat channel!
	
#endif
