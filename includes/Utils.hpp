#ifndef	UTILS_HPP
#define	UTILS_HPP

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <string.h>
#include <vector>
/* might need to add \r\n at the end of each reply !*/

//ERROR REPLIES
#define ERR_ALREADYLOGEDIN(source)						":" + source + " 460 " ":Already logged in\r\n"
#define ERR_ALREADYREGISTRED(source)					":" + source + " 462 " ":Already registered in\r\n"
#define ERR_NEEDMOREPARAMS(source, command)				":" + source + " 461 " + command + " :Not enough parameters\r\n"  //!!
#define ERR_PASSWDMISMATCH(source)						":" + source + " 464 " + "PASS" + " :Password incorrect\r\n"
#define ERR_NONICKNAMEGIVEN(source)						":" + source + " 431 " + "NICK" + " :Nickname not given\r\n"	//!!

#define ERR_NICKNAMEINUSE(src, nick, used)				":" + src + " 433 " + nick + " " + used + " :Nickname is already in use\r\n"
#define ERR_NOSUCHCHANNEL(src, nick, channel)			":" + src + " 403 " +  nick + " " + channel + " :No such channel\r\n"
#define ERR_NOTONCHANNEL(src, nick, channel)			":" + src + " 401 " + nick + " " + channel + " :No such nick/channel\r\n"
#define ERR_CANNOTSENDTOCHAN(src, nick, channel)		":" + src +  " 404 " + nick + " " + channel + " :Cannot send to channel\r\n"
#define ERR_USERSDONTMATCH(src, nick)					":" + src +  " 502 " + nick + " :Cannot change mode for other users\r\n"
#define	ERR_UMODEUNKNOWNFLAUSR(src, nick)				":" + src +  " 501 " + nick + " :Unknown MODE flags\r\n"
#define	ERR_UMODEUNKNOWNFLAGCH(src, nick)				":" + src +  " 472 " + nick + " :Unknown mode char to me\r\n"
#define ERR_CHANOPRIVSNEEDED(src, nick, channel)		":" + src +  " 482 " + nick + " " + channel + " :You're not channel operator \r\n"

#define ERR_ERRONEUSNICKNAME(nick)						"432 " + nick + ":Erroneous nickname"

#define ERR_UNKNOWNCOMMAND(source, command)				"421 " + source + " " + command + " :Unknown command"
#define ERR_NOTREGISTERED(source)						":" + source + " 451 " + ":You have not registered"
#define ERR_TOOMANYCHANNELS(source, channel)			"405 " + source + " " + channel + " :You have joined too many channels"
#define ERR_BADCHANNELKEY(source, channel)				"475 " + source + " " + channel + " :Cannot join channel (+k)"
#define ERR_NOSUCHNICK(source, nickname)				"401 " + source + " " + nickname + " :No such nick/channel"
#define ERR_USERNOTINCHANNEL(source, nickname, channel)	"441 " + source + " " + nickname + " " + channel + " :They aren't on that channel"
#define ERR_CHANNELISFULL(source, channel)				"471 " + source + " " + channel + " :Cannot join channel (+l)"

// NUMERIC REPLIES
#define	RPL_CAP(src)										":" + src + " CAP * LS :cap reply...\r\n"
#define RPL_PING(src, token)								":" + src + " PONG " + src + " :" + token + "\r\n"
#define RPL_WELCOME(src, nick, user, host)					":" + src + " 001 " + nick + " :Welcome to the ft_irc network " + nick + "!" + user + "@" + host + "\r\n"
#define	RPL_NICKCHANGE(nick, user, user_host, nw_nick)		":" + nick + "!" + user + "@" + user_host + " " + "NICK" + " :" + nw_nick + "\r\n"
#define RPL_JOIN(nick, user, user_host, channel)			":" + nick + "!" + user + "@" + user_host + " JOIN " + channel + " * :" + user + "\r\n"
#define	RPL_TOPIC(src, nick, channel, topic)				":" + src + " 332 " + nick + " " + channel + " :" + topic + "\r\n"
#define RPL_NOTOPIC(src, nick, channel)						":" + src + " 331 " + nick + " " + channel + " :No topic is set\r\n"
#define	RPL_NAMREPLY(src, nick, channel)					":" + src + " 353 " + nick + " = " + channel + " :"
#define RPL_ENDOFNAMES(src, nick, channel)					":" + src + " 366 " + nick + " " + channel + " :END of NAMES list\r\n"

// COMMAND REPLIES
#define RPL_PART(src_nick, src_usr, src_host, channel)				":" + src_nick + "!" + src_usr + "@" + src_host + " PART " + channel + " :gonee... :'( \r\n"
#define RPL_PRIVMSG(src_nick, src_usr, src_host, dis_nick, msg)		":" + src_nick + "!" + src_usr + "@" + src_host + " PRIVMSG " + dis_nick + " :" + msg + "\r\n"
#define RPL_NOTICE(src_nick, src_usr, src_host, dis_nick, msg)		":" + src_nick + "!" + src_usr + "@" + src_host + " NOTICE " + dis_nick + " :" + msg + "\r\n"
#define RPL_QUIT(src_nick, src_usr, src_host)						":" + src_nick + "!" + src_usr + "@" + src_host + " QUIT :Client Quit \r\n"
#define RPL_TOPICCHANGE(src_nick, src_usr, src_host, channel, topic)":" + src_nick + "!" + src_usr + "@" + src_host + " TOPIC " + channel + " :" + topic + "\r\n"

#define RPL_KICK(source, channel, target, reason)					":" + source + " KICK " + channel + " " + target + " :" + reason
#define RPL_MODEUSER(src, nick, mode)								":" + src + " 221 " + nick + " " + mode + "\r\n"
#define RPL_MODECHANNEL(src, nick,channel, mode)					":" + src + " 324 " + nick + " " + channel + " " + mode + "\r\n"

std::vector<std::string>	ft_split(const char buffer[1000], std::string delim);

#endif