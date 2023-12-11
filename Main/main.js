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
    ApplicationCommandNumericOptionMinMaxValueMixin,
    range,
} = require( "discord.js" );

const {
    joinVoiceChannel,
    createAudioPlayer, 
    createAudioResource,
} = require( "@discordjs/voice" );

const { 
    execFile,
    execFileSync
} = require( "node:child_process" );

const {
    readFileSync
} = require( "fs" );
const { type } = require("node:os");
const { timeStamp } = require("node:console");
const { stdout, stderr } = require("node:process");

//#endregion


//#region Quints

const NULL_BOUND = "NULL";

const INBOUND_LOW_SPLIT = "||__LOW_SPLIT__||";
const INBOUND_HIGH_SPLIT = "||__HIGH_SPLIT__||";

const INBOUND_SCRIPT = "script";

const INBOUND_REPLY_MESSAGE = "reply_message";

const INBOUND_REPLY_EMBED = "reply_embed"

const INBOUND_VOICE_CONNECT = "voice_connect";
const INBOUND_VOICE_DISCONNECT = "voice_disconnect";
const INBOUND_VOICE_PLAY = "voice_play";
const INBOUND_VOICE_STOP = "voice_stop";


const INBOUND_TICK_GUILD_SET = "tick_guild_set";
const INBOUND_TICK_GUILD_RESET = "tick_guild_reset";


const INBOUND_PYTHON_RESPONSE = "python_response";


const OUTBOUND_MESSAGE = "message";
const OUTBOUND_VOICE_UPDATE = "voice_update";
const OUTBOUND_TICK = "tick";


const OUTBOUND_TICK_INIT = "init";
const OUTBOUND_TICK_VOICE = "voice";
const OUTBOUND_TICK_REMINDER = "reminder";

//#endregion


//#region Utility

const wait_for = async ( millis ) => {
    return new Promise( resolve => setTimeout( resolve, millis ) );
};



class Echo {
    constructor( entries ) {
        this._entries = {};

        this.push( entries );
    }


    release = () => {
        console.log( this._entries );
        console.log();
    }


    push = ( entries ) => {
        if( entries instanceof Array )
            entries.forEach( entry => {
                this._entries[ entry.info ] = entry.data;
            } );
        else if( entries )
            this._entries[ entries.info ] = entries.data;
    }

};



class Voice {
    /**
     @param { VoiceBasedChannel } channel 
    **/
    constructor( channel ) {
        this.guild = channel.guild;

        this.channel = channel;

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
    play = ( path ) => {
        this.audio.play( createAudioResource( path ) );
    }

    stop = () => {
        this.audio.stop();
    }

};

/**
@param   { Guild } guild 
@param   { User }  user 

@returns { VoiceBasedChannel }
**/
const voice_of = ( user, guild ) => {
    let member = guild.members.cache.get( user.id );

    return member.voice;
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
            try {
                this.on_ready();
            } catch( error ) {
                console.log( error );
            }
        } );

        this.client.on( Events.MessageCreate, async ( msg ) => {
            if( msg.author.bot ) return;

            try {
                this.on_message( msg );
            } catch( error ) {
                console.log( "[ CRITICAL FAULT ]\n" );
                console.log( "---------------------------------\n" );
                console.log( msg );
                console.log( "---------------------------------\n" );
                console.log( error );
                console.log( "---------------------------------\n\n" );
            }
        } );

        this.client.on( Events.VoiceStateUpdate, async ( old_state, new_state ) => {
            if( new_state.member.user.bot ) return;

            try {
                this.on_voice_state( old_state, new_state );
            } catch( error ) {
                console.log( "[ CRITICAL FAULT ]\n");
                console.log( "---------------------------------\n" );
                console.log( old_state );
                console.log( "---------------------------------\n" );
                console.log( new_state );
                console.log( "---------------------------------\n" );
                console.log( error );
                console.log( "---------------------------------\n\n" );
            }
        } );   


        /**
         @type { Voice[] }  
        */
        this.voices = [];

        
        this.next_tick_id = 1;

        this.ticks = new Map();


        this.python_script = require( "child_process" ).spawn( "python", [ "chatbot_predictor.py" ] );


        this.client.login( config.token );
    }

    
