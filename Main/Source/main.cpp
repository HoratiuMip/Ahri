/*
    Ahri(ookie) - Processing Engine Section ( C++ )

    Hackerman: Mipsan



    Vectorul de treburi care trebuie facute:
    -keyword map hash order and priority oreder
    -cout<<sstream
    -kiss command check
*/



#pragma region INCLUDES

#include <iostream>
#include <fstream>
#include <filesystem>

#include <chrono>
#include <thread>

#include <string>
#include <string_view>
#include <sstream>

#include <functional>
#include <variant>
#include <optional>

#include <map>
#include <list>
#include <deque>
#include <vector>
#include <set>
#include <bitset>

#include <ranges>

#include <random>

#include <algorithm>
#include <utility>

#pragma endregion INCLUDES



#pragma region QUINTS

#define NULL_BOUND "NULL"


#define OUTBOUND_LOW_SPLIT "||__LOW_SPLIT__||"
#define OUTBOUND_HIGH_SPLIT "||__HIGH_SPLIT__||"


#define OUTBOUND_SCRIPT OUTBOUND_HIGH_SPLIT "script" OUTBOUND_LOW_SPLIT


#define OUTBOUND_REPLY_MESSAGE OUTBOUND_HIGH_SPLIT "reply_message" OUTBOUND_LOW_SPLIT

#define OUTBOUND_REPLY_EMBED OUTBOUND_HIGH_SPLIT "reply_embed" OUTBOUND_LOW_SPLIT


#define OUTBOUND_VOICE_CONNECT OUTBOUND_HIGH_SPLIT "voice_connect" OUTBOUND_LOW_SPLIT

#define OUTBOUND_VOICE_DISCONNECT OUTBOUND_HIGH_SPLIT "voice_disconnect" OUTBOUND_LOW_SPLIT

#define OUTBOUND_VOICE_PLAY OUTBOUND_HIGH_SPLIT "voice_play" OUTBOUND_LOW_SPLIT

#define OUTBOUND_VOICE_STOP OUTBOUND_HIGH_SPLIT "voice_stop" OUTBOUND_LOW_SPLIT


#define OUTBOUND_TICK_GUILD_SET OUTBOUND_HIGH_SPLIT "tick_guild_set" OUTBOUND_LOW_SPLIT

#define OUTBOUND_TICK_GUILD_RESET OUTBOUND_HIGH_SPLIT "tick_guild_reset" OUTBOUND_LOW_SPLIT

#define OUTBOUND_TICK_INIT "init"
#define OUTBOUND_TICK_VOICE "voice"
#define OUTBOUND_TICK_REMINDER "reminder"


#define OUTBOUND_PYTHON_RESPONSE OUTBOUND_HIGH_SPLIT "python_response" OUTBOUND_LOW_SPLIT



#define INBOUND_MESSAGE "message"
#define INBOUND_VOICE_UPDATE "voice_update"
#define INBOUND_TICK "tick"



#define SETTINGS_PATH_MASTER std::string{ ".\\Data\\Settings" }

#define SETTINGS_PATH_VOICE_HI_WAIT "voice_hi_wait.arh"



#define IMAGES_EMBEDS_PATH_MASTER std::string{ ".\\Data\\Images\\Embeds" }



#define EMBEDS_COLOR_INFO "5D3FD3"



#define GUILDS_DEFAULT_PREFIX "."

#define GUILDS_AHRI_PREFIX "ahri"

#define GUILDS_PATH_MASTER std::string{ ".\\Data\\Guilds" }
#define GUILDS_PATH_PREFIX "prefix.arh"
#define GUILDS_PATH_AUTO_VOICE_PLAYS "auto_voice_plays.arh"
#define GUILDS_PATH_GAMBLE_RIG "gamble_rig.arh"
#define GUILDS_PATH_STEAL_CHANCE "steal_chance.arh"
#define GUILDS_PATH_TICK_VOICE "tick_voice.arh"
#define GUILDS_PATH_MUL "mul.arh"



#define USERS_PATH_MASTER std::string{ ".\\Data\\Users" }
#define USERS_PATH_VOICE_HI "voice_hi.arh"
#define USERS_PATH_VOICE_BYE "voice_bye.arh"
#define USERS_PATH_GUILD_CREDITS "credits.arh"
#define USERS_PATH_REQUEST_DEBUG "request_debug.arh"
#define USERS_PATH_GUILD "Guilds"



#define SOUNDS_PATH_MASTER std::string{ ".\\Data\\Audio" }



#define PYTHON_SCRIPT_MAIN "chatbot_predictor.py"

#pragma endregion QUINTS



std::random_device random;
#define RANDOM_SEEDING_OP() \
    srand( \
        static_cast< unsigned int >( \
            std::chrono::duration_cast< std::chrono::milliseconds >( \
                std::chrono::high_resolution_clock::now().time_since_epoch() \
            ).count() \
        ) \
     );
#define RANDOM random()
#define RANDOM_MAX std::random_device::max()



using namespace std::string_literals;
using namespace std::string_view_literals;



#pragma region BOOSTERS

std::string operator + (
    const std::string& rhs,
    const std::string_view& lhs
) {
    return rhs + lhs.data();
}

std::string operator + (
    const std::string_view& rhs,
    const std::string_view& lhs
) {
    return std::string{ rhs.data() } + lhs.data();
}

std::string operator + (
    const char* rhs,
    const std::string_view& lhs
) {
    return std::string{ rhs } + lhs.data();
}

std::string operator + (
    const std::string_view& rhs,
    const char* lhs
) {
    return std::string{ rhs.data() } + lhs;
}

std::string operator + (
    const std::string_view& rhs,
    const char& lhs
) {
    return std::string{ rhs.data() } + lhs;
}


#if 1

double close_match( std::string_view str, std::string_view target ) {
    constexpr int64_t   offsets[]   = { 0, 1, -1 };
    int64_t             at          = 0;
    double              matches     = 0.0;


    for( auto chr : str ) {
        for( auto offset : offsets ) {
            auto idx = at + offset;

            if( static_cast< size_t >( idx ) >= target.length() ) continue;

            if( chr != target[ idx ] ) continue;

            matches += 1.0 / ( 1.0 + abs( offset ) );

            break;
        }

        ++at;
    }

    return pow( matches / std::max( str.length(), target.length() ), 1.0 );
}

#else
double close_match( std::string_view str, std::string_view target ) {
    constexpr int64_t   rad_right   = 1;
    constexpr int64_t   rad_left    = 1;
    int64_t             last        = 0;
    double              matches     = 0.0;


    for( auto chr : str ) {
        for( auto idx = last - rad_right; idx <= last + rad_left; ++idx ) {
            if( static_cast< size_t >( idx ) >= target.length() ) continue;

            if( chr != target[ idx ] ) continue;

            matches += 1.0 / ( 1.0 + abs( idx - last ) );

            break;
        }

        ++last;
    }

    return matches / std::max( str.length(), target.length() );
}

double close_match( std::string_view str, std::string_view target ) {
    if( target.length() < str.length() )
        std::swap( target, str );

    double diff[ target.length() ];

    for( size_t idx = 0; idx < target.length(); ++idx ) {
        diff[ idx ] = target[ idx ] - ( idx > str.length() ? 0 : str[ idx ] );

        diff[ idx ] *= diff[ idx ];
    }

    double accd = std::accumulate( diff, diff + target.length(), 0.0 );

    for( size_t idx = 0; idx < target.length(); ++idx )
        diff[ idx ] = target[ idx ] * target[ idx ];

    double native = std::accumulate( diff, diff + target.length(), 0.0 );

    return ( native - accd ) / native;
}
#endif



class Has_id {
public:
    using type = std::string_view;

public:
    Has_id() = default;

    Has_id( type id )
        : _id{ id }
    {}

protected:
    type   _id   = {};

public:
    type id() const {
        return _id;
    }

};



class Sound {
public:
    static std::string path_of( const std::string_view& name ) {
        return SOUNDS_PATH_MASTER + '\\' + name + ".mp3";
    }

    static bool exists( const std::string_view& name ) {
        return static_cast< bool >( std::ifstream{ Sound::path_of( name ) } );
    }

};



class File {
public:
    static void overwrite(
        const std::string_view& dir,
        const std::string_view& name,
        const auto& content
    ) {
        auto path = dir + '\\' + name;

        std::ofstream file{ path };

        if( !file ) {
            std::filesystem::create_directories( dir );

            file.open( path );
        }

        file << content;
    }

