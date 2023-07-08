#include "std_includes.cpp"
#include "quints.cpp"
#include "utility.cpp"



std :: random_device random = {};



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
            << OUTBOUND_REPLY_EMBED_L_S
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
            Command :: what( guild, user, ins );

        } catch( std :: runtime_error& err ) {
            std :: cout
                << OUTBOUND_REPLY_MESSAGE_L_S
                << "Something went terribly wrong.";

            std :: cerr << '\n' << err.what();
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
    COMMAND_BRANCH( what ) {
        if( Sound :: exists( ins.at( 0 ) ) ) {
            Command :: voice_play( guild, user, ins );

            return;
        }


        std :: cout
            << OUTBOUND_REPLY_MESSAGE_L_S
            << "Whaaaat are you sayinnnnn";
    }

public:
    COMMAND_BRANCH( user_credits_show ) {
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

public:
    COMMAND_BRANCH( hash ) {
        if( !ins.at( 0 ).starts_with( '\"' )
            ||
            !ins.at( 0 ).ends_with( '\"' ) 
        ) {
            std :: cout 
                << OUTBOUND_REPLY_MESSAGE_L_S
                << "Enclose the string in double quotes first.";

            return;
        }

        ins.at( 0 ) = ins.at( 0 ).substr( 1, ins.at( 0 ).size() - 2 );

        std :: cout 
            << OUTBOUND_REPLY_MESSAGE_L_S
            << std :: hash< std :: remove_reference_t< decltype( ins.at( 0 ) ) > >{}( ins.at( 0 ) );
    };

public:
    COMMAND_BRANCH( kiss ) {
        std :: cout
            << OUTBOUND_REPLY_MESSAGE_L_S
            << "https://tenor.com/view/heart-ahri-love-gif-18791933";

        std :: cout << OUTBOUND_HIGH_SPLIT;

        ins.emplace_front( "kiss_1" );

        Command :: voice_play( guild, user, ins );
    }

    COMMAND_BRANCH( pet ) {
        std :: cout 
            << OUTBOUND_REPLY_MESSAGE_L_S
            << "https://tenor.com/view/ahri-league-of-legends-headpats-pats-cute-gif-22621824";
    }

public:
    COMMAND_BRANCH( voice_connect ) {
        std :: cout 
            << OUTBOUND_VOICE_CONNECT;
    }

    COMMAND_BRANCH( voice_disconnect ) {
        std :: cout
            << OUTBOUND_VOICE_DISCONNECT;
    }

    COMMAND_BRANCH( voice_play ) { 
        std :: cout
            << OUTBOUND_VOICE_PLAY_L_S
            << Sound :: path_of( ins.at( 0 ) );
    }

public:
    COMMAND_BRANCH( sounds_show ) {
        std :: string path{};

        path.reserve( PATH_MAX );

        std :: string accumulated{};

        for( auto& file : std :: filesystem :: directory_iterator( SOUNDS_PATH_MASTER ) ) {
            path = file.path().string();

            size_t slash_end = path.find_last_of( '\\' ) + 1;
 
            accumulated += "\n**\"";
            accumulated += path.substr( slash_end, path.size() - slash_end - 4 );
            accumulated += "\"**";
        }

        Embed{
            title: "These are all the sounds I got:",

            description: accumulated,

            color: "5D3FD3",

            image: "vinyl_purple.png"
        }.outbound();
    }

public:
    COMMAND_BRANCH( settings_voice_wait_set ) {
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

    COMMAND_BRANCH( settings_voice_wait_show ) {
        Embed{
            title:
                Stream{}
                << "Waiting **"
                << static_cast< double >( Settings :: voice_hi_wait() ) / 1000.0
                << "** seconds before saying hi!",

            color: "5D3FD3"
        }.outbound();
    }

public:
    COMMAND_BRANCH( guild_prefix_set ) {
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

    COMMAND_BRANCH( guild_prefix_show ) {
        Embed{
            title:
                Stream{}
                << "This guild's prefix is \"" << guild.prefix() << "\".",

            color: "5D3FD3"
        }.outbound();
    }

public:
    COMMAND_BRANCH( user_voice_hi_set ) {
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

            color: "5D3FD3"
        }.outbound();
    }

    COMMAND_BRANCH( user_voice_bye_set ) {
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

            color: "5D3FD3"
        }.outbound();
    }

public:
    class Gamble {
    public:
        enum Color : int {
            RED = 1, BLACK = 2, GREEN = 4
        };

    public:
        COMMAND_BRANCH( main ) { 
            auto color = pull_color( ins );

            if( !color ) {
                std :: cout
                    << OUTBOUND_REPLY_MESSAGE_L_S
                    << "What color are you gambling on?";

                return;
            }


            auto ammount = pull_ammount( ins );

            if( !ammount ) {
                std :: cout
                    << OUTBOUND_REPLY_MESSAGE_L_S
                    << "How much are you gambling?";

                return;
            }

            if( ammount.value() < 0 ) {
                std :: cout
                    << OUTBOUND_REPLY_MESSAGE_L_S
                    << "Peak comedy...";

                return;
            }


            const size_t credits = user.credits( guild );

            if( ammount > credits ) {
                std :: cout
                    << OUTBOUND_REPLY_MESSAGE_L_S
                    << "Might wanna check your wallet...";

                return;
            }


            const auto land   = spin();
            int64_t    result = 0;

            if( color == land_to_color( land ) )
                result = ammount.value() * ( land == 0 ? 13 : 1 );
            else
                result = -ammount.value();

            
            user.credits_to( credits + result, guild );

            
            Embed{
                title:
                    Stream{}
                    << "Landed on "
                    << land
                    << ".",

                description:
                    Stream{}
                    << result
                    << " credits for you sweetie.",

                color: ( [ & ] () -> const char* {
                    switch( color.value() ) {
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

        static std :: optional< int > pull_color( Ref< Inbounds > ins ) {
            std :: bitset< 3 > colors{};

            for( size_t idx = 0; idx < 3; ++idx ) 
                colors[ idx ] = std :: ranges :: find(
                    ins,
                    ( [ & ] () -> std :: string_view {
                        switch( idx ) {
                            case 0: return "red";
                            case 1: return "black";
                            case 2: return "green";
                        }
                    } )()
                ) != ins.end();


            if( colors.count() != 1 )
                return {};

            return static_cast< int >( colors.to_ulong() );
        }
        
        static std :: optional< int64_t > pull_ammount( Ref< Inbounds > ins ) {
            std :: optional< int64_t > max{};

            for( auto& in : ins ) {
                try {
                    if( 
                        int64_t value = std :: stoll( in ); 
                        value > max.value_or( std :: numeric_limits< int64_t > :: min() ) 
                    )
                        max = value;

                } catch( ... ) {
                    continue;
                }
            }

            return max;
        }
    

        static int spin() {
            return static_cast< int >( random() % 32 );
        }

        static int land_to_color( int land ) {
            if( land == 0 )
                return GREEN;

            return land % 2 == 0 ? RED : BLACK;
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
        { 10, { "hi", "hello", "greet", "salut" } },
        { 11, { "bye", "cya" } },
        { 12, { "wait", "hold" } },
        { 13, { "sounds", "soundboard" } },
        { 14, { "gamble", "bet" } }
    };

    inline static Map map = {
        { 4330606938181995941ULL, Command :: user_credits_show },

        { 6839924347221205416ULL, Command :: hash },

        { 7728093003851250935ULL, Command :: kiss },
        { 8501243811175406933ULL, Command :: pet },

        { 7492372067882396056ULL, Command :: voice_connect },
        { 3435728378537700265ULL, Command :: voice_disconnect },
        { 9340956479027659370ULL, Command :: voice_play },

        { 15169021593429937846ULL, Command :: sounds_show },

        { 6865420363795655716ULL, Command :: settings_voice_wait_set },
        { 6685002744963886194ULL, Command :: settings_voice_wait_show },

        { 1062816732498115940ULL, Command :: guild_prefix_set },
        { 2016015562653219627ULL, Command :: guild_prefix_show },

        { 12382791774924742628ULL, Command :: user_voice_hi_set },
        { 183303123199750495ULL,   Command :: user_voice_bye_set },

        { 15754336788579780731ULL, Command :: Gamble :: main }
    };

};



class Message {
public:
    static void on_create( Ref< Inbounds > ins ) {
        ins.pop_front();

        Guild guild{ std :: move( ins.at( 0 ) ) };
        ins.pop_front();

        User user{ std :: move( ins.at( 0 ) ) };
        ins.pop_front();


        user.credits_add( 20, guild );

        
        Command :: execute( guild, user, ins );
    }
};


class Voice {
public:
    static void on_update( Ref< Inbounds > ins ) {
        ins.pop_front();

        Guild guild{ std :: move( ins.at( 0 ) ) };
        ins.pop_front();

        User user{ std :: move( ins.at( 0 ) ) };
        ins.pop_front();


        std :: bitset< 8 > flags{};

        enum What {
            CONNECTED = 1, 
            DISCONNECTED = 2
        };

        flags[ 0 ] = ins.at( 0 ).empty();
        flags[ 1 ] = ins.at( 1 ).empty();

        
        switch( auto what = flags.to_ulong() ) {
            case CONNECTED:
            case DISCONNECTED: {
                const bool connected = ( what == CONNECTED );

                ins.emplace_front( connected ? user.voice_hi() : user.voice_bye() );

                if( ins.front().empty() )
                    ins.front() = connected ? "hello_1" : "bye_great";

                if( connected )
                    std :: this_thread :: sleep_for( 
                        std :: chrono :: milliseconds( Settings :: voice_hi_wait() ) 
                    );

                Command :: voice_play( guild, user, ins );


                break; }
        }
    }

};



Event_map event_map = {
    { "message",  Message :: on_create },
    { "voice_update", Voice :: on_update }
};



int main( int arg_count, char* args[] ) {
    Inbounds ins = {};

    for( int idx = 1; idx < arg_count; ++idx )
        ins.emplace_back( args[ idx ] );


    event_map.at( ins.front() )( ins );

    
    return 0;
}