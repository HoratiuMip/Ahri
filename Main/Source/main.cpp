#include "std_includes.cpp"
#include "quints.cpp"
#include "utility.cpp"



#define RAND rand()



class Settings {
public:
    static void voice_hi_wait_to( size_t value ) {
        File :: overwrite(
            SETTINGS_PATH_MASTER,
            SETTINGS_PATH_VOICE_HI_WAIT,
            value
        );
    }

    static size_t voice_hi_wait() {
        return File :: read< size_t >(
            SETTINGS_PATH_MASTER,
            SETTINGS_PATH_VOICE_HI_WAIT,
            1500
        );
    }

public:
    class Gamble {
    public:
        static void rig_to( double value ) {
            File :: overwrite(
                SETTINGS_PATH_MASTER,
                SETTINGS_PATH_GAMBLE_RIG,
                value
            );
        }

        static double rig() {
            return File :: read< double >(
                SETTINGS_PATH_MASTER,
                SETTINGS_PATH_GAMBLE_RIG,
                0.0
            );
        }

    };

};



class Guild : public Has_id {
public:
    Guild() = default;

    using Has_id :: Has_id;

public:
    double multiplier() const {
        return 1.0;
    }

public:
    void prefix_to( const std :: string_view& value ) {
        File :: overwrite(
            GUILDS_PATH_MASTER + '\\' + this -> _id,
            GUILDS_PATH_PREFIX,
            value
        );
    }

    std :: string prefix() {
        return File :: read< std :: string >(
            GUILDS_PATH_MASTER + '\\' + this -> _id,
            GUILDS_PATH_PREFIX,
            GUILDS_DEFAULT_PREFIX
        );
    }

public:
    auto voice_auto_plays() {
        Voice_auto_plays_pairs pairs{};

        File :: for_each< std :: string, double >(
            GUILDS_PATH_MASTER + '\\' + this -> _id,
            GUILDS_PATH_AUTO_VOICE_PLAYS,

            [ & ] ( std :: string& sound, double& prob ) -> void {
                if( sound.empty() )
                    return;

                pairs.emplace_back( std :: move( sound ), prob );
            }
        );

        return pairs;
    }

    void voice_auto_plays_to( Voice_auto_plays_pairs& pairs ) {
        std :: stringstream formated{};

        std :: sort( pairs.begin(), pairs.end(), [] ( auto& pair1, auto& pair2 ) -> bool {
            return pair1.second > pair2.second;
        } );

        for( auto& pair : pairs )
            formated << pair.first << ' ' << pair.second << '\n';

        File :: overwrite(
            GUILDS_PATH_MASTER + '\\' + this -> _id,
            GUILDS_PATH_AUTO_VOICE_PLAYS,
            formated.str()
        );
    }

    bool voice_auto_plays_calibrate( double prob, Voice_auto_plays_pairs& pairs ) {
        double acc = 0.0;

        for( auto& pair : pairs )
            acc += pair.second;

        if( prob + acc <= 1.0 )
            return false;

        for( auto& pair : pairs )
            pair.second *= ( 1.0 - prob ) / acc;

        return true;
    }

};


class User : public Has_id {
public:
    User() = default;

    using Has_id :: Has_id;

public:
    void credits_to( size_t value, Guild guild ) {
        File :: overwrite(
            USERS_PATH_MASTER + '\\' + this -> _id
                              + '\\' + USERS_PATH_GUILD
                              + '\\' + guild.id(),
            USERS_PATH_GUILD_CREDITS,
            value
        );
    }

    size_t credits( Guild guild ) {
        return File :: read< size_t >(
            USERS_PATH_MASTER + '\\' + this -> _id
                              + '\\' + USERS_PATH_GUILD
                              + '\\' + guild.id(),
            USERS_PATH_GUILD_CREDITS,
            0
        );
    }

    void credits_add( size_t value, Guild guild ) {
        this -> credits_to(
            ( this -> credits( guild ) + value ) * guild.multiplier(),
            guild
        );
    }

    void credits_sub( size_t value, Guild guild ) {
        this -> credits_to(
            this -> credits( guild ) - value,
            guild
        );
    }

public:
    void voice_hi_to( const std :: string_view& sound ) {
        File :: overwrite(
            USERS_PATH_MASTER + '\\' + this -> _id,
            USERS_PATH_VOICE_HI,
            sound
        );
    }

