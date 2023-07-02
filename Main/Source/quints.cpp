#pragma once

#include "std_includes.cpp"



#define OUTBOUND_SPLITTER "||__SPLIT__||"

#define OUTBOUND_MESSAGE_REPLY "message_reply"
#define OUTBOUND_MESSAGE_REPLY_S OUTBOUND_MESSAGE_REPLY OUTBOUND_SPLITTER

#define OUTBOUND_VOICE_CONNECT "voice_connect"
#define OUTBOUND_VOICE_CONNECT_S OUTBOUND_VOICE_CONNECT OUTBOUND_SPLITTER

#define OUTBOUND_VOICE_DISCONNECT "voice_disconnect"
#define OUTBOUND_VOICE_DISCONNECT_S OUTBOUND_VOICE_DISCONNECT OUTBOUND_SPLITTER

#define OUTBOUND_VOICE_PLAY "voice_play"
#define OUTBOUND_VOICE_PLAY_S OUTBOUND_VOICE_PLAY OUTBOUND_SPLITTER



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