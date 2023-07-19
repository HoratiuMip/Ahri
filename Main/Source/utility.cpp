#pragma once

#include "std_includes.cpp"
#include "quints.cpp"



class Inbounds : public std :: deque< std :: string > {
public:
    using Base = std :: deque< std :: string >;

public:
    using Base :: Base;

public:
    template< typename T >
    requires( std :: is_arithmetic_v< T > )
    auto max() {
        if constexpr( std :: is_same_v< int64_t, T > )
            return _max< T >( std :: stoll );
        else if constexpr( std :: is_same_v< double, T > )
            return _max< T, size_t* >( std :: stod, nullptr );
    }

private:
    template< typename T, typename ...Xtra_args >
    requires( std :: is_arithmetic_v< T > )
    std :: optional< T > _max( T ( *func )( const std :: string&, Xtra_args... ), Xtra_args&&... xtra_args ) {
        std :: optional< T > max{};

        for( auto& in : *this ) {
            try {
                if(
                    T value = std :: invoke( func, in, xtra_args... );
                    value > max.value_or( std :: numeric_limits< T > :: min() )
                )
                    max = value;

            } catch( ... ) {
                continue;
            }
        }

        return max;
    }

public:
    auto first_str( const std :: vector< std :: string_view >& strs ) {
        std :: string_view first = {};

        for( auto& in : *this ) {
            if(
                auto itr = std :: find( strs.begin(), strs.end(), in );
                itr != strs.end()
            ) {
                first = *itr;

                break;
            }
        }

        return first;
    }

public:
    struct For_each_result {
        int   completed    = 0;
        int   missing_at   = 0;
    };

public:
    template< typename ...Args >
    For_each_result for_each( 
        std :: pair< 
            std :: function< Args( const std :: string& ) >,
            std :: function< bool( Args& ) >
        >... builds,

        std :: function< void( Args&... ) > op 
    ) {
        return _for_each< Args... >( builds..., op, std :: optional< Args >{}... );
    }

private:
    template< typename ...Args >
    For_each_result _for_each( 
        std :: pair< 
            std :: function< Args( const std :: string& ) >,
            std :: function< bool( Args& ) >
        >... builds,

        std :: function< void( Args&... ) > op,

        std :: optional< Args >... args
    ) {
        For_each_result result{};

        while( true ) {
            ( ( args = this -> _extract_match( builds ) ), ... );

            if( ( ( ++result.missing_at && !args.has_value() ) || ... ) ) 
                break;
            
            std :: invoke( op, args.value()... );

            result.completed++;
            result.missing_at = 0;
        }

        result.missing_at -= 1;

        return result;
    }

    template< typename T >
    std :: optional< T > _extract_match( 
        std :: pair< 
            std :: function< T( const std :: string& ) >,
            std :: function< bool( T& ) >
        > build
    ) {
        auto itr = this -> begin();

        for( ; itr != this -> end(); ++itr ) {
            try {
                T entry = build.first( *itr );

                if( !build.second( entry ) ) 
                    continue;

                this -> erase( itr );
                
                return std :: move( entry );

            } catch( ... ) {
                continue;
            }
        }

        return {};
    }

};



template< typename T > 
using Ref = T&;

using Event_map = std :: map< std :: string_view, std :: function< void( Ref< Inbounds > ) > >;

using ID = std :: string;

using Voice_auto_plays_pairs = std :: vector< std :: pair< std :: string, double > >;

#define COMMAND_BRANCH( name ) inline static void name( Guild guild, User user, Ref< Inbounds > ins )



std :: string operator + ( 
    const std :: string& rhs, 
    const std :: string_view& lhs 
) {
    return rhs + lhs.data();
}

std :: string operator + ( 
    const std :: string_view& rhs, 
    const std :: string_view& lhs 
) {
    return std :: string{ rhs.data() } + lhs.data();
}

std :: string operator + ( 
    const char* rhs, 
    const std :: string_view& lhs 
) {
    return std :: string{ rhs } + lhs.data();
}

std :: string operator + ( 
    const std :: string_view& rhs, 
    const char* lhs 
) {
    return std :: string{ rhs.data() } + lhs;
}

std :: string operator + ( 
    const std :: string_view& rhs, 
    const char& lhs 
) {
    return std :: string{ rhs.data() } + lhs;
}



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



class Sound {
public:
    static std :: string path_of( const std :: string_view& name ) {
        return SOUNDS_PATH_MASTER + '\\' + name + ".mp3";
    }

    static bool exists( const std :: string_view& name ) {
        return static_cast< bool >( std :: ifstream{ Sound :: path_of( name ) } );
    }

};



class File {
public:
    static void overwrite( 
        const std :: string_view& dir, 
        const std :: string_view& name,
        const auto& content 
    ) {
        auto path = dir + '\\' + name;

        std :: ofstream file{ path };

        if( !file ) {
            std :: filesystem :: create_directories( dir );

            file.open( path );
        }
        
        file << content;
    }

    template< typename T >
    static T read(
        const std :: string_view& dir, 
        const std :: string_view& name,
        const T& default_content = {}
    ) {
        auto path = dir + '\\' + name;

        std :: ifstream file{ path };

        if( !file ) {
            File :: overwrite( dir, name, default_content );

            return default_content;
        }

        T content{};

        file >> content;

        return content;
    }

public:
    template< typename ...Args >
    static void for_each( 
        const std :: string_view& dir,
        const std :: string_view& name,
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
        const std :: string_view& dir,
        const std :: string_view& name,
        const auto& op,
        Args... args
    ) {
        auto path = dir + '\\' + name; 

        std :: ifstream file( path );

        if( !file ) {
            File :: overwrite( dir, name, "" );

            return;
        }


        while( !file.eof() ) {
            ( ( file >> args ),... );

            op( args... );
        }
    }

};



class Stream : public std :: ostringstream {
public:
    using Base = std :: ostringstream;

public:
    Stream() = default;

    using Base :: Base;

public:
    operator std :: string() const {
        return this -> str();
    }

    operator std :: string_view () const {
        return this -> view();
    }

};