    std :: string voice_hi() {
        return File :: read< std :: string >(
            USERS_PATH_MASTER + '\\' + this -> _id,
            USERS_PATH_VOICE_HI,
            "hello_1"
        );
    }

    void voice_bye_to( const std :: string_view& sound ) {
        File :: overwrite(
            USERS_PATH_MASTER + '\\' + this -> _id,
            USERS_PATH_VOICE_BYE,
            sound
        );
    }

    std :: string voice_bye() {
        return File :: read< std :: string >(
            USERS_PATH_MASTER + '\\' + this -> _id,
            USERS_PATH_VOICE_BYE,
            "bye_great"
        );
    }

};



struct Embed {
public:
    const std :: string_view&   title         = {};
    const std :: string_view&   description   = {};
    const std :: string_view&   color         = {};
    const std :: string_view&   image         = {};

public:
    void outbound() {
        std :: cout
            << OUTBOUND_REPLY_EMBED
            << ( title.empty() ? " " : title )

            << OUTBOUND_LOW_SPLIT
            << ( description.empty() ? " " : description )

            << OUTBOUND_LOW_SPLIT
            << ( color.empty() ? "000000" : color )

            << OUTBOUND_LOW_SPLIT
            << ( IMAGES_EMBEDS_PATH_MASTER + '\\' + ( image.empty() ? "empty.png" : image ) );
    }

};



class Command {
public:
    using Keyword = std :: tuple< int, std :: vector< std :: string_view > >;

    using Function = std :: function< void( Guild, User, Ref< Inbounds > ) >;

    using Map = std :: map< size_t, Function >;

public:
    static void execute( Guild guild, User user, Ref< Inbounds > ins ) {
        auto guild_prefix = guild.prefix();

        if( !ins.at( 0 ).starts_with( guild_prefix ) ) return;

        ins.at( 0 ) = ins.at( 0 ).substr( guild_prefix.size() );

        if( ins.at( 0 ).empty() )
            ins.pop_front();


        try {
            std :: invoke( map.at( make_sense_of( ins ) ), guild, user, ins );

        } catch( std :: out_of_range& err ) {
            what( guild, user, ins );

        } catch( std :: runtime_error& err ) {
            std :: cout
                << OUTBOUND_REPLY_MESSAGE
                << "Something went terribly wrong.";

            std :: cerr << '\n' << err.what();
        } catch( ... ) {
            std :: cout
                << OUTBOUND_REPLY_MESSAGE
                << "If this message is sent, it is bad.";
        }
    }

    static size_t make_sense_of( Ref< Inbounds > ins ) {
        std :: string             chain{};
        std :: vector< Keyword* > kws{};


        for( auto itr = ins.begin(); itr != ins.end(); ) {
            auto found = std :: find_if(
                keywords.begin(), keywords.end(),

                [ & ] ( auto& entry ) -> bool {
                    for( auto& alias : std :: get< 1 >( entry ) )
                        if( *itr == alias )
                            return true;

                    return false;
                }
            );

            if(
                found == keywords.end()
                ||
                (
                    std :: find_if( kws.begin(), kws.end(), [ & ] ( auto& kw ) -> bool {
                        return &*found == &*kw;
                    } ) != kws.end()
                )
            ) {
                ++itr;

                continue;
            }

            kws.push_back( &*found );

            itr = ins.erase( itr );
        }


        std :: sort( kws.begin(), kws.end(), [] ( const auto& kw_1, const auto& kw_2 ) -> bool {
            return std :: get< 0 >( *kw_1 ) < std :: get< 0 >( *kw_2 );
        } );


        for( auto& kw : kws )
            chain += std :: get< 1 >( *kw )[ 0 ];


        return std :: hash< std :: string_view >{}( chain );
    }

public:

#pragma region Branches

public:
    GUI_OP( what ) {
        if( !ins.empty() ) {
            auto try_play = [ & ] ( const std :: string& sound ) -> bool {
                if( !Sound :: exists( sound ) )
                    return false;

                ins.push_front( std :: move( sound ) );

                Command :: voice_play( guild, user, ins );

                return true;
            };

            std :: string sound = std :: move( ins.at( 0 ) );

            if( try_play( sound ) )
                return;

            ins.pop_front();

            for( auto& sound_part : ins ) {
                sound += '_';
                sound += sound_part;

                if( try_play( sound ) )
                    return;
            }
        }


        std :: cout
            << OUTBOUND_REPLY_MESSAGE
            << "Whaaaat are you sayinnnnn...";
    }

public:
    GUI_OP( hash ) {
        if( !ins.at( 0 ).starts_with( '\"' )
            ||
            !ins.at( 0 ).ends_with( '\"' )
        ) {
            std :: cout
                << OUTBOUND_REPLY_MESSAGE
                << "Enclose the string in double quotes first.";

            return;
        }

        ins.at( 0 ) = ins.at( 0 ).substr( 1, ins.at( 0 ).size() - 2 );

        std :: cout
            << OUTBOUND_REPLY_MESSAGE
            << std :: hash< std :: remove_reference_t< decltype( ins.at( 0 ) ) > >{}( ins.at( 0 ) );
    };

public:
    GUI_OP( kiss ) {
        std :: cout
            << OUTBOUND_REPLY_MESSAGE
            << "https://tenor.com/view/heart-ahri-love-gif-18791933";


        ins.emplace_front( "kiss_1" );

        Command :: voice_play( guild, user, ins );
    }

