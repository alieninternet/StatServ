/* config.h
 * VersionServ configuration file
 * Copyright (c) 2002 Alien Internet Services
 */

#ifndef __CONFIG_H_
# define __CONFIG_H_

// Are users allowed to tell us to ignore them?
# define IGNORE_ALLOWED

// Should we tell a client we got an unknown command, or be quiet?
# define ANNOYING_SILENT_TREATMENT

// Should we have a reply for CTCP SEX? :)
# define CTCP_SEX_REPLY		"Oh, but I have to wash my hair tonight"

// Should we have the 'stats' command?
# define STATS_COMMAND

// Passphrase for the 'stats' command (comment to turn off the passphrase)
//# define STATS_PASSPHRASE	"very secret pass-phrase"

// NickOP identification, comment these out to disable
//# define NICKOP_NAME	"NickOP@austnet.org"
//# define NICKOP_PASSWD	"qwp87346im"

// Various files we need
# define FILE_MOTD	"motd.txt"
# define FILE_HELP	"help.txt"
# define FILE_IGNORES	"ignore.txt"
# define FILE_VERSIONS	"versions.txt"

// Info about us
# define MY_SERVERNAME	"version.mothership.alien.intranet"
# define MY_SERVERDESC	"Client Version Tester"
# define MY_USERNICK	"Statistics"
# define MY_USERDESC	"Statistics Collector: \002/msg " MY_USERNICK " help\002"
# define MY_USERMODES	"+dkRi"
# define MY_USERNAME	"Statistics"
# define MY_USERHOST	"stats.intranet"
# define MY_USERVWHOST	"stats.intranet"

// Server info to connect to
//# define CONNECT_SERVER			0x7F000001
# define CONNECT_SERVER			0xAC1002FE
# define CONNECT_SERVERNAME		"mothership.alien.intranet"
# define CONNECT_PORT			6667
# define CONNECT_PASSWORD		"secretword"

// Various messages to throw at people
# define MSG_IGNORE		"Your nickname is being ignored, " \
   				"it will no longer be checked in the future."
# define MSG_IGNORE_ON		"Your nickname is already being ignored."
# define MSG_UNIGNORE		"Your nickname is no longer being ignored," \
   				"your client will be checked in the future."
# define MSG_UNIGNORE_ON	"Your nickname is not being ignored."
# define MSG_UNKNOWN		"Unknown command - Please type " \
   				"\002/msg Statistics help\002 for assistance"

// Timing stuff (times are in seconds)
# define PING_TIME			300
# define TIMEOUT			600
# define RECONNECT_DELAY		30
# define CHECKPOINT_TIME		3600
# define VERSION_REPLY_TIMEOUT		60

// Would be nice if I fixed these hey? :(
# define DODGEY_SERVER_TS_1		"995639470"
# define DODGEY_SERVER_TS_2		"995691000"

// Built-in bursting test by way of clones (uncomment to enable)
//# define BURST_CLONE_TEST		50
//# define BURST_CLONE_PREFIX		"Clone-"
//# define BURST_CLONE_MODES		"+d"
//# define BURST_CLONE_CHAN_PREFIX	"#CloneChan-"
//# define BURST_CLONE_FAT_CHAN		"#Clones"
//# define BURST_CLONE_SERVERS		50
//# define BURST_CLONE_SERVER_PREFIX	"clone"
//# define BURST_CLONE_SERVER_SUFFIX	".clones.alien.intranet"

/* Are we running SNMP at all?
 * Note - if you uncomment this, you also need to comment out snmp.cpp and
 * the SNMP_LIBS variable in the makefile.
 * Also, you will be wanting ucd-snmp/net-snmp (same thing) version 4.2.4
 * **AT LEAST**, else the sub-agent most likely won't connect to your
 * master agent (I wasted a day on trying to figure that out :)
 */
//# define WITH_SNMP

// Are we running the SNMP portion as an agentx or as a master agent?
//# define SNMP_AS_AGENTX
//# define SNMP_AGENT_NAME		"statserv"
//# define SNMP_MASTER_AGENT_PORT		161

// Size of the malloc()/realloc() chunks for the socket input buffer
# define BUFFER_CHUNK_SIZE		128

// Debugging?
//# define DEBUG

// Debugging the protocol? Protocol will be output via stderr
//# define DEBUG_PROTOCOL

// Allow the 'raw' command to work (should never be allowed in production!!)
//# define ALLOW_RAW_COMMAND

// Version identification
# define VERSION	"StatServ 0.2"

// Make sure we were configured properly
#if (PING_TIME >= TIMEOUT)
# error "PING_TIME is greater than TIMEOUT"
#endif

#endif
