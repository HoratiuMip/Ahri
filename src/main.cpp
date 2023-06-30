#include <iostream>
#include <fstream>
#include <filesystem>

#include <chrono>
#include <thread>

#include <string>
#include <string_view>

#include <functional>

#include <map>
#include <list>
#include <deque>
#include <vector>
#include <set>

#include <random>

#include <algorithm>
#include <utility>



using namespace std :: string_literals;



template< typename T > 
using Ref = T&;

using Inbounds = std :: deque< std :: string_view >;

using Event_map = std :: map< std :: string_view, std :: function< void( Ref< Inbounds > ) > >;


using ID = std :: string_view;



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

#define GUILDS_PATH_MASTER ".\\Data\\Guilds"s
#define GUILDS_PATH_PREFIX "prefix.ahr"s


#define USERS_PATH_MASTER ".\\Data\\Users"s
#define USERS_PATH_CREDITS "credits.ahr"s
#define USERS_PATH_GUILD "Guilds"s


#define SOUNDS_PATH_MASTER ".\\Data\\Audio"s



std :: string operator + ( 
    const std :: string& string, 
    const std :: string_view& string_view 
) {
    return string + string_view.data();
}



std :: random_device random = {};



class Has_id {
public:
    Has_id() = default;

    Has_id( ID id )
        : _id{ id }
    {}

protected:
    ID   _id   = {};

public:
    ID id() const {
        return _id;
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
    void prefix_set( std :: string_view prefix ) const {
        auto dir = GUILDS_PATH_MASTER
                   + '\\' + this -> _id;

        auto path = dir + '\\' + GUILDS_PATH_PREFIX;

        std :: ofstream file{ path };

        if( !file ) {
            std :: filesystem :: create_directories( dir );

            file.open( path );
        }

        file << prefix;
    }

    std :: string prefix_get() const {
        std :: ifstream file{
            GUILDS_PATH_MASTER
            + '\\' + this -> _id
            + '\\' + GUILDS_PATH_PREFIX
        };

        if( !file ) {
            this -> prefix_set( GUILDS_DEFAULT_PREFIX );

            return GUILDS_DEFAULT_PREFIX;
        }

        std :: string prefix{};

        file >> prefix;

        return prefix;
    }

};



class User : public Has_id {
public:
    User() = default;

    using Has_id :: Has_id;

public:
    void credits_set( size_t value, Guild guild ) {
        auto dir = USERS_PATH_MASTER 
                   + '\\' + this -> _id 
                   + '\\' + USERS_PATH_GUILD 
                   + '\\' + guild.id();

        auto path = dir + '\\' + USERS_PATH_CREDITS;

        std :: ofstream file{ path };

        if( !file ) {
            std :: filesystem :: create_directories( dir );

            file.open( path );
        }

        file << value;
    }

    size_t credits_get( Guild guild ) {
        std :: ifstream file { 
            USERS_PATH_MASTER 
            + '\\' + this -> _id 
            + '\\' + USERS_PATH_GUILD 
            + '\\' + guild.id() 
            + '\\' + USERS_PATH_CREDITS 
        };

        if( !file ) {
            this -> credits_set( 0, guild );

            return 0;
        }

        size_t credits{};

        file >> credits;

        return credits;
    }

    void credits_add( size_t value, Guild guild ) {
        this -> credits_set( 
            ( this -> credits_get( guild ) + value ) * guild.multiplier(), 
            guild 
        );
    }

    void credits_sub( size_t value, Guild guild ) {
        this -> credits_set( 
            this -> credits_get( guild ) - value, 
            guild 
        );
    }

};



#define CMD_METHOD( name ) inline static void name( Guild guild, User user, Ref< Inbounds > ins )

class Command {
public:
    using Keyword = std :: tuple< int, std :: vector< std :: string_view > >;

    using Function = std :: function< void( Guild, User, Ref< Inbounds > ) >;

    using Map = std :: map< size_t, Function >;

public:
    static void execute( Ref< Inbounds > ins ) {
        Guild guild{ ins.at( 0 ) }; 
        ins.pop_front();

        User user{ ins.at( 0 ) }; 
        ins.pop_front();


        user.credits_add( 20, guild );


        if( !ins.at( 0 ).starts_with( guild.prefix_get() ) ) return;

        ins.at( 0 ) = ins.at( 0 ).substr( 1 );

        if( ins.at( 0 ).empty() )
            ins.pop_front();

        
        try {
            std :: invoke( map.at( make_sense_of( ins ) ), guild, user, ins );

        } catch( std :: out_of_range& err ) { 
            Command :: what( guild, user, ins );
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
    CMD_METHOD( what ) {
        std :: cout
            << OUTBOUND_MESSAGE_REPLY_S
            << "Whaaaat are you sayinnnnn";
    }

    CMD_METHOD( credits ) {
        std :: cout
            << OUTBOUND_MESSAGE_REPLY_S
            << user.credits_get( guild );
    };

    CMD_METHOD( hash ) {
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

    CMD_METHOD( kiss ) {
        std :: cout
            << OUTBOUND_MESSAGE_REPLY_S
            << "https://tenor.com/view/heart-ahri-love-gif-18791933";
    }

    CMD_METHOD( pet ) {
        std :: cout 
            << OUTBOUND_MESSAGE_REPLY_S
            << "https://tenor.com/view/ahri-league-of-legends-headpats-pats-cute-gif-22621824";
    }

    CMD_METHOD( voice ) {
        std :: cout 
            << OUTBOUND_VOICE_CONNECT;
    }

    CMD_METHOD( leave ) {
        std :: cout
            << OUTBOUND_VOICE_DISCONNECT;
    }

    CMD_METHOD( play ) {
        std :: cout
            << OUTBOUND_VOICE_PLAY_S
            << SOUNDS_PATH_MASTER
            << '\\' << ins.at( 0 ) << ".mp3";
    }

public:
    inline static std :: vector< Keyword > keywords = {
        { 1, { "credits", "money", "balance", "coins" } },
        { 2, { "hash" } },
        { 3, { "kiss" } },
        { 4, { "pet" } },
        { 5, { "voice" } },
        { 6, { "leave" } },
        { 7, { "play" } }
    };

    inline static Map map = {
        { 4330606938181995941ULL, Command :: credits },
        { 6839924347221205416ULL, Command :: hash },
        { 7728093003851250935ULL, Command :: kiss },
        { 8501243811175406933ULL, Command :: pet },
        { 7492372067882396056ULL, Command :: voice },
        { 3435728378537700265ULL, Command :: leave },
        { 9340956479027659370ULL, Command :: play }
    };

};



class Message {
public:
    static void on( Ref< Inbounds > ins ) {
        ins.pop_front();

        Command :: execute( ins );
    }
};



class Core {
public:
    static void on( Ref< Inbounds > ins ) {
        ins.pop_front();

        if( ins.front() == "hash" ) {
            ins.pop_front();

            std :: cout << std :: hash< std :: string_view >{}( ins.front() );
        }
    }

};



Event_map event_map = {
    { "core", Core :: on },
    { "message",  Message :: on }
};



int main( int arg_count, char* args[] ) {
    Inbounds inbounds{};

    for( int idx = 1; idx < arg_count; ++idx )
        inbounds.push_back( args[ idx ] );


    event_map.at( inbounds.front() )( inbounds );


    return 0;
}