    GUI_OP( pet ) {
        std :: cout
            << OUTBOUND_REPLY_MESSAGE
            << "https://tenor.com/view/ahri-league-of-legends-headpats-pats-cute-gif-22621824";
    }

public:
    GUI_OP( voice_connect ) {
        std :: cout
            << OUTBOUND_VOICE_CONNECT;
    }

    GUI_OP( voice_disconnect ) {
        std :: cout
            << OUTBOUND_VOICE_DISCONNECT;
    }

    GUI_OP( voice_play ) {
        std :: cout
            << OUTBOUND_VOICE_PLAY
            << guild.id()
            << OUTBOUND_LOW_SPLIT
            << Sound :: path_of( ins.at( 0 ) );
    }

    GUI_OP( voice_stop ) {
        std :: cout
            << OUTBOUND_VOICE_STOP
            << guild.id();
    }

    GUI_OP( voice_sounds_show ) {
        std :: string path{};

        path.reserve( PATH_MAX );

        std :: string accumulated{};

        for( auto& file : std :: filesystem :: directory_iterator( SOUNDS_PATH_MASTER ) ) {
            path = file.path().string();

            size_t slash_end = path.find_last_of( '\\' ) + 1;

            accumulated += "\n\n**\"";
            accumulated += path.substr( slash_end, path.size() - slash_end - 4 );
            accumulated += "\"**";
        }

        Embed{
            title: "DJ Ahri spinnin' these bad boys:",

            description:
                accumulated
                +=
                "\n\nYou may write the sounds with space instead of underscore.",

            color: EMBEDS_COLOR_INFO,

            image: "vinyl_purple.png"
        }.outbound();
    }

public:
    GUI_OP( settings_voice_wait_set ) {
        try {
            Settings :: voice_hi_wait_to( std :: abs( std :: stod( ins.at( 0 ) ) ) * 1000.0 );

            auto value = static_cast< double >( Settings :: voice_hi_wait() );

            Embed{
                title:
                    Stream{}
                    << "Now waiting **"
                    << value / 1000.0
                    << "** seconds before saying hi!",

                description: ( value >= 5000 ? "\nI could take a bath in the meantime tho..." : "" ),

                color: "00FF00"

            }.outbound();

        } catch( const std :: invalid_argument& err ) {
            Embed{
                title: "Try again after looking at this: ",

                description: "https://www.skillsyouneed.com/num/numbers.html",

                color: "FF0000"

            }.outbound();

        } catch( const std :: out_of_range& err ) {
            Embed{
                title: "I can't count that much...",

                color: "FF0000"

            }.outbound();

        }
    }

    GUI_OP( settings_voice_wait_show ) {
            Embed{
                title:
                    Stream{}
                    << "Waiting **"
                    << static_cast< double >( Settings :: voice_hi_wait() ) / 1000.0
                    << "** seconds before saying hi!",

                color: EMBEDS_COLOR_INFO
            }.outbound();
        }

public:
    GUI_OP( guild_prefix_set ) {
        auto last = guild.prefix();

        guild.prefix_to( ins.at( 0 ) );

        Embed{
            title:
                Stream{}
                << "This guild's prefix is now \"" << guild.prefix() << "\".",

            description:
                Stream{}
                << "Changed it from \"" << last << "\".",

            color: "00FF00"
        }.outbound();
    }

