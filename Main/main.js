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


        let args = [
            "message",
            msg.guildId,
            msg.author.id,
        ];

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


        let inbound = this.run_engine( [
            "voice_update",
            new_state.guild.id,
            new_state.member.user.id,
            old_state.channelId ?? "",
            new_state.channelId ?? ""
        ] );


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


        let inbound = this.run_engine( [
            OUTBOUND_TICK,
            tick.guild.id
        ] );


        echo.push( {
            info: "Inbound", data: inbound
        } );

        echo.release();


        this.execute_chain( inbound, null, null, tick );
    }

//#endregion


//#region Exe

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
                xtra.delay = parseInt( ins.at( 0 ) );

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
    voice_in( guild_id ) {
        let found = null;

        this.voices.forEach( ( voice ) => {
            if( voice.guild.id == guild_id )
                found = voice;
        } );

        return found;
    }

//#endregion Utility


    guilds_launch_threads = () => {
        this.client.guilds.cache.forEach( guild => {
            this.guild_thread( guild );
        } );
    }

    guild_thread = async ( guild ) => {
        let tick = {
            guild: guild,
            delay: 6,
            unlock: null
        };

        while( true ) {
            await wait_for( tick.delay * 1000 );

            tick.unlock = null;

            let sync = new Promise( resolve => {
                tick.unlock = resolve;
            } );

            this.on_tick( tick );

            await sync;
        }
    }
};



var ahri = new Engine();


