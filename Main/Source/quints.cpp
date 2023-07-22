#pragma once

#include "std_includes.cpp"



#define OUTBOUND_LOW_SPLIT "||__LOW_SPLIT__||"
#define OUTBOUND_HIGH_SPLIT "||__HIGH_SPLIT__||"


#define OUTBOUND_SCRIPT "script"
#define OUTBOUND_SCRIPT_L_S OUTBOUND_SCRIPT OUTBOUND_LOW_SPLIT


#define OUTBOUND_REPLY_MESSAGE OUTBOUND_HIGH_SPLIT "reply_message" OUTBOUND_LOW_SPLIT

#define OUTBOUND_REPLY_EMBED OUTBOUND_HIGH_SPLIT "reply_embed" OUTBOUND_LOW_SPLIT


#define OUTBOUND_VOICE_CONNECT OUTBOUND_HIGH_SPLIT "voice_connect" OUTBOUND_LOW_SPLIT

#define OUTBOUND_VOICE_DISCONNECT OUTBOUND_HIGH_SPLIT "voice_disconnect" OUTBOUND_LOW_SPLIT

#define OUTBOUND_VOICE_PLAY OUTBOUND_HIGH_SPLIT "voice_play" OUTBOUND_LOW_SPLIT

#define OUTBOUND_VOICE_STOP OUTBOUND_HIGH_SPLIT "voice_stop" OUTBOUND_LOW_SPLIT


#define OUTBOUND_TICK_GUILD_SET OUTBOUND_HIGH_SPLIT "tick_guild_set" OUTBOUND_LOW_SPLIT



#define INBOUND_MESSAGE "message"
#define INBOUND_VOICE_UPDATE "voice_update"
#define INBOUND_TICK "tick"



#define SETTINGS_PATH_MASTER std :: string{ ".\\Data\\Settings" }

#define SETTINGS_PATH_VOICE_HI_WAIT "voice_hi_wait.arh"

#define SETTINGS_PATH_GAMBLE_RIG "gamble_rig.arh"



#define IMAGES_EMBEDS_PATH_MASTER std :: string{ ".\\Data\\Images\\Embeds" }



#define EMBEDS_COLOR_INFO "5D3FD3"



#define GUILDS_DEFAULT_PREFIX "."

#define GUILDS_PATH_MASTER std :: string{ ".\\Data\\Guilds" }
#define GUILDS_PATH_PREFIX "prefix.arh"
#define GUILDS_PATH_AUTO_VOICE_PLAYS "auto_voice_plays.arh"



#define USERS_PATH_MASTER std :: string{ ".\\Data\\Users" }
#define USERS_PATH_VOICE_HI "voice_hi.arh"
#define USERS_PATH_VOICE_BYE "voice_bye.arh"
#define USERS_PATH_GUILD_CREDITS "credits.arh"
#define USERS_PATH_GUILD "Guilds"



#define SOUNDS_PATH_MASTER std :: string{ ".\\Data\\Audio" }