    template< typename T >
    static T read(
        const std::string_view& dir,
        const std::string_view& name,
        const T& default_content = {}
    ) {
        auto path = dir + '\\' + name;

        std::ifstream file{ path };

        if( !file ) {
            File::overwrite( dir, name, default_content );

            return default_content;
        }

        T content{};

        file >> content;

        return content;
    }

public:
    template< typename ...Args >
    static void for_each(
        const std::string_view& dir,
        const std::string_view& name,
        const auto& op
    ) {
        _for_each< Args... >(
            dir, name, op,
            Args{}...
        );
    }

private:
    template< typename ...Args >
    static void _for_each(
        const std::string_view& dir,
        const std::string_view& name,
        const auto& op,
        Args... args
    ) {
        auto path = dir + '\\' + name;

        std::ifstream file( path );

        if( !file ) {
            File::overwrite( dir, name, "" );

            return;
        }


        while( !file.eof() ) {
            ( ( file >> args ),... );

            std::invoke( op, args... );
        }
    }

};



class Stream : public std::stringstream {
public:
    using Base = std::stringstream;

public:
    Stream() = default;

    Stream( Stream&& other ) {
        static_cast< Base& >( *this ) = std::move( static_cast< Base& >( other ) );
    }

public:
    operator std::string() const {
        return this->str();
    }

    operator std::string_view () const {
        return this->view();
    }

};

Stream operator ""stm ( const char* str, size_t ) {
    return Stream{} << str;
}

#pragma endregion BOOSTERS



#pragma region OUTBOUND_STRUCTURES

class Outbound {
public:
    class Message {
    public:
        class Content : public std::stringstream {
        public:
            using Base = std::stringstream;
        
        public:
            using Base::Base;

        public:
            Content() = default;

            template< typename T >
            requires requires {
                std::stringstream{} << T{};
            }
            Content( const T& _T ) {
                *this << _T;
            }

        };

    public:
        Content   content   = {};

    public:
        void outlink() {
            std::cout
                << OUTBOUND_REPLY_MESSAGE
                << content.view();
        }
    
    };

public:
    class Embed {
    public:
        std::string   title         = {};
        std::string   description   = {};
        std::string   color         = {};
        std::string   image         = {};

    public:
        void outlink() {
            std::cout
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

};

#pragma endregion OUTBOUND_STRUCTURES



class Inbounds : public std::deque< std::string > {
public:
    using Container = std::deque< std::string >;

public:
    using Container::Container;

public:
    static constexpr int   mandatory_ins_count   = 5;

public:
    Inbounds() = default;

    Inbounds( int arg_count, char** args ) {
        for( int idx = 0; idx < mandatory_ins_count; ++idx ) {
            _out_refs[ idx ] = args[ idx + 1 ];
        }

        for( int idx = mandatory_ins_count + 1; idx < arg_count; ++idx ) {
            if( args[ idx ][ 0 ] == '-' )
                _flags.insert( args[ idx ] );
            else
                this->emplace_back( args[ idx ] );
        }
    }

private:
    enum _OUT_REFS_ACCESS_IDX {
        _EVENT, _GUILD_ID, _VOICE_ID, _USER_ID, _USER_VOICE_ID
    };

    std::string                    _out_refs[ mandatory_ins_count ]   = {};
    std::set< std::string_view >   _flags                             = {};

public:
    auto& event() const {
        return _out_refs[ _EVENT ];
    }

    auto& guild_id() const {
        return _out_refs[ _GUILD_ID ];
    }

    auto& voice_id() const {
        return _out_refs[ _VOICE_ID ];
    }

    auto& user_id() const {
        return _out_refs[ _USER_ID ];
    }

    auto& user_voice_id() const {
        return _out_refs[ _USER_VOICE_ID ];
    }

public:
    bool voice_connected() const {
        return !this->voice_id().empty();
    }

    bool user_voice_connected() const {
        return !this->user_voice_id().empty();
    }

public:
    bool operator () ( std::string_view flag ) const {
        return _flags.find( flag ) != _flags.end();
    }

public:
    template< typename T >
    requires( std::is_arithmetic_v< T > )
    auto max() {
        if constexpr( std::is_same_v< int64_t, T > )
            return _max< T >( std::stoll );
        else if constexpr( std::is_same_v< double, T > )
            return _max< T, size_t* >( std::stod, nullptr );
    }

private:
    template< typename T, typename ...Xtra_args >
    requires( std::is_arithmetic_v< T > )
    std::optional< T > _max( T ( *func )( const std::string&, Xtra_args... ), Xtra_args&&... xtra_args ) {
        std::optional< T > max{};

        for( auto& in : *this ) {
            try {
                if(
                    T value = std::invoke( func, in, xtra_args... );
                    value > max.value_or( std::numeric_limits< T >::min() )
                )
                    max = value;

            } catch( ... ) {
                continue;
            }
        }

        return max;
    }

public:
    auto first_str( const std::vector< std::string_view >& strs ) {
        std::string_view first = {};

        for( auto& in : *this ) {
            if(
                auto itr = std::find( strs.begin(), strs.end(), in );
                itr != strs.end()
            ) {
                first = *itr;

                break;
            }
        }

        return first;
    }

public:
    class FE_payload : public std::bitset< 32 > {
    public:
        int    done_count   = 0;
        int    missing_at   = 0;
        bool   abort        = false;
    };

public:
    template< typename ...Args >
    FE_payload for_each(
        std::pair<
            std::function< Args( const std::string& ) >,
            std::function< bool( Args&, FE_payload& ) >
        >... builds,

        std::function< void( Args&..., FE_payload& ) > op
    ) {
        return _for_each< Args... >( builds..., op, std::optional< Args >{}... );
    }

private:
    template< typename ...Args >
    FE_payload _for_each(
        std::pair<
            std::function< Args( const std::string& ) >,
            std::function< bool( Args&, FE_payload& ) >
        >... builds,

        std::function< void( Args&..., FE_payload& ) > op,

        std::optional< Args >... args
    ) {
        FE_payload payload{};

        while( true ) {
            if( payload.abort )
                break;

            ( ( args = this->_extract_match( builds, payload ) ), ... );

            if( ( ( ++payload.missing_at && !args.has_value() ) || ... ) )
                break;

            std::invoke( op, args.value()..., payload );

            payload.done_count++;
            payload.missing_at = 0;
        }

        payload.missing_at -= 1;

        return payload;
    }

    template< typename T >
    std::optional< T > _extract_match(
        std::pair<
            std::function< T( const std::string& ) >,
            std::function< bool( T&, FE_payload& ) >
        > build,

        FE_payload& payload
    ) {
        auto itr = this->begin();

        for( ; itr != this->end(); ++itr ) {
            try {
                T entry = build.first( *itr );

                if( !build.second( entry, payload ) )
                    continue;

                this->erase( itr );

                return std::move( entry );

            } catch( ... ) {
                continue;
            }
        }

        return {};
    }

};



#define DEBUG_LAYER_PUSH_ARGS_V std::vector< std::string_view > location, std::string log

class DebugLayer {
protected:
    class Cascade {
    public:
        Cascade() = default;

        Cascade( const auto& data )
            : _data{ data }
        {}

        Cascade( auto&& data ) noexcept
            : _data{ std::move( data ) }
        {}

    protected:
        std::variant< std::string, std::vector< Cascade > >   _data   = {};

    public:
        std::string& msg() { 
            return std::get< 0 >( _data ); 
        }

        std::vector< Cascade >& cat() { 
            return std::get< 1 >( _data ); 
        }

        std::string& cat_head() { 
            return this->cat().front().msg(); 
        }

        bool is_cat() const {
            return _data.index() == 1;
        }

    public:
        bool operator == ( const char* str ) const {
            if( !this->is_cat() ) return false;

            return const_cast< Cascade* >( this )->cat_head() == str;
        }

    };

public:
    using Cat = std::vector< Cascade >;

protected:
    inline static Cat   _root   = {};

protected:
    static void _nop( DEBUG_LAYER_PUSH_ARGS_V ) {}

    static void _push( DEBUG_LAYER_PUSH_ARGS_V ) {
        Cat* cat = &_root;

        for( auto branch = std::begin( location ); branch != std::end( location ); ++branch ) {
            auto itr = std::find( cat->begin(), cat->end(), branch->data() );

            if( itr != cat->end() ) {
                if( !itr->is_cat() ) {
                    // Handle case
                    return;
                }

                cat = &itr->cat();
            } else {
                do {
                    cat = &cat->emplace_back( Cat{ { branch->data() } } ).cat();
                } while( ++branch != std::end( location ) );

                break;
            }
        }

        cat->emplace_back( log.data() );
    }