    GUI_OP( guild_prefix_show ) {
            Embed{
                title:
                    Stream{}
                    << "This guild's prefix is \"" << guild.prefix() << "\".",

                color: EMBEDS_COLOR_INFO
            }.outbound();
        }

    GUI_OP( guild_voice_auto_plays_add ) {
        enum Payload_info {
            PROBS_CALIBD
        };


        Stream embed_desc_acc{};

        auto payload
        =
        ins.for_each< std :: string, double >(
            {
                [] ( const std :: string& in ) -> std :: string {
                    return in;
                },

                [] ( std :: string& match ) -> bool {
                    return Sound :: exists( match );
                }
            },

            {
                [] ( const std :: string& in ) -> double {
                    return std :: stod( in );
                },

                [] ( double& match ) -> bool {
                    bool in_range = match >= 0.0 && match <= 1.0;

                    return in_range;
                }
            },

            [ & ] ( std :: string& sound, double& prob, auto& payload ) -> void {
                auto pairs = guild.voice_auto_plays();

                std :: erase_if( pairs, [ & ] ( auto& pair ) -> bool {
                    pair.first == sound;
                } );


                payload[ PROBS_CALIBD ]
                =
                payload[ PROBS_CALIBD ] | guild.voice_auto_plays_calibrate( prob, pairs );


                pairs.emplace_back( sound, prob );

                guild.voice_auto_plays_to( pairs );

                embed_desc_acc << "\n" << sound << "  |  " << prob;
            }
        );


        if( payload.done_count != 0 ) goto L_OK;
        if( payload.missing_at == 0 ) goto L_NO_SOUND;
        if( payload.missing_at == 1 ) goto L_NO_PROB;


        L_OK: {
            if( payload[ PROBS_CALIBD ] )
                embed_desc_acc
                << "\n\n"
                << "Probabilities did not account to 1, therefore they have been calibrated.";

            Embed{
                title:
                    Stream{}
                    << "Pushed "
                    << payload.done_count
                    << " sound"
                    << ( payload.done_count == 1 ? "" : "s" )
                    << " to the guild's auto plays.",

                description: embed_desc_acc,

                color: EMBEDS_COLOR_INFO
            }.outbound();

            return;
        }

        L_NO_SOUND: {
            std :: cout
                << OUTBOUND_REPLY_MESSAGE
                << "Double-check the sound cutey.";

            return;
        }

        L_NO_PROB: {
            std :: cout
                << OUTBOUND_REPLY_MESSAGE
                << "Need a probability between 0 and 1.";

            return;
        }
    }

    GUI_OP( guild_voice_auto_plays_clear ) {
        File :: overwrite(
            GUILDS_PATH_MASTER + '\\' + guild.id(),
            GUILDS_PATH_AUTO_VOICE_PLAYS,
            ""
        );

        Embed{
            title: "Cleared this guild's voice auto plays.",

            color: EMBEDS_COLOR_INFO
        }.outbound();
    }

    GUI_OP( guild_voice_auto_plays_show ) {
        auto pairs = guild.voice_auto_plays();

        if( pairs.empty() ) {
            Embed{
                title: "This guild has no voice auto plays.",

                color: EMBEDS_COLOR_INFO
            }.outbound();

            return;
        }


        std :: stringstream accumulated;

        for( auto& pair : pairs )
            accumulated << pair.first << " ---** " << pair.second << "**\n";

        Embed{
            title: "These are this guild's voice auto plays:",

            description: accumulated.str(),

            color: EMBEDS_COLOR_INFO
        }.outbound();
    }

public:
    GUI_OP( user_credits_show ) {
        Embed{
            title:
                Stream{}
                << "You have **"
                << user.credits( guild )
                << "** credits cutey!",

            description:
                Stream{}
                << "Guild multiplier is **"
                << guild.multiplier()
                << "**.",

            color: "FFD700",

            image: "credits.png"

        }.outbound();
    };

    GUI_OP( user_voice_hi_set ) {
        std :: string_view name = ins.at( 0 );

        if( !Sound :: exists( name ) ) {
            Embed{
                title: "There's no such sound...",

                color: "FF0000"
            }.outbound();

            return;
        }

        user.voice_hi_to( name );

        Embed{
            title:
                Stream{}
                << "Now I'm greeting you with \"" << name << "\".",

            color: EMBEDS_COLOR_INFO
        }.outbound();
    }

