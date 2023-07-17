//#region Includes

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
    Guild,
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
const { type } = require("node:os");
const { timeStamp } = require("node:console");

//#endregion


//#region Quints

const INBOUND_LOW_SPLIT = "||__LOW_SPLIT__||";
const INBOUND_HIGH_SPLIT = "||__HIGH_SPLIT__||";

const INBOUND_SCRIPT = "script";

const INBOUND_REPLY_MESSAGE = "reply_message";

const INBOUND_REPLY_EMBED = "reply_embed"

const INBOUND_VOICE_CONNECT = "voice_connect";
const INBOUND_VOICE_DISCONNECT = "voice_disconnect";
const INBOUND_VOICE_PLAY = "voice_play";

const INBOUND_AUTO_VOICE_PLAY = "auto_voice_play";


const OUTBOUND_TICK = "tick";

//#endregion


//#region Utility

const wait_for = async ( millis ) => {
    return new Promise( resolve => setTimeout( resolve, millis ) );
};



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

/**
@param   { Guild } guild 
@param   { User }  user 

@returns { VoiceBasedChannel }
**/
const voice_channel_of = ( guild, user ) => {
    let member = guild.members.cache.get( user.id );

    return member.voice.channel;
}

//#endregion Utility



class Engine {
    constructor() { 
        this.client = new Client( {
            intents: [
                GatewayIntentBits.Guilds,
                GatewayIntentBits.GuildMessages,
                GatewayIntentBits.MessageContent,
                GatewayIntentBits.GuildMembers,
                GatewayIntentBits.GuildVoiceStates,
                GatewayIntentBits.GuildPresences
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

        this.main_thread( [] );
    }


//#region Ons

    on_ready() {
        console.log( `All nine tails ready to go. ${ this.client.user.tag } reporting in.` );
       
        this.client.user.setActivity( {
            type: 0,
            name: "in the wilds."
        } );
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

//#endregion


//#region Exe

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
                let channel = voice_channel_of( guild, user );

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

            

            case INBOUND_AUTO_VOICE_PLAY: {
                this.voices.forEach( voice => {
                    voice.play( ins.at( 0 ) );
                } );

                
                xtra.delay = parseInt( ins.at( 1 ) );

                xtra.unlock();


                break; }
        }
    }

//#endregion


//#region Utility

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

//#endregion Utility


    main_thread = async ( args ) => { 
        let tick = {
            delay: 6,
            unlock: null
        };

        while( true ) {
            await wait_for( tick.delay * 1000 );

            tick.unlock = null;

            let sync = new Promise( resolve => {
                tick.unlock = resolve;
            } );
            
            this.run_engine( 
                [
                    OUTBOUND_TICK
                ],
    
                ( cout ) => {
                    console.log(
                        `\nTick at ${ new Date().toLocaleTimeString() }. Engine inbound: \n"${ cout }".`
                    );

                    this.execute_chain( cout, null, null, tick );
                }
            );

            await sync;
        }
    }
};



var ahri = new Engine();