    inline static void ( *_route )( DEBUG_LAYER_PUSH_ARGS_V ) = DebugLayer::_nop;

public:
    static void push( DEBUG_LAYER_PUSH_ARGS_V ) {
        std::invoke( DebugLayer::_route, location, log );
    }

public:
    static void uplink() {
        DebugLayer::_route = DebugLayer::_push;
    }

    static void downlink() {
        DebugLayer::_route = DebugLayer::_nop;
    }

    static bool is_uplinked() {
        return DebugLayer::_route == DebugLayer::_push;
    }

public:
    static void uplinked_op( std::function< void() > op ) {
        if( !DebugLayer::is_uplinked() ) return;

        std::invoke( op );
    }

protected:
    static void _outlink( Outbound::Message& msg, Cat& root, size_t depth ) {
        auto shift = [ & ] () -> auto& {
            for( size_t n = 0; n < depth; ++n )
                msg.content << "\t\t\t";

            return msg.content;
        };

        for( auto& entry : root ) {
            if( entry.is_cat() ) {
                shift() << "**" << entry.cat_head() << "**\n";

                DebugLayer::_outlink( msg, entry.cat(), depth + 1 );
            } else {
                if( &entry != &root.front() )
                    shift() << entry.msg() << '\n';
            }
        }
    }

public:
    static void outlink() {
        if( !DebugLayer::is_uplinked() ) return;

        Outbound::Message msg{};

        DebugLayer::_outlink( msg, _root, 0 );

        msg.outlink();
    }

};



template< typename T >
using Ref = T&;

using Voice_auto_plays_pairs = std::vector< std::pair< std::string, double > >;
using Tick_pair              = std::pair< size_t, size_t >;


#define IGU_ARGS_V Ref< Inbounds > ins, Guild guild, User user
#define IGU_ARGS_I Ref< Inbounds >, Guild, User
#define IGU_ARGS_U ins, guild, user

#define IGU_OP( name ) static void name( IGU_ARGS_V )



class Setting {
public:
    static void voice_hi_wait_to( size_t value ) {
        File::overwrite(
            SETTINGS_PATH_MASTER,
            SETTINGS_PATH_VOICE_HI_WAIT,
            value
        );
    }

    static size_t voice_hi_wait() {
        return File::read< size_t >(
            SETTINGS_PATH_MASTER,
            SETTINGS_PATH_VOICE_HI_WAIT,
            1500
        );
    }

};



#pragma region INBOUND_STRUCTURES

class Guild : public Has_id {
public:
    static constexpr size_t   tick_min   = 1;
    static constexpr size_t   tick_max   = 6000;

public:
    Guild() = default;

    using Has_id::Has_id;

public:
    double mul() const {
        return File::read< double >(
            GUILDS_PATH_MASTER + '\\' + this->_id,
            GUILDS_PATH_MUL,
            1.0
        );
    }

    void mul_to( double value ) {
        File::overwrite(
            GUILDS_PATH_MASTER + '\\' + this->_id,
            GUILDS_PATH_MUL,
            value
        );
    }

public:
    void prefix_to( const std::string_view& value ) {
        File::overwrite(
            GUILDS_PATH_MASTER + '\\' + this->_id,
            GUILDS_PATH_PREFIX,
            value
        );
    }

    std::string prefix() {
        return File::read< std::string >(
            GUILDS_PATH_MASTER + '\\' + this->_id,
            GUILDS_PATH_PREFIX,
            GUILDS_DEFAULT_PREFIX
        );
    }

public:
    auto voice_auto_plays() {
        Voice_auto_plays_pairs pairs{};

        File::for_each< std::string, double >(
            GUILDS_PATH_MASTER + '\\' + this->_id,
            GUILDS_PATH_AUTO_VOICE_PLAYS,

            [ & ] ( std::string& sound, double& prob ) -> void {
                if( sound.empty() )
                    return;

                pairs.emplace_back( std::move( sound ), prob );
            }
        );

        return pairs;
    }