    GUI_OP( user_voice_bye_set ) {
            std :: string_view name = ins.at( 0 );

            if( !Sound :: exists( name ) ) {
                Embed{
                    title: "There's no such sound...",

                    color: "FF0000"
                }.outbound();

                return;
            }

            user.voice_bye_to( name );

            Embed{
                title:
                    Stream{}
                    << "Now I'm parting you with \"" << name << "\".",

                color: EMBEDS_COLOR_INFO
            }.outbound();
        }

public:
    class Gamble {
    public:
        enum Color : int {
            RED = 1, BLACK = 2, GREEN = 4
        };

        static constexpr std :: array< const char*, 3 > colors{ "red", "black", "green" };

    public:
        GUI_OP( main ) {
            const size_t credits = user.credits( guild );

            auto payload
            =
            ins.for_each< int, int64_t >(
                {
                    [] ( const std :: string& in ) -> int {
                        for( size_t idx = 0; idx <= 2; ++idx )
                            if( in == colors[ idx ] )
                                return std :: pow( 2, idx );

                        return 0;
                    },

                    [] ( int& match ) -> bool {
                        return match != 0;
                    }
                },

                {
                    [] ( const std :: string& in ) -> int64_t {
                        return std :: stoll( in );
                    },

                    [ & ] ( int64_t& match ) -> bool {
                        return match <= credits && match >= 0;
                    }
                },

                [ & ] (
                    int& gambled_color,
                    int64_t& gambled_ammount,
                    Inbounds :: FE_payload& payload
                ) -> void {
                    const auto land   = _roulette_spin( gambled_color );
                    int64_t    acc = 0;

                    if( gambled_color == _roulette_land_to_color( land ) )
                        acc = gambled_ammount * ( land == 0 ? 13 : 1 );
                    else
                        acc = -gambled_ammount;


                    user.credits_to( credits + acc, guild );


                    Embed{
                        title:
                            Stream{}
                            << "Landed on "
                            << land
                            << ".",

                        description:
                            Stream{}
                            << acc
                            << " credits for you sweetie.",

                        color: ( [ & ] () -> const char* {
                            switch( gambled_color ) {
                                case RED: return "FF0000";
                                case BLACK: return "000000";
                                case GREEN: return "00FF00";
                            }
                        } )(),

                        image: ( [ & ] () -> const char* {
                            if( land == 0 )
                                return "green.png";

                            return land % 2 == 0 ? "red.png" : "black.png";
                        } )()
                    }.outbound();
                }
            );

            if( payload.done_count > 0 )
                return;

            switch( payload.missing_at ) {
                case 0: {
                    std :: cout
                    << OUTBOUND_REPLY_MESSAGE
                    << "What color are you gambling on?";

                    return;
                }

                case 1: {
                    std :: cout
                    << OUTBOUND_REPLY_MESSAGE
                    << "Might wanna double-check how much are you gambling.";

                    return;
                }
            }
        }

        GUI_OP( rig_set ) {
            double rig_value = -1.0;

            for( auto& in : ins ) {
                try {
                    double entry = std :: stod( in );

                    if( !_rig_in_range( entry ) ) continue;

                    rig_value = entry;

                } catch( ... ) {
                    continue;
                }
            }

            if( rig_value == -1.0 ) {
                std :: cout
                    << OUTBOUND_REPLY_MESSAGE
                    << "I need something between 0.0 and 1.0...";

                return;
            }


            Settings :: Gamble :: rig_to( rig_value );


            Embed{
                title:
                    Stream{} << "Gamble rig value is now **" << rig_value << "**",

                description:
                    Stream{} << ( rig_value >= 0.5 ? "Hehe" : "" ),

                color: EMBEDS_COLOR_INFO

            }.outbound();
        }

    private:
        static int _roulette_spin( int gambled_color ) {
            int land = static_cast< int >( RAND % 32 );

            if( _roulette_land_to_color( land ) == gambled_color )
                if( RAND % 100 < Settings :: Gamble :: rig() * 100 )
                    land = std :: clamp( land + 1, 0, 31 );

            return land;
        }

        static int _roulette_land_to_color( int land ) {
            if( land == 0 )
                return GREEN;

            return land % 2 == 0 ? RED : BLACK;
        }

