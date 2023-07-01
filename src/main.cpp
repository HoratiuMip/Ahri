#include "Source\\std_includes.cpp"
#include "Source\\quints.cpp"
#include "Source\\utility.cpp"



std :: random_device random = {};



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

        } catch( ... ) {
            std :: cout
                << OUTBOUND_MESSAGE_REPLY_S
                << "Something went terribly wrong.";
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
            << OUTBOUND_MESSAGE_REPLY_S
            << "Whaaaat are you sayinnnnn";
    }

public:
    COMMAND_BRANCH( user_credits_show ) {
        std :: cout
            << OUTBOUND_MESSAGE_REPLY_S
            << user.credits( guild );
    };

public:
    COMMAND_BRANCH( hash ) {
        if( !ins.at( 0 ).starts_with( '\"' )
            ||
            !ins.at( 0 ).ends_with( '\"' ) 
        ) {
            std :: cout 
                << OUTBOUND_MESSAGE_REPLY_S
                << "Enclose the string in double quotes first.";

            return;
        }

        ins.at( 0 ) = ins.at( 0 ).substr( 1, ins.at( 0 ).size() - 2 );

        std :: cout 
            << OUTBOUND_MESSAGE_REPLY_S
            << std :: hash< std :: remove_reference_t< decltype( ins.at( 0 ) ) > >{}( ins.at( 0 ) );
    };

public:
    COMMAND_BRANCH( kiss ) {
        std :: cout
            << OUTBOUND_MESSAGE_REPLY_S
            << "https://tenor.com/view/heart-ahri-love-gif-18791933";
    }

    COMMAND_BRANCH( pet ) {
        std :: cout 
            << OUTBOUND_MESSAGE_REPLY_S
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
            << OUTBOUND_VOICE_PLAY_S
            << Sound :: path_of( ins.at( 0 ) );
    }

public:
    COMMAND_BRANCH( guild_prefix_set ) {
        guild.prefix_to( ins.at( 0 ) );

        std :: cout
            << OUTBOUND_MESSAGE_REPLY_S
            << "Guild prefix is now \"" << guild.prefix() << "\"."; 
    }

    COMMAND_BRANCH( guild_prefix_show ) {
        std :: cout
            << OUTBOUND_MESSAGE_REPLY_S
            << "Guild prefix is \"" << guild.prefix() << "\".";
    }

public:
    COMMAND_BRANCH( user_voice_hi_set ) {
        std :: string_view name = ins.at( 0 );

        if( !Sound :: exists( name ) ) {
            std :: cout
                << OUTBOUND_MESSAGE_REPLY_S
                << "There's no such sound...";

            return;
        }

        user.voice_hi_to( name );

        std :: cout 
            << OUTBOUND_MESSAGE_REPLY_S
            << "All done.";
    }

    COMMAND_BRANCH( user_voice_bye_set ) {
        std :: string_view name = ins.at( 0 );

        if( !Sound :: exists( name ) ) {
            std :: cout
                << OUTBOUND_MESSAGE_REPLY_S
                << "There's no such sound...";

            return;
        }

        user.voice_bye_to( name );

        std :: cout 
            << OUTBOUND_MESSAGE_REPLY_S
            << "All done.";
    }

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
        { 11, { "bye", "cya" } }
    };

    inline static Map map = {
        { 4330606938181995941ULL, Command :: user_credits_show },

        { 6839924347221205416ULL, Command :: hash },

        { 7728093003851250935ULL, Command :: kiss },
        { 8501243811175406933ULL, Command :: pet },

        { 7492372067882396056ULL, Command :: voice_connect },
        { 3435728378537700265ULL, Command :: voice_disconnect },
        { 9340956479027659370ULL, Command :: voice_play },

        { 1062816732498115940ULL, Command :: guild_prefix_set },
        { 2016015562653219627ULL, Command :: guild_prefix_show },

        { 12382791774924742628ULL, Command :: user_voice_hi_set },
        { 183303123199750495ULL,   Command :: user_voice_bye_set }
    };

};



class Message {
public:
    static void on_create( Ref< Inbounds > ins ) {
        ins.pop_front();

        Guild guild{ ins.at( 0 ) };
        ins.pop_front();

        User user{ ins.at( 0 ) };
        ins.pop_front();


        user.credits_add( 20, guild );


        Command :: execute( guild, user, ins );
    }
};


class Voice {
public:
    static void on_update( Ref< Inbounds > ins ) {
        ins.pop_front();

        Guild guild{ ins.at( 0 ) };
        ins.pop_front();

        User user{ ins.at( 0 ) };
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
                    std :: this_thread :: sleep_for( std :: chrono :: milliseconds( 1500 ) );

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