//#region Ons

    on_ready() {
        console.log( `All nine tails ready to go. ${ this.client.user.tag } reporting in.\n\n` );
    
        this.client.user.setActivity( {
            type: 0,
            name: "in the wilds."
        } );

        this.guilds_launch_ticks();

        ///this.client.user.setAvatar( "C:/Hackerman/Git/Ahri/Pfps/arcade.png" );
    }

    /**
    @param { Message } msg
    **/
    on_message( msg ) {
        let echo = new Echo( [
            { info: "Message", data: `At ${ new Date().toLocaleTimeString() }` },
            { info: "In", data: { guild: msg.guild.name, id: msg.guildId } },
            { info: "By", data: { user: msg.author.username, id: msg.author.id } }
        ] );


        let args = this.make_front( OUTBOUND_MESSAGE, msg.guild, msg.author );

        msg.content.split( " " ).forEach( ( component ) => {
            args.push( component );
        } );
        
        
        let inbound = this.run_engine( args );


        echo.push( {
            info: "Inbound", data: inbound
        } );

        echo.release();


        this.execute_chain(
            inbound,
            msg.guild, msg.author,
            msg
        );
    } 

    /**
    @param { VoiceState } old_state
    @param { VoiceState } new_state
    **/
    on_voice_state( old_state, new_state ) {
        let echo = new Echo( [
            { info: "VoiceStateUpdate", data: `At ${ new Date().toLocaleTimeString() }` },
            { info: "In", data: { guild: new_state.guild.name, id: new_state.guild.id } },
            { info: "By", data: { user: new_state.member.user.username, id: new_state.member.user.id } }
        ] );


        let args = this.make_front( 
            OUTBOUND_VOICE_UPDATE, 
            new_state.guild, 
            new_state.member.user
        );

        args.push( old_state.channelId ?? "" );
        args.push( new_state.channelId ?? "" );

        let inbound = this.run_engine( args );


        echo.push( {
            info: "Inbound", data: inbound
        } );

        echo.release();


        this.execute_chain(
            inbound,
            new_state.guild, new_state.member.user,
            new_state
        );
    }


    on_tick = ( guild, type ) => {
        let echo = new Echo( [
            { info: "Tick", data: `At ${ new Date().toLocaleTimeString() }` },
            { info: "In", data: { guild: guild.name, id: guild.id } }
        ] );

        let args = this.make_front( OUTBOUND_TICK, guild, null );
        args.push( type );

        let inbound = this.run_engine( args );


        echo.push( {
            info: "Inbound", data: inbound
        } );

        echo.release();


        this.execute_chain( inbound, null, null, null );
    }

//#endregion


