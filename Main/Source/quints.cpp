#pragma once

#include "std_includes.cpp"



#define OUTBOUND_LOW_SPLIT "||__LOW_SPLIT__||"
#define OUTBOUND_HIGH_SPLIT "||__HIGH_SPLIT__||"


#define OUTBOUND_REPLY_MESSAGE "reply_message"
#define OUTBOUND_REPLY_MESSAGE_L_S OUTBOUND_REPLY_MESSAGE OUTBOUND_LOW_SPLIT


#define OUTBOUND_VOICE_CONNECT "voice_connect"
#define OUTBOUND_VOICE_CONNECT_L_S OUTBOUND_VOICE_CONNECT OUTBOUND_LOW_SPLIT
#define OUTBOUND_VOICE_CONNECT_H_S OUTBOUND_VOICE_CONNECT OUTBOUND_HIGH_SPLIT

#define OUTBOUND_VOICE_DISCONNECT "voice_disconnect"
#define OUTBOUND_VOICE_DISCONNECT_L_S OUTBOUND_VOICE_DISCONNECT OUTBOUND_LOW_SPLIT
#define OUTBOUND_VOICE_DISCONNECT_H_S OUTBOUND_VOICE_DISCONNECT OUTBOUND_HIGH_SPLIT

#define OUTBOUND_VOICE_PLAY "voice_play"
#define OUTBOUND_VOICE_PLAY_L_S OUTBOUND_VOICE_PLAY OUTBOUND_LOW_SPLIT



#define SETTINGS_PATH_MASTER std :: string{ ".\\Data\\Settings" }

#define SETTINGS_PATH_VOICE_HI_WAIT "voice_hi_wait.arh"



#define GUILDS_DEFAULT_PREFIX "."

#define GUILDS_PATH_MASTER std :: string{ ".\\Data\\Guilds" }
#define GUILDS_PATH_PREFIX "prefix.arh"



#define USERS_PATH_MASTER std :: string{ ".\\Data\\Users" }
#define USERS_PATH_VOICE_HI "voice_hi.arh"
#define USERS_PATH_VOICE_BYE "voice_bye.arh"
#define USERS_PATH_GUILD_CREDITS "credits.arh"
#define USERS_PATH_GUILD "Guilds"



#define SOUNDS_PATH_MASTER std :: string{ ".\\Data\\Audio" }