#pragma once

#include "std_includes.cpp"



#define OUTBOUND_SPLITTER "||SPLIT||"

#define OUTBOUND_MESSAGE_REPLY "message_reply"
#define OUTBOUND_MESSAGE_REPLY_S OUTBOUND_MESSAGE_REPLY OUTBOUND_SPLITTER

#define OUTBOUND_VOICE_CONNECT "voice_connect"
#define OUTBOUND_VOICE_CONNECT_S OUTBOUND_VOICE_CONNECT OUTBOUND_SPLITTER

#define OUTBOUND_VOICE_DISCONNECT "voice_disconnect"
#define OUTBOUND_VOICE_DISCONNECT_S OUTBOUND_VOICE_DISCONNECT OUTBOUND_SPLITTER

#define OUTBOUND_VOICE_PLAY "voice_play"
#define OUTBOUND_VOICE_PLAY_S OUTBOUND_VOICE_PLAY OUTBOUND_SPLITTER


#define GUILDS_DEFAULT_PREFIX "."

#define GUILDS_PATH_MASTER std :: string{ ".\\Data\\Guilds" }
#define GUILDS_PATH_PREFIX "prefix.ahr"


#define USERS_PATH_MASTER std :: string{ ".\\Data\\Users" }
#define USERS_PATH_VOICE_HI "voice_hi.ahr"
#define USERS_PATH_VOICE_BYE "voice_bye.ahr"
#define USERS_PATH_GUILD_CREDITS "credits.ahr"
#define USERS_PATH_GUILD "Guilds"


#define SOUNDS_PATH_MASTER std :: string{ ".\\Data\\Audio" }