    private:
        static bool _rig_in_range( double rig_value ) {
            return rig_value >= 0.0 && rig_value <= 1.0;
        }

    };

#pragma endregion Branches

public:
    inline static std :: vector< Keyword > keywords = {
        { 1, { "credits", "money", "balance", "coins" } },
        { 2, { "hash" } },
        { 3, { "kiss" } },
        { 4, { "pet" } },
        { 5, { "voice", "connect" } },
        { 6, { "leave", "disconnect" } },
        { 7, { "play" } },
        { 8, { "set", "change", "=", "make" } },
        { 9, { "prefix" } },
        { 10, { "hi", "hello", "greet" } },
        { 11, { "bye", "cya" } },
        { 12, { "wait", "hold" } },
        { 13, { "sounds", "soundboard" } },
        { 14, { "gamble", "bet" } },
        { 15, { "rig" } },
        { 16, { "push", "add", "+" } },
        { 17, { "pop", "sub", "-", "erase", "remove" } },
        { 18, { "auto" } },
        { 19, { "stop" } },
        { 20, { "clear" } }
    };

    inline static Map map = {
        { 6839924347221205416ULL, hash },

        { 7728093003851250935ULL, kiss },
        { 8501243811175406933ULL, pet },

        { 7492372067882396056ULL,  voice_connect },
        { 3435728378537700265ULL,  voice_disconnect },
        { 9340956479027659370ULL,  voice_play },
        { 15925390277482132049ULL, voice_stop },
        { 15169021593429937846ULL, voice_sounds_show },

        { 6865420363795655716ULL, settings_voice_wait_set },
        { 6685002744963886194ULL, settings_voice_wait_show },

        { 1062816732498115940ULL,  guild_prefix_set },
        { 2016015562653219627ULL,  guild_prefix_show },
        { 14632587987046264091ULL, guild_voice_auto_plays_add },
        { 8901744138937185971ULL,  guild_voice_auto_plays_clear },
        { 11115212484132745176ULL, guild_voice_auto_plays_show },

        { 4330606938181995941ULL,  user_credits_show },
        { 12382791774924742628ULL, user_voice_hi_set },
        { 183303123199750495ULL,   user_voice_bye_set },

        { 15754336788579780731ULL, Gamble :: main },
        { 11779848440330006868ULL, Gamble :: rig_set }
    };

};



class Message {
public:
    GUI_OP( on_create ) {
        user.credits_add( 20, guild );


        Command :: execute( guild, user, ins );
    }
};


class Voice {
public:
    GUI_OP( on_update ) {
        auto& old_ch = ins.at( 0 );
        auto& new_ch = ins.at( 1 );

        if( old_ch == new_ch ) 
            return;


        bool connected = ( new_ch == ins.voice_id() );

        ins.emplace_front( connected ? user.voice_hi() : user.voice_bye() );

        if( connected )
            std :: this_thread :: sleep_for(
                std :: chrono :: milliseconds( Settings :: voice_hi_wait() )
            );

        Command :: voice_play( guild, {}, ins );
    }

};



class Tick {
public:
    GUI_OP( on_tick ) {
        std :: cout
            << OUTBOUND_TICK_GUILD_SET
            << RAND % 1200 + 600;


        voice_auto_play( guild, ins );
    }

public:
    static void voice_auto_play( Guild guild, Ref< Inbounds > ins ) {
        constexpr int precision = 10000;
        double        gauge     = RAND % precision;
        double        sum       = 0.0;

        auto          pairs     = guild.voice_auto_plays();
        auto          itr       = pairs.begin();

        for( ; itr != pairs.end(); ++itr ) {
            sum += itr -> second * precision;

            if( gauge <= sum ) break;
        }

        if( itr == pairs.end() )
            return;

        ins.push_front( itr -> first );

        Command :: voice_play( guild, {}, ins );
    }

};



std :: map< 
    std :: string_view, 
    std :: function< void( Guild, User, Ref< Inbounds > ) > 
>  event_map = {
    { INBOUND_MESSAGE,      Message :: on_create },
    { INBOUND_VOICE_UPDATE, Voice :: on_update },
    { INBOUND_TICK,         Tick :: on_tick }
};



int main( int arg_count, char* args[] ) {    
    srand(
        static_cast< unsigned int >(
            std :: chrono :: duration_cast< std :: chrono :: milliseconds >(
                std :: chrono :: high_resolution_clock :: now().time_since_epoch()
            ).count()
        )
     );


    Inbounds ins  { arg_count, args };
    Guild    guild{ ins.guild_id() };
    User     user { ins.user_id() }; 


    event_map.at( ins.event() )( guild, user, ins );
    

    return 0;
}
