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


const OUTBOUND_MESSAGE = "message";
const OUTBOUND_VOICE_UPDATE = "voice_update";
const OUTBOUND_TICK = "tick";

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

        this.g_threads = [];


        this.client.login( config.token )
        .then( () => {
            this.guilds_launch_threads();
        } );
    }

    
//#region Ons

    on_ready() {
        console.log( `All nine tails ready to go. ${ this.client.user.tag } reporting in.\n\n` );
    
        this.client.user.setActivity( {
            type: 0,
            name: "in the wilds."
        } );
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


    on_tick = ( tick ) => {
        let echo = new Echo( [
            { info: "Tick", data: `At ${ new Date().toLocaleTimeString() }` },
            { info: "In", data: { guild: tick.guild.name, id: tick.guild.id } }
        ] );


        let inbound = this.run_engine( this.make_front( OUTBOUND_TICK, tick.guild, null ) );


        echo.push( {
            info: "Inbound", data: inbound
        } );

        echo.release();


        this.execute_chain( inbound, null, null, tick );
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
    @param { * } xtra 
    @param { string[] } ins 
    **/
    execute_chain( chain, guild, user, xtra ) {
        chain.split( INBOUND_HIGH_SPLIT ).forEach( ( high ) => {
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
    execute_link = async ( type, guild, user, xtra, ins ) => {
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
                if( ins.at( 0 ) == NULL_BOUND ) break;

                let channel;
                
                try {
                    channel = await this.client.channels.fetch( ins.at( 0 ) )
                } catch( fault ) {
                    xtra.reply( ins.at( 0 ) );

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
                if( xtra instanceof Message )
                    xtra = this.payload_of( guild.id );

                if( !xtra ) break;
                    
                xtra.delay = parseInt( ins.at( 0 ) );

                clearTimeout( xtra.timeout );

                if( xtra.resolve ) xtra.resolve();
                if( xtra.unlock ) xtra.unlock();

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

//#endregion Utility


    guilds_launch_threads = () => {
        this.client.guilds.cache.forEach( guild => {
            this.g_threads.push( {
                guild:   guild,
                delay:   4,
                unlock:  null,
                timeout: null,
                resolve: null
            } );

            this.guild_thread( this.g_threads.at( -1 ) );
        } );
    }

    guild_thread = async ( payload ) => {
        while( true ) {
            await new Promise( resolve => {
                payload.resolve = resolve;
                payload.timeout = setTimeout( resolve, payload.delay * 1000 );
            } );

            let sync = new Promise( resolve => {
                payload.unlock = resolve;
            } );

            this.on_tick( payload );

            await sync;
        }
    }
};



var ahri = new Engine();


