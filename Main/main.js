const config = require( "./config.json" );

const {
    Client,
    Events,
    GatewayIntentBits,
    SlashCommandBuilder,
    Message,
    VoiceState,
    VoiceBasedChannel
} = require( "discord.js" );

const {
    joinVoiceChannel, VoiceConnection, createAudioPlayer, getVoiceConnection, createAudioResource,
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
const INBOUND_VOICE_DISCONNECT = "voice_disconnect";
const INBOUND_VOICE_PLAY = "voice_play"



class Voice {
    /**
     @param { VoiceBasedChannel } channel 
    **/
    constructor( channel ) {
        this.guild_id = channel.guildId;

        this.connection = joinVoiceChannel( {
            channelId: channel.id,
            guildId: this.guild_id,
            adapterCreator: channel.guild.voiceAdapterCreator  
        } );

        this.audio = createAudioPlayer();

        this.connection.subscribe( this.audio );
    }


    /**
    @param { string } path 
    **/
    play( path ) {
        this.audio.play( createAudioResource( path ) );
    }

};



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


        this.client.on( Events.ClientReady, async ( client ) => {
            this.on_ready();
        } );

        this.client.on( Events.MessageCreate, async ( msg ) => {
            this.on_message( msg );
        } );

        this.client.on( Events.VoiceStateUpdate, async ( old_state, new_state ) => {
            this.on_voice_state( old_state, new_state );
        } );   


        /**
         @type { Voice[] }  
        */
        this.voices = [];


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

    /**
    @param { VoiceState } old_state
    @param { VoiceState } new_state
    **/
    on_voice_state( old_state, new_state ) {
        this.run_engine(
            [
                "voice_update",
                new_state.guild.id,
                new_state.member.user.id,
                old_state.channelId ?? "",
                new_state.channelId ?? ""
            ],

            /**
             @param { string } cout 
            **/
            ( cout ) => {
                let words = cout.split( INBOUND_SPLITTER );

                this.execute( words[ 0 ], new_state.guild.id, new_state.member.user.id, new_state, words.slice( 1 ) );
            }
        );
    }


    run_engine( args, callback ) {
        let exe = execFile( config.exe, args );

        exe.stdout.on( "data", ( cout ) => {
            callback( cout.toString() );
        } );

        exe.stderr.on( "data", ( cerr ) => {
            console.log( cerr );
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
                xtra.reply( ins.at( 0 ) );


                break; }


            case INBOUND_VOICE_CONNECT: {
                let channel = this.voice_channel_of( guild_id, user_id );

                if( !channel ) {
                    if( xtra instanceof Message ) {
                        this.execute( INBOUND_MESSAGE_REPLY, guild_id, user_id, xtra, [ "Where are youuu..." ] );
                    }

                    break;
                }


                this.voices.push( new Voice( channel ) );
            

                break; }

            case INBOUND_VOICE_DISCONNECT: {
                let voice = this.voice_in( guild_id );

                if( !voice ) break;
                
                voice.connection.destroy();

                
                break; }

            case INBOUND_VOICE_PLAY: {
                let voice = this.voice_in( guild_id );

                if( !voice ) break;

                voice.play( ins.at( 0 ) );


                break; }
        }
    }


    /**
     @param { string } guild_id 
     @param { string } user_id 
     @returns { import("discord.js").VoiceBasedChannel }
    **/
    voice_channel_of( guild_id, user_id ) {
        let guild = this.client.guilds.cache.get( guild_id );
        let member = guild.members.cache.get( user_id );

        return member.voice.channel;
    }

    /**
     @param { string } guild_id 
     @returns { Voice }
    **/
    voice_in( guild_id ) {
        let found = null;

        this.voices.forEach( ( voice ) => {
            if( voice.guild_id == guild_id )
                found = voice;
        } );

        return found;
    }

};



var ahri = new Engine();