//#region Exe

    make_front = ( event, guild, user ) => {
        let args = [ event, guild.id ];

        let voice = this.voice_in( guild.id );

        args.push( voice ? voice.channel.id : "" );

        if( !user ) {
            for( let n = 1; n <= 2; ++n )
                args.push( "" );
            
            return args;
        } 

        args.push( user.id );

        voice = voice_of( user, guild );

        args.push( voice.channel ? voice.channel.id : ""  );

        return args;
    }

    run_engine( args ) {
        try {
            return execFileSync( 
                config.exe, 
                args
            ).toString();

        } catch( fault ) {
            console.log( 
                "\x1b[31mEngine returned: %d\nWhat():\n%s\x1b[37m",
                fault.status,
                fault.stderr.toString()
             );

            return "ENGINE_FAULT";
        }
    }

    /**
    @param { string } chain 
    @param { Guild } guild 
    @param { User } user 
    @param { * } payload 
    @param { string[] } ins 
    **/
    execute_chain( chain, guild, user, payload ) {
        chain.split( INBOUND_HIGH_SPLIT ).forEach( ( high ) => {
            let lows = high.split( INBOUND_LOW_SPLIT );

            this.execute_link( lows.at( 0 ), guild, user, payload, lows.slice( 1 ) );
        } );
    }

    /**
    @param { string }   type 
    @param { Guild }    guild 
    @param { User }     user
    @param { * }        payload 
    @param { string[] } ins 
    **/
    execute_link = async ( type, guild, user, payload, ins ) => {
        switch( type ) {
            case INBOUND_SCRIPT: {
                eval( ins.at( 0 ) );

                break; }



            case INBOUND_REPLY_MESSAGE: {
                payload.reply( ins.at( 0 ) );

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

                payload.reply( {
                    files: [ path ], embeds: [ embed ]
                } );

                break; }



            case INBOUND_VOICE_CONNECT: {  
                if( ins.at( 0 ) == NULL_BOUND ) break;

                let channel;
                
                try {
                    channel = await this.client.channels.fetch( ins.at( 0 ) )
                } catch( fault ) {
                    payload.reply( ins.at( 0 ) );

                    break;
                }

                if( !channel ) break;

                let idx = this.voices.findIndex( voice => { 
                    return voice.channel.id == channel.id; 
                } );

                if( idx >= 0 )
                    this.voices.splice( idx, 1 );

                this.voices.push( new Voice( channel ) );

                break; }

            case INBOUND_VOICE_DISCONNECT: {
                let idx = this.voices.findIndex( voice => {
                    return voice.guild.id == ins.at( 0 );
                } );

                if( idx < 0 ) break;

                this.voices[ idx ].connection.destroy();

                this.voices.splice( idx, 1 );

                
                break; }

            case INBOUND_VOICE_PLAY: {
                let voice = this.voice_in( ins.at( 0 ) );

                if( !voice ) break;

                voice.play( ins.at( 1 ) );


                break; }

            case INBOUND_VOICE_STOP: {
                let voice = this.voice_in( ins.at( 0 ) );

                if( !voice ) break;

                voice.stop();
                

                break; }

            

            case INBOUND_TICK_GUILD_SET: {
                let tick_id  = this.pull_tick_id();
                let guild_id = ins.at( 0 );
                let delay    = parseInt( ins.at( 1 ) ) * 1000;
                let type     = ins.at( 2 );

                this.ticks.set(
                    tick_id, 

                    {
                        guild_id: guild_id,

                        type: type,

                        timeout: setTimeout( () => {
                            this.on_tick( this.pull_guild( guild_id ), type );
                            
                            this.ticks.delete( tick_id );

                        }, delay )
                    }
                );

                break; }

            case INBOUND_TICK_GUILD_RESET: {
                let guild_id = ins.at( 0 );
                let type     = ins.at( 1 );


                let matches = [];

                this.ticks.forEach( ( value, key ) => {
                    if( !( value.guild_id == guild_id && value.type == type ) ) return;

                    clearTimeout( value.timeout );
                        
                    matches.push( key );
                } );

                matches.forEach( key => {
                    this.ticks.delete( key );
                } );


                this.on_tick( this.pull_guild( guild_id ), type );

                break; }


            
            case INBOUND_PYTHON_RESPONSE: {
                let response = await this.python_response( ins.at( 0 ) );

                if( !response ) break;

                payload.reply( response );

                break; }
        }
    }

//#endregion


//#region Utility

    /**
     @param { Guild } guild 
     @returns { Voice }
    **/
    voice_in( guild_id ) {
        let found = null;

        this.voices.forEach( ( voice ) => {
            if( voice.guild.id == guild_id )
                found = voice;
        } );

        return found;
    }

    payload_of( guild_id ) {
        let found = null;

        this.g_threads.forEach( ( thread ) => {
            if( thread.guild.id == guild_id )
                found = thread;
        } );

        return found;
    }

    pull_tick_id = () => {
        return this.next_tick_id++;
    }

    pull_guild = ( guild_id ) => {
        return this.client.guilds.cache.find( guild => guild.id == guild_id );
    }

//#endregion Utility


    guilds_launch_ticks = () => {
        this.client.guilds.cache.forEach( guild => {
            this.on_tick( guild, OUTBOUND_TICK_INIT );
        } );
    }


    python_response = async ( str ) => {
        let sync = new Promise( ( resolve, reject ) => {
            this.python_script.stdout.on( "data", ( response ) => {
                resolve( response.toString() );
            } );

            this.python_script.stdin.write( str + "\n" );
        } );
        
        return await sync;
    }
};



var ahri = new Engine();



