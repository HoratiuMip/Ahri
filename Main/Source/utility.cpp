#pragma once

#include "std_includes.cpp"
#include "quints.cpp"



template< typename T > 
using Ref = T&;

using Inbounds = std :: deque< std :: string >;

using Event_map = std :: map< std :: string_view, std :: function< void( Ref< Inbounds > ) > >;

using ID = std :: string;

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
