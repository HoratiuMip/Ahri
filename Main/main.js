const config = require( "./config.json" );

const {
    Client,
    Events,
    GatewayIntentBits,
    SlashCommandBuilder,
    Message,
    AttachmentBuilder,
    EmbedBuilder,
    VoiceState,
    VoiceBasedChannel,
    User,
    Guild
} = require( "discord.js" );

const {
    joinVoiceChannel,
    createAudioPlayer, 
    createAudioResource,
} = require( "@discordjs/voice" );

const { 
    execFile 
} = require( "node:child_process" );

const {
    readFileSync
} = require( "fs" );



const INBOUND_LOW_SPLIT = "||__LOW_SPLIT__||";
const INBOUND_HIGH_SPLIT = "||__HIGH_SPLIT__||";

const INBOUND_SCRIPT = "script";

const INBOUND_REPLY_MESSAGE = "reply_message";

const INBOUND_REPLY_EMBED = "reply_embed"

const INBOUND_VOICE_CONNECT = "voice_connect";
const INBOUND_VOICE_DISCONNECT = "voice_disconnect";
const INBOUND_VOICE_PLAY = "voice_play"



class Voice {
    /**
     @param { VoiceBasedChannel } channel 
    **/
    constructor( channel ) {
        this.guild = channel.guild;

        this.connection = joinVoiceChannel( {
            channelId: channel.id,
            guildId: this.guild.id,
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
            if( msg.author.bot ) return;

            this.on_message( msg );
        } );

        this.client.on( Events.VoiceStateUpdate, async ( old_state, new_state ) => {
            if( new_state.member.user.bot ) return;

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
                console.log(
                    `\nMessage at ${ new Date().toLocaleTimeString() }. Engine inbound: \n"${ cout }".`
                );

                this.execute_chain(
                    cout,
                    msg.guild, msg.author,
                    msg
                );
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
                console.log(
                    `\nVoice state update at ${ new Date().toLocaleTimeString() }. Engine inbound: \n"${ cout }".`
                );

                this.execute_chain(
                    cout,
                    new_state.guild, new_state.member.user,
                    new_state
                );
            }
        );
    }


    run_engine( args, callback ) {
        let exe = execFile( config.exe, args );

        exe.stdout.on( "data", ( cout ) => {
            callback( cout.toString() );
        } );

        exe.stderr.on( "data", ( cerr ) => {
            console.log( `Engine cerr'd: "${ cerr }".` );
        } );
        
        exe.on( "exit", ( ret ) => {
            if( ret == 0 ) return;

            console.log( `Engine returned: "${ ret }".` );
        } ); 
    }


    /**
    @param { string } chain 
    @param { Guild } guild 
    @param { User } user 
    @param { * } xtra 
    @param { string[] } ins 
    **/
    execute_chain( chain, guild, user, xtra ) {
        chain.split( INBOUND_HIGH_SPLIT ).forEach( async ( high ) => {
            let lows = high.split( INBOUND_LOW_SPLIT );

            this.execute_link( lows.at( 0 ), guild, user, xtra, lows.slice( 1 ) );
        } );
    }

    /**
    @param { string } type 
    @param { Guild } guild 
    @param { User } user
    @param { * } xtra 
    @param { string[] } ins 
    **/
    execute_link( type, guild, user, xtra, ins ) {
        switch( type ) {
            case INBOUND_SCRIPT: {
                eval( ins.at( 0 ) );

                break; }


            case INBOUND_REPLY_MESSAGE: {
                xtra.reply( ins.at( 0 ) );

                break; }
            
            
            case INBOUND_REPLY_EMBED: {
                let path = new AttachmentBuilder( ins.at( 3 ) );

                let name = ins.at( 3 ).split( "\\" );
                name = name.at( name.length - 1 );

                let embed = new EmbedBuilder()
                .setTitle( ins.at( 0 ) )
                .setDescription( ins.at( 1 ) )
                .setColor( ins.at( 2 ) )
                .setThumbnail( `attachment://${ name }` );

                xtra.reply( {
                    files: [ path ], embeds: [ embed ]
                } );

                break; }


            case INBOUND_VOICE_CONNECT: {
                let channel = this.voice_channel_of( guild, user );

                if( !channel ) {
                    if( xtra instanceof Message ) {
                        this.execute_link( INBOUND_REPLY_MESSAGE, guild, user, xtra, [ "Where are youuu..." ] );
                    }

                    break;
                }


                this.voices.push( new Voice( channel ) );
            

                break; }

            case INBOUND_VOICE_DISCONNECT: {
                let voice = this.voice_in( guild );

                if( !voice ) break;
                
                voice.connection.destroy();

                
                break; }

            case INBOUND_VOICE_PLAY: {
                let voice = this.voice_in( guild );

                if( !voice ) break;

                voice.play( ins.at( 0 ) );


                break; }
        }
    }


    /**
     @param { Guild } guild 
     @param { User } user 
     @returns { VoiceBasedChannel }
    **/
    voice_channel_of( guild, user ) {
        let member = guild.members.cache.get( user.id );

        return member.voice.channel;
    }

    /**
     @param { Guild } guild 
     @returns { Voice }
    **/
    voice_in( guild ) {
        let found = null;

        this.voices.forEach( ( voice ) => {
            if( voice.guild.id == guild.id )
                found = voice;
        } );

        return found;
    }

};



var ahri = new Engine();