    void voice_auto_plays_to( Voice_auto_plays_pairs& pairs ) {
        std::stringstream formated{};

        std::sort( pairs.begin(), pairs.end(), [] ( auto& pair1, auto& pair2 ) -> bool {
            return pair1.second > pair2.second;
        } );

        for( auto& pair : pairs )
            formated << pair.first << ' ' << pair.second << '\n';

        File::overwrite(
            GUILDS_PATH_MASTER + '\\' + this->_id,
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

public:
    void rig_to( double value ) {
        File::overwrite(
            GUILDS_PATH_MASTER + '\\' + this->_id,
            GUILDS_PATH_GAMBLE_RIG,
            value
        );
    }

    double rig() {
        return File::read< double >(
            GUILDS_PATH_MASTER + '\\' + this->_id,
            GUILDS_PATH_GAMBLE_RIG,
            0.0
        );
    }

public:
    void steal_chance_to( double value ) {
        File::overwrite(
            GUILDS_PATH_MASTER + '\\' + this->_id,
            GUILDS_PATH_STEAL_CHANCE,
            value
        );
    }

    double steal_chance() {
        return File::read< double >(
            GUILDS_PATH_MASTER + '\\' + this->_id,
            GUILDS_PATH_STEAL_CHANCE,
            0.36
        );
    }

public:
    void ticks_voice_to( size_t low, size_t high ) {
        File::overwrite< std::string >(
            GUILDS_PATH_MASTER + '\\' + this->_id,
            GUILDS_PATH_TICK_VOICE,
            ( Stream{} << low << ' ' << high ).str()
        );
    }

    Tick_pair ticks_voice() {
        std::optional< std::pair< size_t, size_t > > values{};

        File::for_each< size_t, size_t >(
            GUILDS_PATH_MASTER + '\\' + this->_id,
            GUILDS_PATH_TICK_VOICE,

            [ & ] ( size_t& low, size_t& high ) -> void {
                values = std::make_pair( low, high );
            }
        );

        if( !values.has_value() ) {
            values = std::make_pair( 600, 1800 );

            ticks_voice_to( values.value().first, values.value().second );
        }

        return values.value();
    }

    size_t pull( Tick_pair ( Guild::*method )() ) {
        auto ticks = std::invoke( method, this );

        auto diff = ticks.second - ticks.first + 1;

        return ( diff == 0 ? 0 : ( RANDOM % diff ) ) + ticks.first;
    }

    static bool tick_in_range( size_t value ) {
        return value >= tick_min && value <= tick_max;
    }

};


class User : public Has_id {
public:
    User() = default;

    using Has_id::Has_id;

public:
    void credits_to( int64_t value, Guild guild ) {
        File::overwrite(
            USERS_PATH_MASTER + '\\' + this->_id
                              + '\\' + USERS_PATH_GUILD
                              + '\\' + guild.id(),
            USERS_PATH_GUILD_CREDITS,
            value
        );
    }

    int64_t credits( Guild guild ) {
        return File::read< int64_t >(
            USERS_PATH_MASTER + '\\' + this->_id
                              + '\\' + USERS_PATH_GUILD
                              + '\\' + guild.id(),
            USERS_PATH_GUILD_CREDITS,
            0
        );
    }

    void credits_add( int64_t value, Guild guild ) {
        this->credits_to(
            ( this->credits( guild ) + value ),
            guild
        );
    }

    void credits_sub( int64_t value, Guild guild ) {
        this->credits_to(
            this->credits( guild ) - value,
            guild
        );
    }

public:
    void voice_hi_to( const std::string_view& sound ) {
        File::overwrite(
            USERS_PATH_MASTER + '\\' + this->_id,
            USERS_PATH_VOICE_HI,
            sound
        );
    }

    std::string voice_hi() {
        return File::read< std::string >(
            USERS_PATH_MASTER + '\\' + this->_id,
            USERS_PATH_VOICE_HI,
            "hello_1"
        );
    }

    void voice_bye_to( const std::string_view& sound ) {
        File::overwrite(
            USERS_PATH_MASTER + '\\' + this->_id,
            USERS_PATH_VOICE_BYE,
            sound
        );
    }

    std::string voice_bye() {
        return File::read< std::string >(
            USERS_PATH_MASTER + '\\' + this->_id,
            USERS_PATH_VOICE_BYE,
            "bye_bye"
        );
    }

public:
    static std::string make_id( std::string_view str ) {
        auto first = std::find_if( str.begin(), str.end(), [] ( const char& c ) -> bool {
            return std::isdigit( c );
        } );

        auto last = &*std::find_if( str.rbegin(), str.rend(), [] ( const char& c ) -> bool {
            return std::isdigit( c );
        } );


        if( first >= last ) return "";


        std::string id{ first, last - first + 1ULL };


        if( id.length() != 18 && id.length() != 19 ) return "";


        if(
            std::find_if( id.begin(), id.end(), [] ( const char& c ) -> bool {
                return !std::isdigit( c );
            } )
            !=
            id.end()
        )
            return "";

        return id;
    }

};

#pragma endregion INBOUND_STRUCTURES



class Powers {
public:
    using Keyword = std::tuple< int, std::vector< std::string_view > >;

    using Function = std::function< void( Ref< Inbounds >, Guild, User ) >;

    using Map = std::map< size_t, Function >;

public:
    static void execute( IGU_ARGS_V ) {
        if( std::tolower( ins.at( 0 ).at( 0 ) ) == GUILDS_AHRI_PREFIX[ 0 ] ) {
            std::for_each( ins.at( 0 ).begin(), ins.at( 0 ).end(), [] ( char& c ) -> void {
                c = std::tolower( c );
            } );

            if( ins.at( 0 ) != GUILDS_AHRI_PREFIX ) return;

            ins.pop_front();

            Python::main( IGU_ARGS_U );

            return;
        }


        auto guild_prefix = guild.prefix();

        if( !ins.at( 0 ).starts_with( guild_prefix ) ) return;

        ins.at( 0 ) = ins.at( 0 ).substr( guild_prefix.size() );

        if( ins.at( 0 ).empty() )
            ins.pop_front();


        try {
            std::invoke( make_sense_of_2( ins ), IGU_ARGS_U );

        } catch( std::out_of_range& err ) {
            what( IGU_ARGS_U );

        } catch( std::runtime_error& err ) {
            DebugLayer::push( { "Powers", "<execute>" }, "Runtime error: "s + err.what() );

        } catch( ... ) {
            DebugLayer::push( { "Powers", "<execute>" }, "Unknown runtime error." );
        }
    }


/* DEPRECATED
    static size_t make_sense_of(
        Guild           guild,
        User            user,
        Ref< Inbounds > ins,
        const bool      first_time = true
    ) {
        std::string               chain{};
        std::vector< Keyword* >   kws{};

        kws.reserve( ins.size() );


        for( auto itr = ins.begin(); itr != ins.end(); ) {
            auto found = std::find_if(
                keywords.begin(), keywords.end(),

                [ & ] ( auto& entry ) -> bool {
                    for( auto& alias : std::get< 1 >( entry ) )
                        if( *itr == alias )
                            return true;

                    return false;
                }
            );

            if(
                found == keywords.end()
                ||
                (
                    std::find_if( kws.begin(), kws.end(), [ & ] ( auto& kw ) -> bool {
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


        std::sort( kws.begin(), kws.end(), [] ( const auto& kw_1, const auto& kw_2 ) -> bool {
            return std::get< 0 >( *kw_1 ) < std::get< 0 >( *kw_2 );
        } );


        for( auto& kw : kws )
            chain += std::get< 1 >( *kw )[ 0 ];


        auto sense = std::hash< std::string_view >{}( chain );

        if( !first_time || map.contains( sense ) )
            return sense;


        for( auto& kw : kws )
            ins.push_front( std::get< 1 >( *kw ).front().data() );


        if( try_sound_play( guild, user, ins ) )
            return 0ULL;


        return make_sense_of( guild, user, calibrate_ins( ins ), false );
    }

    static Ref< Inbounds > calibrate_ins( Ref< Inbounds > ins ) {
        for( auto& in : ins ) {
            std::pair< double, Keyword* > best_match{};

            for( auto& kw : keywords )
                for( auto& alias : std::get< 1 >( kw ) )
                    if( double match = close_match( in, alias ); match > 0.5 )
                        if( match > best_match.first )
                            best_match = std::make_pair( match, &kw );

            if( best_match.second )
                in = std::get< 1 >( *best_match.second )[ 0 ];
        }


        return ins;
    }

    static bool try_sound_play( Guild guild, User user, Ref< Inbounds > ins ) {
        auto try_play = [ & ] ( const std::string& sound ) -> bool {
            if( !Sound::exists( sound ) )
                return false;

            ins.push_front( std::move( sound ) );

            Powers::Voices::play( guild, user, ins );

            return true;
        };

        std::string sound{};

        for( auto& sound_part : ins ) {
            sound += sound_part;

            if( try_play( sound ) )
                return true;

            sound += '_';
        }

        return false;
    }

*/


    struct ExtractorRet {
        ExtractorRet() = default;
        ExtractorRet( const ExtractorRet& ) = default;
        ExtractorRet( ExtractorRet&& ) = default;

        ExtractorRet(
            Function                     func,
            double                       conf,
            const Inbounds::Container&   cnt,
            bool                         asgn   = true
        )
        : function{ func }, confidence{ conf }, container{ cnt }, assign{ asgn }
        {}

        ExtractorRet(
            Function                func,
            double                  conf,
            Inbounds::Container&&   cnt,
            bool                    asgn   = true
        )
        : function{ func }, confidence{ conf }, container{ std::move( cnt ) }, assign{ asgn }
        {}

        Function              function     = {};
        double                confidence   = {};
        Inbounds::Container   container    = {};
        bool                  assign       = true;
    };


    static ExtractorRet extract_instruction_sense_2(
        Ref< Inbounds > ins
    ) {
        Inbounds::Container cnt{ ins.begin(), ins.end() };

        std::vector< std::tuple< Keyword*, double, std::string > > kws{};
        

        for( auto& in : cnt ) {
            auto [ kw, match ] = calibrate_for_keyword_2( in );

            if( !kw ) continue;

            kws.emplace_back( kw, match, in );
        }

        std::sort( kws.begin(), kws.end(), [ & ] ( const auto& kw_1, const auto& kw_2 ) -> bool {
            return std::get< 0 >( *std::get< 0 >( kw_1 ) )
                   >
                   std::get< 0 >( *std::get< 0 >( kw_2 ) );
        } );


        DebugLayer::uplinked_op( [ & ] {
             for( auto& kw : kws )
                DebugLayer::push(
                    { "Powers", "<extract_instruction_sense_2>", "Keywords:" },
                    std::get< 1 >( *std::get< 0 >( kw ) )[ 0 ].data()
                );
        } );


        uint64_t combs = std::pow( 2, kws.size() ) - 1;

        for( ; combs != 0; --combs ) {
            std::string chain{};

            for( int64_t offs = kws.size() - 1; offs >= 0; --offs )
                if( ( combs >> offs ) & 1 )
                    chain += std::get< 1 >( *std::get< 0 >( kws[ offs ] ) )[ 0 ];

            auto sense = std::hash< std::string_view >{}( chain );

            try {
                Function op = map.at( sense );


                double  accumulated_match = 0.0;
                int64_t activation_count  = 0;

                for( auto in = cnt.begin(); in != cnt.end(); ) {
                    auto itr = std::find_if( kws.begin(), kws.end(), [ & ] ( const auto& kw ) -> bool {
                        return *in == std::get< std::string >( kw );
                    } );

                    if( itr == kws.end() ) goto L_KEEP;

                    if( !( ( combs >> std::distance( kws.begin(), itr ) ) & 1 ) ) goto L_KEEP;

                    accumulated_match += std::get< double >( *itr );
                    ++activation_count;

                    in = cnt.erase( in ); continue;

                    L_KEEP: ++in; continue;
                };


                double confidence = accumulated_match / activation_count;

                DebugLayer::uplinked_op( [ & ] {
                    DebugLayer::push(
                        { "Powers", "<extract_instruction_sense_2>" },
                        "Extracted \""stm << chain << "\" with **" << confidence << "** confidence."
                    );

                    for( auto& in : cnt )
                        DebugLayer::push( 
                            { "Powers", "<extract_instruction_sense_2>", "Remaining words:" }, 
                            "\""s + in + "\""
                        );
                } );

                return { op, confidence, std::move( cnt ) }; /* move? */
            } catch( ... ) {
                continue;
            }
        }

        DebugLayer::uplinked_op( [ & ] {
            DebugLayer::push( 
                { "Powers", "<extract_instruction_sense_2>" }, 
                "All sense confidence too low to complete extraction." 
            );
        } );

        return { nullptr, 0.0, {} };
    }

    static ExtractorRet extract_sound_sense_2(
        Ref< Inbounds > ins
    ) {
        std::string sound{};

        for( auto& in : ins )
            ( sound += in ) += '_';

        if( !sound.empty() ) 
            sound.pop_back();
        else {
            DebugLayer::push( 
                { "Powers", "<extract_sound_sense_2>" },
                "Attempt to extract sound sense from an empty inbounds' deque." 
            );

            return { nullptr, 0.0, {} };
        }


        std::pair< double, std::string > best_match{ 0.5, "" };

        for( auto& file : std::filesystem::directory_iterator{ SOUNDS_PATH_MASTER } ) {
            std::string entry{ file.path().string().substr( 13 ) };
            entry.resize( entry.size() - 4 );

            if( sound == entry ) {
                best_match = { 1.0, sound };

                break;
            }

            if( double match = close_match( sound, entry ); match > best_match.first )
                best_match = { match, std::move( entry ) };
        }


        if( best_match.second.empty() ) {
            DebugLayer::uplinked_op( [ & ] {
                DebugLayer::push( 
                    { "Powers", "<extract_sound_sense_2>" }, 
                    "All senses too low to complete extraction." 
                );
            } );

            return { nullptr, 0.0, {} };
        }


        DebugLayer::uplinked_op( [ & ] {
            DebugLayer::push(
                { "Powers", "<extract_sound_sense_2>" },
                "Extracted \""stm << best_match.second << "\" with **" << best_match.first << "** confidence."
            );
        } );

        return { map.at( 9340956479027659370ULL ), best_match.first, { 1, best_match.second } };
    }

    static Function make_sense_of_2(
        Ref< Inbounds > ins
    ) {
        static ExtractorRet ( *extractors[] )( Ref< Inbounds > ) = {
            extract_instruction_sense_2,
            extract_sound_sense_2
        };

        static auto set_compare = [] ( const ExtractorRet& ex_1, const ExtractorRet& ex_2 ) -> bool {
            return ex_1.confidence > ex_2.confidence;
        };

        std::set< ExtractorRet, decltype( set_compare ) > set;


        for( auto& extractor : extractors )
            set.insert( std::invoke( extractor, ins ) );

        
        if( set.empty() ) 
            throw std::runtime_error{ "Powers: <make_sense_of_2> Empty extractor set." };


        auto& ext = *set.begin();

        if( ext.confidence < 0.5 ) {
            DebugLayer::uplinked_op( [ & ] {
                DebugLayer::push( 
                    { "Powers", "<make_sense_of_2>" }, 
                    "All senses too low to consider." 
                );
            } );

            throw std::out_of_range{ "Powers: <make_sense_of_2> Sense extraction failed." };
        }

        if( ext.assign )
            ins.assign( ext.container.begin(), ext.container.end() );

        return ext.function;
    }

    static std::pair< Keyword*, double > calibrate_for_keyword_2( std::string_view str ) {
        std::pair< Keyword*, double > best_match{ nullptr, 0.5 };

        for( auto& kw : keywords )
            for( auto& synonym : std::get< 1 >( kw ) )
                if( str == synonym )
                    return { &kw, 1.0 };
                else if( double match = close_match( str, synonym ); match > best_match.second )
                    best_match = { &kw, match };

        if( best_match.first ) {
            DebugLayer::uplinked_op( [ & ] {
                DebugLayer::push(
                    { "Powers", "<calibrate_for_keyword_2>" },
                    "\""stm << str << "\" -> \"" << std::get< 1 >( *best_match.first )[ 0 ] << "\" with **" << best_match.second << "**"
                );
            } );
        }

        return best_match;
    }

public:

#pragma region Branches

public:
    static void nop( IGU_ARGS_V ) {}

    IGU_OP( what ) {
        Outbound::Message{
            content: "Whaaaat are you sayinnnnnn..."
        }.outlink();
    }

public:
    IGU_OP( hash ) {
        if( !ins.at( 0 ).starts_with( '\"' )
            ||
            !ins.at( 0 ).ends_with( '\"' )
        ) {
            Outbound::Message{
                content: "Enclose the string in double quotes first."
            }.outlink();

            return;
        }

        ins.at( 0 ) = ins.at( 0 ).substr( 1, ins.at( 0 ).size() - 2 );

        Outbound::Message{
            content: std::hash< std::remove_reference_t< decltype( ins.at( 0 ) ) > >{}( ins.at( 0 ) )
        }.outlink();
    };

public:
    IGU_OP( kiss ) {
        Outbound::Message{
            content: "https://tenor.com/view/heart-ahri-love-gif-18791933"
        }.outlink();

        ins.emplace_front( "kiss_1" );

        Powers::Voices::play( IGU_ARGS_U );
    }

    IGU_OP( pet ) {
        Outbound::Message{
            content: "https://tenor.com/view/ahri-league-of-legends-headpats-pats-cute-gif-22621824"
        }.outlink();
    }

    IGU_OP( boobas ) {
        const char* gifs[] = {
            "https://tenor.com/view/boobs-anime-kawaii-hot-anime-girl-bigboobs-gif-21508889",
            "https://tenor.com/view/anime-boobies-boobs-big-boobs-pretty-gif-17901457",
            "https://tenor.com/view/bunny-boobs-anime-gif-19764518",
            "https://tenor.com/view/anime-gif-23317513",
            "https://tenor.com/view/erza-scarlet-fairy-tail-anime-gif-11588106",
            "https://tenor.com/view/boobs-oppai-motor-boating-face-boobed-love-anime-gif-17210152",
            "https://tenor.com/view/anime-gif-5881824",
            "https://tenor.com/view/boobs-anime-gif-18953661",
            "https://tenor.com/view/anime-boobs-jiggle-red-bra-gif-20935078",
            "https://tenor.com/view/squeeze-rabbit-boob-job-boobs-gif-15366098",
            "https://tenor.com/view/unzip-girl-bounce-boobs-gif-16742734",
            "https://tenor.com/view/noucome-oppai-smack-anime-ayame-reikadou-gif-20051530"
        };

        Outbound::Message{
            content: gifs[ RANDOM % std::size( gifs ) ]
        }.outlink();
    }

public:
    struct Voices {
        IGU_OP( connect ) {
            if( !ins.user_voice_connected() ) {
                Outbound::Message{
                    content: "Where are you..."
                }.outlink();

                return;
            }

            std::cout
                << OUTBOUND_VOICE_CONNECT
                << ins.user_voice_id();
        }

        IGU_OP( disconnect ) {
            std::cout
                << OUTBOUND_VOICE_DISCONNECT
                << guild.id();
        }

        IGU_OP( play ) {
            if( ins.event() == INBOUND_MESSAGE )
                if( !( ins.user_voice_connected() && ( ins.voice_id() == ins.user_voice_id() ) ) )
                    goto L_WHERE;

            std::cout
            << OUTBOUND_VOICE_PLAY
            << guild.id()
            << OUTBOUND_LOW_SPLIT
            << Sound::path_of( ins.at( 0 ) );

            return;
    
            L_WHERE:

            std::cout
            << OUTBOUND_REPLY_MESSAGE
            << "Where are you my cutey patootie?";
        }

        IGU_OP( stop ) {
            std::cout
                << OUTBOUND_VOICE_STOP
                << guild.id();
        }

        IGU_OP( sounds_show ) {
        std::string path{};

        path.reserve( PATH_MAX );

        std::string accumulated{};

        for( auto& file : std::filesystem::directory_iterator( SOUNDS_PATH_MASTER ) ) {
            path = file.path().string();

            size_t slash_end = path.find_last_of( '\\' ) + 1;

            accumulated += "\n\n**\"";
            accumulated += path.substr( slash_end, path.size() - slash_end - 4 );
            accumulated += "\"**";
        }

        Outbound::Embed{
            title: "DJ Ahri spinnin' these bad boys:",

            description:
                accumulated
                +=
                "\n\nYou may write the sounds with space instead of underscore.",

            color: EMBEDS_COLOR_INFO,

            image: "vinyl_purple.png"
        }.outlink();
    }
    };

public:
    struct Settings {
        IGU_OP( voice_wait_set ) {
            try {
                Setting::voice_hi_wait_to( std::abs( std::stod( ins.at( 0 ) ) ) * 1000.0 );

                auto value = static_cast< double >( Setting::voice_hi_wait() );

                Outbound::Embed{
                    title:
                        Stream{}
                        << "Now waiting **"
                        << value / 1000.0
                        << "** seconds before saying hi!",

                    description: ( value >= 5000 ? "\nI could take a bath in the meantime tho..." : "" ),

                    color: EMBEDS_COLOR_INFO

                }.outlink();

            } catch( const std::invalid_argument& err ) {
                Outbound::Embed{
                    title: "Try again after looking at this: ",

                    description: "https://www.skillsyouneed.com/num/numbers.html",

                    color: "FF0000"

                }.outlink();

            } catch( const std::out_of_range& err ) {
                Outbound::Embed{
                    title: "I can't count that much...",

                    color: "FF0000"

                }.outlink();

            }
        }

        IGU_OP( voice_wait_show ) {
                Outbound::Embed{
                    title:
                        Stream{}
                        << "Waiting **"
                        << static_cast< double >( Setting::voice_hi_wait() ) / 1000.0
                        << "** seconds before saying hi!",

                    color: EMBEDS_COLOR_INFO
                }.outlink();
            }
    };

public:
    struct Guilds {
        IGU_OP( prefix_set ) {
            if( ins.at( 0 ) == GUILDS_AHRI_PREFIX ) goto L_NOT_ELIGIBLE;

            {
            auto last = guild.prefix();

            guild.prefix_to( ins.at( 0 ) );

            Outbound::Embed{
                title:
                    Stream{}
                    << "This guild's prefix is now \"" << guild.prefix() << "\".",

                description:
                    Stream{}
                    << "Changed it from \"" << last << "\".",

                color: EMBEDS_COLOR_INFO
            }.outlink();

            return;
            }

            L_NOT_ELIGIBLE: {
                std::cout
                    << OUTBOUND_REPLY_MESSAGE
                    << "**" GUILDS_AHRI_PREFIX "**"
                    << " is not eligible as an instruction prefix.";

                return;
            }
        }

        IGU_OP( prefix_show ) {
                Outbound::Embed{
                    title:
                        Stream{}
                        << "This guild's prefix is \"" << guild.prefix() << "\".",

                    color: EMBEDS_COLOR_INFO
                }.outlink();
            }

        IGU_OP( voice_auto_plays_add ) {
            enum Payload_info {
                PROBS_CALIBD
            };


            Stream embed_desc_acc{};

            auto payload
            =
            ins.for_each< std::string, double >(
                {
                    [] ( const std::string& in ) -> std::string {
                        return in;
                    },

                    [] ( std::string& match, auto& payload ) -> bool {
                        return Sound::exists( match );
                    }
                },

                {
                    [] ( const std::string& in ) -> double {
                        return std::stod( in );
                    },

                    [] ( double& match, auto& payload ) -> bool {
                        bool in_range = match >= 0.0 && match <= 1.0;

                        return in_range;
                    }
                },

                [ & ] ( std::string& sound, double& prob, auto& payload ) -> void {
                    auto pairs = guild.voice_auto_plays();

                    std::erase_if( pairs, [ & ] ( auto& pair ) -> bool {
                        return pair.first == sound;
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

                Outbound::Embed{
                    title:
                        Stream{}
                        << "Pushed "
                        << payload.done_count
                        << " sound"
                        << ( payload.done_count == 1 ? "" : "s" )
                        << " to the guild's auto plays.",

                    description: embed_desc_acc,

                    color: EMBEDS_COLOR_INFO
                }.outlink();

                return;
            }

            L_NO_SOUND: {
                std::cout
                    << OUTBOUND_REPLY_MESSAGE
                    << "Double-check the sound cutey.";

                return;
            }

            L_NO_PROB: {
                std::cout
                    << OUTBOUND_REPLY_MESSAGE
                    << "Need a probability between 0 and 1.";

                return;
            }
        }

        IGU_OP( voice_auto_plays_clear ) {
            File::overwrite(
                GUILDS_PATH_MASTER + '\\' + guild.id(),
                GUILDS_PATH_AUTO_VOICE_PLAYS,
                ""
            );

            Outbound::Embed{
                title: "Cleared this guild's voice auto plays.",

                color: EMBEDS_COLOR_INFO
            }.outlink();
        }

        IGU_OP( voice_auto_plays_show ) {
            auto pairs = guild.voice_auto_plays();

            if( pairs.empty() ) {
                Outbound::Embed{
                    title: "This guild has no voice auto plays.",

                    color: EMBEDS_COLOR_INFO
                }.outlink();

                return;
            }


            std::stringstream accumulated;

            for( auto& pair : pairs )
                accumulated << pair.first << " ---** " << pair.second << "**\n";

            Outbound::Embed{
                title: "These are this guild's voice auto plays:",

                description: accumulated.str(),

                color: EMBEDS_COLOR_INFO
            }.outlink();
        }

        IGU_OP( gamble_rig_set ) {
            enum {
                RIG_IN_RANGE = 0
            };


            auto payload
            =
            ins.for_each< double >(
                {
                    [] ( const std::string& in ) -> double {
                        return std::stod( in );
                    },

                    [] ( double& match, auto& payload ) -> bool {
                        return payload[ RIG_IN_RANGE ] = ( match >= 0.0 && match <= 1.0 );
                    }
                },

                [ & ] ( double& rig_value, auto& payload ) -> void {
                    payload.abort = true;

                    guild.rig_to( rig_value );

                    Outbound::Embed{
                        title:
                            Stream{} << "Gamble rig value is now **" << rig_value << "**.",

                        description:
                            Stream{} << ( rig_value >= 0.5 ? "Hehe." : "" ),

                        color: EMBEDS_COLOR_INFO

                    }.outlink();
                }
            );


            if( payload.done_count != 0 )
                return;


            if( !payload[ RIG_IN_RANGE ] ) goto L_RIG_NOT_IN_RANGE;
            else                           goto L_NO_RIG_VALUE;


            L_RIG_NOT_IN_RANGE: {
                std::cout
                    << OUTBOUND_REPLY_MESSAGE
                    << "The rig value shall be between **0.0** and **1.0**.";

                return;
            }

            L_NO_RIG_VALUE: {
                std::cout
                    << OUTBOUND_REPLY_MESSAGE
                    << "I need a rig value between **0.0** and **1.0**.";

                return;
            }
        }

        IGU_OP( gamble_rig_show ) {
            Outbound::Embed{
                title:
                    Stream{}
                    << "This guild's gamble rig is **" << guild.rig() << "**.",

                color: EMBEDS_COLOR_INFO
            }.outlink();
        }

        IGU_OP( steal_chance_set ) {
            enum {
                IN_RANGE = 0
            };


            auto payload
            =
            ins.for_each< double >(
                {
                    [] ( const std::string& in ) -> double {
                        return std::stod( in );
                    },

                    [] ( double& match, auto& payload ) -> bool {
                        return payload[ IN_RANGE ] = ( match >= 0.0 && match <= 1.0 );
                    }
                },

                [ & ] ( double& chance, auto& payload ) -> void {
                    payload.abort = true;

                    guild.steal_chance_to( chance );

                    Outbound::Embed{
                        title:
                            Stream{} << "Steal chance is now **" << chance << "**.",

                        color: EMBEDS_COLOR_INFO

                    }.outlink();
                }
            );


            if( payload.done_count != 0 )
                return;


            if( !payload[ IN_RANGE ] ) goto L_CHANCE_NOT_IN_RANGE;
            else                       goto L_NO_CHANCE_VALUE;


            L_CHANCE_NOT_IN_RANGE: {
                std::cout
                    << OUTBOUND_REPLY_MESSAGE
                    << "The chance shall be between **0.0** and **1.0**.";

                return;
            }

            L_NO_CHANCE_VALUE: {
                std::cout
                    << OUTBOUND_REPLY_MESSAGE
                    << "I need a chance between **0.0** and **1.0**.";

                return;
            }
        }

        IGU_OP( steal_chance_show ) {
            Outbound::Embed{
                title:
                    Stream{}
                    << "This guild's steal chance is **" << guild.steal_chance() << "**.",

                color: EMBEDS_COLOR_INFO
            }.outlink();
        }

        IGU_OP( ticks_voice_set ) {
            enum {
                IN_RANGE,
                MATCH_FOUND
            };


            auto build_process = [] ( const std::string& in ) -> size_t {
                return std::stoull( in );
            };

            auto build_confirm = [] ( size_t& match, auto& payload ) -> bool {
                payload[ MATCH_FOUND ] = true;

                return payload[ IN_RANGE ] = payload[ IN_RANGE ] | Guild::tick_in_range( match );
            };


            size_t                    low  = {};
            std::optional< size_t > high = {};


            auto payload
            =
            ins.for_each< size_t >(
                { build_process, build_confirm },

                [ & ] ( size_t& match, auto& payload ) -> void {
                    payload.abort = true;

                    low = match;
                }
            );

            if( payload[ MATCH_FOUND ] && !payload[ IN_RANGE ] ) goto L_NOT_IN_RANGE;

            if( payload.done_count == 0 ) goto L_NO_TICK;


            payload
            =
            ins.for_each< size_t >(
                { build_process, build_confirm },

                [ & ] ( size_t& match, auto& payload ) -> void {
                    payload.abort = true;

                    high = match;
                }
            );


            if( payload[ MATCH_FOUND ] && !payload[ IN_RANGE ] ) goto L_NOT_IN_RANGE;


            if( !high.has_value() )
                high = low;
            else
                if( low > high.value() )
                    std::swap( low, high.value() );

            guild.ticks_voice_to( low, high.value() );


            {
                std::cout
                    << OUTBOUND_TICK_GUILD_RESET
                    << guild.id()
                    << OUTBOUND_LOW_SPLIT
                    << OUTBOUND_TICK_VOICE;


                Stream embed_title{};
                embed_title << "Playing a sound every **"
                            << low;

                if( low != high.value() )
                    embed_title << " - "
                                << high.value();

                embed_title << "** seconds.";


                Outbound::Embed{
                    title: embed_title,

                    description:
                        Stream{}
                        << "There are **"
                        << guild.voice_auto_plays().size()
                        << "** auto sounds in this guild.",

                    color: EMBEDS_COLOR_INFO
                }.outlink();


                return;
            }


            L_NO_TICK: {
                std::cout
                << OUTBOUND_REPLY_MESSAGE
                << "How many seconds?";

                return;
            }

            L_NOT_IN_RANGE: {
                std::cout
                << OUTBOUND_REPLY_MESSAGE
                << "The ticks must be over **"
                << Guild::tick_min
                << "** seconds and under **"
                << Guild::tick_max
                << "** seconds.";

                return;
            }
        }

        IGU_OP( ticks_voice_show ) {
            auto [ low, high ] = guild.ticks_voice();

            Stream embed_title{};
            embed_title << "Playing a sound every **"
                        << low;

            if( low != high )
                embed_title << " - "
                            << high;

            embed_title << "** seconds.";


            Outbound::Embed{
                title: embed_title,

                description:
                    Stream{}
                    << "There are **"
                    << guild.voice_auto_plays().size()
                    << "** auto sounds in this guild.",

                color: EMBEDS_COLOR_INFO
            }.outlink();
        }

        IGU_OP( mul_show ) {
            Outbound::Embed{
                title:
                    Stream{}
                    << "Guild's credit multiplier is **"
                    << guild.mul()
                    << "**.",

                color: EMBEDS_COLOR_INFO
            }.outlink();
        }

    };

public:
    struct Users {
        IGU_OP( credits_show ) {
            Outbound::Embed{
                title:
                    Stream{}
                    << "You have **"
                    << user.credits( guild )
                    << "** credits cutey!",

                description:
                    Stream{}
                    << "Guild's credit multiplier is **"
                    << guild.mul()
                    << "**.",

                color: "FFD700",

                image: "credits.png"

            }.outlink();
        };

        IGU_OP( voice_hi_set ) {
            voice_sound_set( IGU_ARGS_U, User::voice_hi_to, "intro" );
        }

        IGU_OP( voice_bye_set ) {
            voice_sound_set( IGU_ARGS_U, User::voice_bye_to, "outro" );
        }

        static void voice_sound_set(
            IGU_ARGS_V,
            auto ( User::   *method ) ( auto ),
            std::string_view type
        ) {
            std::string_view sound{};

            for( auto& in : ins )
                if( Sound::exists( in ) ) {
                    sound = in;

                    goto L_SOUND_FOUND;
                }


            {
            Outbound::Embed{
                title: "There's no such sound...",

                color: "FF0000"
            }.outlink();

            return;
            }


            L_SOUND_FOUND: {
                std::invoke( method, &user, sound );

                Outbound::Embed{
                    title:
                        Stream{}
                        << "Your "
                        << type
                        << " sound is now \"" << sound << "\".",

                    color: EMBEDS_COLOR_INFO
                }.outlink();
            }
        }

        IGU_OP( credits_steal ) {
            enum {
                NO_CREDITS
            };


            auto payload = ins.for_each< std::string >(
                {
                    [] ( const std::string& in ) -> std::string {
                        return in;
                    },

                    [] ( std::string& match, auto& payload ) -> bool {
                        return !( match = User::make_id( match ) ).empty();
                    }
                },

                [ & ] ( std::string& from_id, auto& payload ) -> void {
                    constexpr int precision = 10000;


                    ///payload.abort = true;


                    User from{ from_id };

                    double mul = std::pow(
                                     static_cast< double >( RANDOM ) / RANDOM_MAX,
                                     3
                                 ) * 0.5;


                    auto steal_success = [ & ] () -> void {
                        const int64_t credits = from.credits( guild );

                        if( credits <= 100 || RANDOM % 12 == 0 ) {
                            payload[ NO_CREDITS ] = true;

                            return;
                        }


                        int64_t stolen = credits * mul;

                        from.credits_to( credits - stolen, guild );
                        user.credits_add( stolen, guild );


                        Outbound::Embed{
                            title: "Sneaky beaky.",

                            description:
                                Stream{}
                                << "You got **"
                                << stolen
                                << "** credits from "
                                << "<@" << from.id() << ">.",

                            color: EMBEDS_COLOR_INFO

                        }.outlink();
                    };

                    auto steal_failed = [ & ] () -> void {
                        const int64_t credits = user.credits( guild );


                        int64_t owed = credits * mul;

                        from.credits_add( owed, guild );
                        user.credits_to( credits - owed, guild );


                        Outbound::Embed{
                            title: "Busted!",

                            description:
                                Stream{}
                                << "<@" << from.id() << ">"
                                << " caught you, paid them **"
                                << owed
                                << "** credits.",

                            color: EMBEDS_COLOR_INFO

                        }.outlink();
                    };


                    if( RANDOM % precision <= guild.steal_chance() * precision )
                        steal_success();
                    else
                        steal_failed();
                }
            );


            if( payload.done_count == 0 ) goto L_NO_USER;
            if( payload[ NO_CREDITS ] )   goto L_NO_CREDITS;

            return;


            L_NO_USER: {
                std::cout
                << OUTBOUND_REPLY_MESSAGE
                << "Who are you stealing from?";

                return;
            }

            L_NO_CREDITS: {
                std::cout
                << OUTBOUND_REPLY_MESSAGE
                << "Lasa-i ma bani de o pita.";

                return;
            }
        }

    };

public:
    class Gamble {
    public:
        enum Color : int {
            RED = 1, BLACK = 2, GREEN = 4
        };

        static constexpr std::array< const char*, 3 > colors{ "red", "black", "green" };

    public:
        IGU_OP( main ) {
            const auto credits = user.credits( guild );

            auto payload
            =
            ins.for_each< int, int64_t >(
                {
                    [] ( const std::string& in ) -> int {
                        for( size_t idx = 0; idx <= 2; ++idx )
                            if( in == colors[ idx ] )
                                return std::pow( 2, idx );

                        return 0;
                    },

                    [] ( int& match, auto& payload ) -> bool {
                        return match != 0;
                    }
                },

                {
                    [] ( const std::string& in ) -> int64_t {
                        return std::stoll( in );
                    },

                    [ & ] ( int64_t& match, auto& payload ) -> bool {
                        return match <= credits && match >= 0;
                    }
                },

                [ & ] (
                    int& gambled_color,
                    int64_t& gambled_ammount,
                    Inbounds::FE_payload& payload
                ) -> void {
                    const auto land   = _roulette_spin( guild, gambled_color );
                    int64_t    acc    = 0;

                    if( gambled_color == _roulette_land_to_color( land ) )
                        acc = gambled_ammount * ( land == 0 ? 13 : 1 );
                    else
                        acc = -gambled_ammount;


                    user.credits_to( credits + acc, guild );


                    Outbound::Embed{
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

                            return EMBEDS_COLOR_INFO;
                        } )(),

                        image: ( [ & ] () -> const char* {
                            if( land == 0 )
                                return "green.png";

                            return land % 2 == 0 ? "red.png" : "black.png";
                        } )()
                    }.outlink();
                }
            );

            if( payload.done_count > 0 )
                return;

            switch( payload.missing_at ) {
                case 0: {
                    std::cout
                    << OUTBOUND_REPLY_MESSAGE
                    << "What color are you gambling on?";

                    return;
                }

                case 1: {
                    std::cout
                    << OUTBOUND_REPLY_MESSAGE
                    << "Might wanna double-check how much are you gambling.";

                    return;
                }
            }
        }

        IGU_OP( coinflip ) {
            bool is_tails = RANDOM % 2;

            Outbound::Embed{
                title: is_tails ? "Tails." : "Heads.",

                color: "FFD700",

                image: is_tails ? "coin_tails.png" : "coin_heads.png"
            }.outlink();
        }

    private:
        static int _roulette_spin( Guild guild, int gambled_color ) {
            static constexpr int precision = 1000000;

            int land = static_cast< int >( RANDOM % 32 );

            if( _roulette_land_to_color( land ) == gambled_color )
                if( RANDOM % precision < guild.rig() * precision )
                    land = std::clamp( land + 1, 0, 31 );

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

public:
    class Python {
    public:
        IGU_OP( main ) {
            std::string str{};

            for( auto& in : ins )
                str += in, str += ' ';

            std::cout
            << OUTBOUND_PYTHON_RESPONSE
            << str;
        }

    };

#pragma endregion Branches

public:
    inline static std::vector< Keyword > keywords = {
        { 1, { "credits", "money", "balance", "coins" } },
        { 2, { "hash" } },
        { 3, { "kiss" } },
        { 4, { "pet" } },
        { 5, { "voice", "connect", "join" } },
        { 6, { "leave", "disconnect", "quit" } },
        { 7, { "play", "p" } },
        { 8, { "set", "change", "=", "make" } },
        { 9, { "prefix" } },
        { 10, { "hi", "hello", "greet", "intro" } },
        { 11, { "bye", "cya", "outro" } },
        { 12, { "wait", "hold" } },
        { 13, { "sounds", "soundboard" } },
        { 14, { "gamble", "bet" } },
        { 15, { "rig" } },
        { 16, { "push", "add", "+" } },
        { 17, { "pop", "sub", "-", "erase", "remove" } },
        { 18, { "auto" } },
        { 19, { "stop" } },
        { 20, { "clear" } },
        { 21, { "tick" } },
        { 22, { "steal", "rob" } },
        { 23, { "chance", "probability" } },
        { 24, { "boobas", "boobs", "tiddies", "tt" } },
        { 25, { "mul", "multiplier" } },
        { 26, { "guild" } },
        { 27, { "user" } },
        { 28, { "coinflip" } }
    };

    inline static Map map = {
        { 0ULL, nop },

        { 6839924347221205416ULL, hash },

        { 7728093003851250935ULL,  kiss },
        { 8501243811175406933ULL,  pet },
        { 17600742676213031575ULL, boobas },

        { 7492372067882396056ULL,  Voices::connect },
        { 3435728378537700265ULL,  Voices::disconnect },
        { 9340956479027659370ULL,  Voices::play },
        { 15925390277482132049ULL, Voices::stop },
        { 15169021593429937846ULL, Voices::sounds_show },

        { 6865420363795655716ULL, Settings::voice_wait_set },
        { 6685002744963886194ULL, Settings::voice_wait_show },

        { 1062816732498115940ULL,  Guilds::prefix_set },
        { 2016015562653219627ULL,  Guilds::prefix_show },
        { 14632587987046264091ULL, Guilds::voice_auto_plays_add },
        { 8901744138937185971ULL,  Guilds::voice_auto_plays_clear },
        { 11115212484132745176ULL, Guilds::voice_auto_plays_show },
        { 11779848440330006868ULL, Guilds::gamble_rig_set },
        { 5995434624829588022ULL,  Guilds::gamble_rig_show },
        { 9483180197231311695ULL,  Guilds::steal_chance_set },
        { 11508569767627199743ULL, Guilds::steal_chance_show },
        { 9960837043950823267ULL,  Guilds::ticks_voice_set },
        { 1450920041828067319ULL,  Guilds::ticks_voice_show },
        { 8604667370303577938ULL,  Guilds::mul_show },

        { 4330606938181995941ULL,  Users::credits_show },
        { 12382791774924742628ULL, Users::voice_hi_set },
        { 183303123199750495ULL,   Users::voice_bye_set },
        { 3546343074134243021ULL,  Users::credits_steal },
        { 6100755804673006265ULL,  Users::credits_steal },

        { 15754336788579780731ULL, Gamble::main },
        { 16987320819845335283ULL, Gamble::main },
        { 6322474587853159699ULL,  Gamble::coinflip }
    };

};



#pragma region EMITTERS

class Message {
public:
    IGU_OP( on_create ) {
        double mul = guild.mul();

        user.credits_add( 20.0 * mul, guild );

        guild.mul_to( std::min( mul + 0.001, 10.0 ) );


        Powers::execute( IGU_ARGS_U );
    }
};


class Voice {
public:
    IGU_OP( on_update ) {
        if( ins.voice_id().empty() ) return;


        auto& old_ch = ins.at( 0 );
        auto& new_ch = ins.at( 1 );

        if( old_ch == new_ch ) return;


        bool connected = ( new_ch == ins.voice_id() );

        if( !connected && ( old_ch != ins.voice_id() ) ) return;


        ins.emplace_front( connected ? user.voice_hi() : user.voice_bye() );

        if( connected )
            std::this_thread::sleep_for(
                std::chrono::milliseconds( Setting::voice_hi_wait() )
            );

        Powers::Voices::play( ins, guild, {} );
    }

};


class Tick {
public:
    IGU_OP( on_tick ) {
        switch( std::hash< std::string_view >{}( ins.at( 0 ) ) ) {
            case 7492372067882396056ULL:  voice( IGU_ARGS_U ); break;

            case 10702603417961775396ULL: init( IGU_ARGS_U ); break;
        }
    }

public:
    IGU_OP( init ) {
        voice( IGU_ARGS_U );
    }

    IGU_OP( voice ) {
        Tick::outbound( guild, guild.pull( Guild::ticks_voice ), OUTBOUND_TICK_VOICE );


        if( ins.voice_id().empty() ) return;


        constexpr int precision = 1000000;
        double        gauge     = RANDOM % precision;
        double        sum       = 0.0;

        auto          pairs     = guild.voice_auto_plays();
        auto          itr       = pairs.begin();

        for( ; itr != pairs.end(); ++itr ) {
            sum += itr -> second * precision;

            if( gauge <= sum ) break;
        }

        if( itr == pairs.end() )
            return;

        ins.push_front( itr->first );

        Powers::Voices::play( ins, guild, {} );
    }

public:
    static void outbound( Guild guild, size_t delay, std::string_view type ) {
        std::cout
            << OUTBOUND_TICK_GUILD_SET
            << guild.id()
            << OUTBOUND_LOW_SPLIT
            << delay
            << OUTBOUND_LOW_SPLIT
            << type;
    }

};

#pragma endregion EMITTERS



std::map<
    std::string_view,
    std::function< void( IGU_ARGS_I ) >
>  event_map = {
    { INBOUND_MESSAGE,      Message::on_create },
    { INBOUND_VOICE_UPDATE, Voice::on_update },
    { INBOUND_TICK,         Tick::on_tick }
};



int main( int arg_count, char* args[] ) {
    auto dbg_begin_time = std::chrono::high_resolution_clock::now();


    RANDOM_SEEDING_OP();


    Inbounds ins{ arg_count, args };


    if( ins( "-debug" ) ) {
        DebugLayer::uplink();

        DebugLayer::push( { "Inbound front", "IDs" }, "Voice: "s + ins.voice_id() );
        DebugLayer::push( { "Inbound front", "IDs" }, "Guild: "s + ins.guild_id() );
        DebugLayer::push( { "Inbound front", "IDs" }, "User: "s + ins.user_id() );
        DebugLayer::push( { "Inbound front", "IDs" }, "User voice: "s + ins.user_voice_id() );
    }

    Guild guild{ ins.guild_id() };
    User  user { ins.user_id() };


    try {
        std::invoke( event_map.at( ins.event() ), IGU_ARGS_U );
    } catch( std::out_of_range& err ) {
        DebugLayer::push( { "Exceptions", "<main>" }, "Out of range: "s + err.what() );
    } catch( std::runtime_error& err ) {
        DebugLayer::push( { "Exceptions", "<main>" }, "Runtime error: "s + err.what() );
    } catch( ... ) {
        DebugLayer::push( { "Exceptions", "<main>" }, "Unknown runtime error." );
    }


    double dbg_elapsed = std::chrono::duration_cast< std::chrono::microseconds >(
        std::chrono::high_resolution_clock::now() - dbg_begin_time
    ).count();


    DebugLayer::uplinked_op( [ & ] {
        DebugLayer::push( { "Chronos" }, "Core executed in: **"stm << dbg_elapsed << "**us." );
    } );
    

    DebugLayer::outlink();


    return 0;
}
