const config = require( "./config.json" );

const {
    Client,
    Events,
    GatewayIntentBits,
    SlashCommandBuilder,
    Message
} = require( "discord.js" );

const {
    joinVoiceChannel, VoiceConnection, createAudioPlayer, getVoiceConnection, createAudioResource
} = require( "@discordjs/voice" );

const { 
    execFile 
} = require( "node:child_process" );

const {
    readFileSync
} = require( "fs" );



const INBOUND_SPLITTER = "||SPLIT||";

const INBOUND_MESSAGE_REPLY = "message_reply";

const INBOUND_VOICE_CONNECT = "voice_connect";
const INBOUND_VOICE_PLAY = "voice_play"


const SOUNDS_PATH_MASTER = ".\\Data\\Audio"



class Engine {
    constructor() { 
        this.client = new Client( {
            intents: [
                GatewayIntentBits.Guilds,
                GatewayIntentBits.GuildMessages,
                GatewayIntentBits.MessageContent,
                GatewayIntentBits.GuildMembers,
                GatewayIntentBits.GuildVoiceStates
            ]
        } );


        this.client.on( Events.ClientReady, ( client ) => {
            this.on_ready();
        } );

        this.client.on( Events.MessageCreate, ( msg ) => {
            this.on_message( msg );
        } );


        this.voices = [];

        this.sounds = new Map( Object.entries( JSON.parse( readFileSync( ".\\Data\\Audio\\sounds.json" ) ) ) );


        this.client.login( config.token );
    }


    on_ready() {
        console.log( `All nine tails ready to go. ${ this.client.user.tag } reporting in.` );
    }


    /**
    @param { Message } msg
    **/
    on_message( msg ) {
        let args = [
            "message",
            msg.guildId,
            msg.author.id,
        ];

        msg.content.split( " " ).forEach( ( component ) => {
            args.push( component );
        } );

        this.run_engine(
            args,

            /**
             @param { string } cout 
            **/
            ( cout ) => {
                let words = cout.split( INBOUND_SPLITTER );

                this.execute( words[ 0 ], msg.guildId, msg.author.id, msg, words.slice( 1 ) );
            }
        );
    }


    run_engine( args, callback ) {
        let exe = execFile( config.exe, args );

        exe.stdout.on( "data", ( cout ) => {
            callback( cout.toString() );
        } );

        exe.stderr.on( "data", ( cerr ) => {
            
        } );
        
        exe.on( "exit", ( ret ) => {
            
        } ); 
    }


    /**
    @param { string } type 
    @param { string } guild_id 
    @param { string } user_id 
    @param { * } xtra 
    @param { string[] } ins 
    **/
    execute( type, guild_id, user_id, xtra, ins ) {
        switch( type ) {
            case INBOUND_MESSAGE_REPLY: {
                xtra.reply( ins[ 0 ] );

                break; }

            case INBOUND_VOICE_CONNECT: {
                let channel = this.voice_channel_get( guild_id, user_id );

                if( !channel ) {
                    if( xtra instanceof Message ) {
                        this.execute( INBOUND_MESSAGE_REPLY, guild_id, user_id, xtra, [ "Where are youuu..." ] );
                    }

                    break;
                }


                joinVoiceChannel( {
                    channelId: channel.id,
                    guildId: channel.guildId,
                    adapterCreator: channel.guild.voiceAdapterCreator  
                } )
                .subscribe( this.voices[
                    this.voices.push( {
                        guild_id: guild_id,
                        audio: createAudioPlayer()
                    } )
                - 1 ].audio );
            
                
                break; }

            case INBOUND_VOICE_PLAY: {
                let voice = this.voice_get( guild_id );

                let sound = createAudioResource( ins[ 0 ] );

                voice.audio.play( sound );

                break; }
        }
    }


    /**
     @param { string } guild_id 
     @param { string } user_id 
     @returns { import("discord.js").VoiceBasedChannel }
    **/
    voice_channel_get( guild_id, user_id ) {
        let guild = this.client.guilds.cache.get( guild_id );
        let member = guild.members.cache.get( user_id );

        return member.voice.channel;
    }

    /**
     @param { string } guild_id 
    **/
    voice_get( guild_id ) {
        let found = null;

        this.voices.forEach( ( voice ) => {
            if( voice.guild_id == guild_id )
                found = voice;
        } );

        return found;
    }

};



var ahri = new Engine();