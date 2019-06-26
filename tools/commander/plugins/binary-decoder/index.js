/****************************************************************************
 *
 *   Copyright (c) 2018 Windhover Labs, L.L.C. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name Windhover Labs nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

'use strict';

const Parser = require( 'binary-parser' ).Parser;
const net = require( 'net' );
const Emitter = require( 'events' );
const fs = require( 'fs' );
const util = require( 'util' );
const Promise = require( 'promise' );
const mergeJSON = require( 'merge-json' );
const convict = require( 'convict' );
const config = require( './config.js' );
const Int64LE = require( 'int64-buffer' ).Int64LE;
const Int64BE = require( 'int64-buffer' ).Int64BE;
const Uint64LE = require( 'int64-buffer' ).Uint64LE;
const Uint64BE = require( 'int64-buffer' ).Uint64BE;
const path = require( 'path' );
const Long = require( 'long' );
const autoBind = require('auto-bind');
const CdrGroundPlugin = require(path.join(global.CDR_INSTALL_DIR, '/commander/classes/CdrGroundPlugin')).CdrGroundPlugin;



class BinaryDecoder extends CdrGroundPlugin {
	/**
	 * Constructor for binary decoder
	 * @param       {String} workspace  path to commander workspace
	 * @param       {String} configFile path to binary-decoder-config.json
	 * @constructor
	 */
    constructor(configObj) {
        configObj.webRoot = path.join( __dirname, 'web');  
        super(configObj); 
        
        autoBind(this);
        
    	var self = this;
        this.defs;
        this.workspace = configObj.workspace;
        this.cmdHeaderLength = 64;
        this.sequence = 0;
        this.cdd = {};
        this.endian;
        this.namespace = configObj.namespace;
        this.name = configObj.name;

        /* Initialize the configuration. */
        this.initConfig(configObj.name, configObj.configFile);
        
        /* Initialize server side housekeeping telemetry that we'll publish 
         * later. */
        this.initTelemetry();

        /* Initialize client side interface. */
        this.initClientInterface();

        /* Initialize server side commands. */
        this.initCommands();

        var inMsgDefs = config.get( 'msgDefs' )

        for ( var i = 0; i < inMsgDefs.length; ++i ) {
            if ( typeof process.env.AIRLINER_MSG_DEF_PATH === 'undefined' ) {
                var fullPath = path.join( this.workspace, config.get( 'msgDefPath' ), inMsgDefs[ i ].file );
            } else {
                var fullPath = path.join( process.env.AIRLINER_MSG_DEF_PATH, inMsgDefs[ i ].file );
            }
            var msgDefInput = JSON.parse( fs.readFileSync( fullPath, 'utf8' ) );
            this.defs = mergeJSON.merge( this.defs, msgDefInput );
        }

        if ( this.defs.Airliner.little_endian ) {
            this.endian = 'little';
        } else {
            this.endian = 'big';
        }

        this.ccsdsPriHdr = new Parser()
            .endianess( 'big' )
            .bit3( 'version' )
            .bit1( 'pktType' )
            .bit1( 'secHdr' )
            .bit11( 'apid' )
            .bit2( 'segment' )
            .bit14( 'sequence' )
            .uint16( 'length' );

        if ( this.endian == 'little' ) {
            this.ccsdsCmdSecHdr = new Parser()
                .endianess( 'little' )
                .bit1( 'reserved' )
                .bit7( 'code' )
                .uint8( 'checksum' );
        } else {
            this.ccsdsCmdSecHdr = new Parser()
                .endianess( 'big' )
                .uint8( 'checksum' )
                .bit1( 'reserved' )
                .bit7( 'code' )
        }

        switch ( config.get( 'CFE_SB_PACKET_TIME_FORMAT' ) ) {
            case 'CFE_SB_TIME_32_16_SUBS':
                this.ccsdsTlmSecHdr = new Parser()
                    .endianess( this.endian )
                    .uint32( 'seconds' )
                    .uint16( 'subseconds' );
                this.tlmHeaderLength = 96;
                break;

            case 'CFE_SB_TIME_32_32_SUBS':
                this.ccsdsTlmSecHdr = new Parser()
                    .endianess( this.endian )
                    .uint32( 'seconds' )
                    .uint32( 'subseconds' );
                this.tlmHeaderLength = 98;
                break;

            case 'CFE_SB_TIME_32_32_M_20':
                this.ccsdsTlmSecHdr = new Parser()
                    .endianess( this.endian )
                    .uint32( 'seconds' )
                    .uint32( 'subseconds' );
                this.tlmHeaderLength = 98;
            break;

            default:
                break;
        }

        this.ccsds = new Parser()
            .endianess( this.endian )
            .nest( 'PriHdr', {
                    type: this.ccsdsPriHdr
                } )
            .choice( 'SecHdr', {
                    tag: 'PriHdr.pktType',
                    choices: {
                        0: this.ccsdsTlmSecHdr,
                        1: this.ccsdsCmdSecHdr
                    }
                } )
            .buffer( 'payload', {
                    readUntil: 'eof'
                } );
        
        this.namespace.recv( config.get( 'binaryInputStreamID' ), this.processBinaryMessage);

        this.namespace.recv( config.get( 'tlmDefReqStreamID' ), this.processTlmDefReq);
        
        this.logInfo('Initialized');
    };
    
    
    processTlmDefReq( tlmReqs, cb ) {
//        var found = false;
//        
//        var self = this;
//        
//        if ( typeof tlmReqs.length === 'number' ) {
//            /* This must be an array. */
//            var outTlmDefs = [];
//            for ( var i = 0; i < tlmReqs.length; ++i ) {
//                var tlmDef = self.getTlmDefByName( self.stripArrayIdentifiers( tlmReqs[ i ].name ) );
//                if ( typeof tlmDef !== 'undefined' ) {
//                    tlmDef.opsPath = tlmReqs[ i ].name;
//                    outTlmDefs.push( tlmDef );
//                    found = true;
//                } else {
//                    self.logDebug( 'TlmDefReq: Telemetry not found.  \'' + tlmReqs[ i ].name + '\'' );
//                }
//            }
//            
//            if(found == true) {
//                cb( outTlmDefs );
//            }
//        } else {
//            /* This is a single request. */
//            var tlmDef = self.getTlmDefByName( self.stripArrayIdentifiers( tlmReqs.name ) );
//            if ( typeof tlmDef === 'undefined' ) {
//                self.logDebug('TlmDefReq: Telemetry not found.  \'' + tlmReqs.name + '\'' );
//            } else {
//                tlmDef.opsPath = tlmReqs.name;
//                found = true;
//            }
//            
//            if(found == true) {
//                cb( tlmDef );
//            }
//        }
    };



    /**
     * Gets telemetry definition by name
     * @param  {String} name telemetry operation name
     * @return {Object}      telemetry definition object
     */
    getTlmDefByName( name ) {
//        var fieldDef = this.getTlmDefByPath( name );
//
//        if ( typeof fieldDef === 'undefined' ) {
//            return undefined;
//        } else {
//            var outTlmDef = {
//                    opsPath: name
//            };
//            switch ( fieldDef.airliner_type ) {
//            case 'char':
//            case 'uint8':
//            case 'int8':
//            case 'string':
//            case 'uint16':
//            case 'int16':
//            case 'uint32':
//            case 'int32':
//            case 'float':
//            case 'double':
//            case 'boolean':
//            case 'uint64':
//            case 'int64':
//                outTlmDef.dataType = fieldDef.airliner_type;
//                break;
//
//            default:
//                switch ( fieldDef.pb_type ) {
//                case 'char':
//                case 'uint8':
//                case 'int8':
//                case 'string':
//                case 'uint16':
//                case 'int16':
//                case 'uint32':
//                case 'int32':
//                case 'float':
//                case 'double':
//                case 'boolean':
//                case 'uint64':
//                case 'int64':
//                    outTlmDef.dataType = fieldDef.pb_type;
//                    break;
//
//                default:
//                    // console.log( "Data type not found" );
//                    outTlmDef.dataType = undefined;
//                }
//            }
//
//            if ( fieldDef.count != 0 ) {
//                outTlmDef.arrayLength = fieldDef.count;
//            }
//
//            return outTlmDef;
//        }
    }


    /**
     * Parse and return app name
     * @param  {String} path telemetry path
     * @return {String}      App name
     */
    getAppNameFromPath( path ) {
        if ( typeof path === 'string' ) {
            var splitName = path.split( '/' );
            return splitName[ 1 ];
        }
        return undefined;
    }


    /**
     * Parse and return operation name
     * @param  {String} path telemetry path
     * @return {String}      Operation name
     */
    getOperationFromPath( path ) {
        if ( typeof path === 'string' ) {
            var splitName = path.split( '/' );
            return splitName[ 2 ];
        }
        return undefined;
    }


    /**
     * Gets application definition from application name
     * @param  {String} appName application name
     * @return {Object}         application object
     */
    getAppDefinition( appName ) {
    	return this.defs.Airliner.modules[appName];
    }


    /**
     * Gets telemetry definition by telemetry path
     * @param  {String} path telemetry path
     * @return {Object}      telemetry definition
     */
    getTlmDefByPath( path ) {
//        var self = this;
//        var appName = self.getAppNameFromPath( path );
//        var operationName = self.getOperationFromPath( path );
//        if ( typeof operationName === 'undefined' ) {
//            this.logDebug( 'getTlmDefByPath: Ops path not found. \'' + path + '\'' );
//            return undefined;
//        } else {
//            var appDefinition = this.getAppDefinition( appName );
//
//            if ( typeof appDefinition === 'undefined' ) {
//                this.logDebug( 'getTlmDefByPath: App not found. \'' + appName + '\'' );
//                return undefined;
//            } else {
//                var operation = appDefinition.operations[ operationName ];
//                if ( typeof operation !== 'undefined' ) {
//                    if ( operation.hasOwnProperty( 'airliner_msg' ) ) {
//                        var msgDef = this.getMsgDefByName( operation.airliner_msg );
//
//                        if ( typeof msgDef === 'undefined' ) {
//                            return undefined;
//                        } else {
//                            var splitName = path.split( '/' );
//                            var opNameID = self.stripArrayIdentifier( splitName[ 3 ] );
//
//                            var fieldObj = msgDef.operational_names[ opNameID ];
//
//                            if ( typeof fieldObj === 'undefined' ) {
//                                return undefined;
//                            } else {
//                                var fieldPath = fieldObj.field_path;
//
//                                if ( typeof fieldPath === 'undefined' ) {
//                                    return undefined;
//                                } else {
//                                    var fieldDef = self.getFieldFromOperationalName( msgDef, fieldPath, 0 );
//
//                                    return fieldDef.fieldDef;
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
    }


    /**
     * Checks if opName is a string array
     * @param  {String} opName operation name
     * @return {Boolean}        true if opName is a string array otherwise false
     */
    isOpNameAnArray( opName ) {
        if ( typeof opName == 'string' ) {
            var start = opName.indexOf( '[' );

            if ( start > 0 ) {
                var end = opName.indexOf( ']' );

                if ( end > start ) {
                    return true;
                }
            }
        }
        return false;
    }


    /**
     * String array for telemetry identifier
     * @param  {String} opName operation name
     * @return {String}        indentifier
     */
    stripArrayIdentifier( opName ) {
        var self = this;
        if ( self.isOpNameAnArray( opName ) == true ) {
            var start = 0;
            var end = opName.indexOf( '[' );

            if ( end > 0 ) {
                var outString = opName.substring( start, end );
                return outString;
            }
        }
        return opName;
    }


    /**
     * String array for telemetry identifiers
     * @param  {String} opName operation name
     * @return {String}        indentifiers
     */
    stripArrayIdentifiers( opName ) {
        var self = this;
        if ( typeof opName == 'string' ) {
            var splitPath = opName.split( '.' );

            for ( var i = 0; i < splitPath.length; ++i ) {
                splitPath[ i ] = self.stripArrayIdentifier( splitPath[ i ] );
            }
            return splitPath.join( '.' );
        }
        return undefined;


    }



    /**
     * Get telemetry definiton by telemetry name
     * @param  {String} msgName message name
     * @return {Object}         telemetry defintion
     */
    getMsgDefByName( msgName ) {
        var self = this;
        var msgDef = undefined;

        try {
        	for(var appID in this.defs.Airliner.modules) {
                var app = this.defs.Airliner.modules[appID];
                
                if(app.symbols.hasOwnProperty(msgName)) {
                	msgDef = app.symbols[msgName];
                }
        	}
        } catch ( e ) {
            self.logError('getMsgDefByName: Cannot get definition by name.' );
        }
        
        return msgDef;
    }


    
    /**
     * Get telemetry definition by message id
     * @param  {Number} msgID message id
     * @return {Object}       telemetry definition object
     */
    getMsgDefByMsgID( msgID ) {
        var self = this;
        var msgDefOut = undefined;
        
        try {
        	for(var appID in this.defs.Airliner.modules) {
                var app = this.defs.Airliner.modules[appID];

            	for(var symbolName in app.symbols) {
            		var msgDef = app.symbols[symbolName];
                    if(msgDef.hasOwnProperty('msg_id')) {
                	    if(msgDef.msg_id == msgID) {
                	    	msgDefOut = msgDef;
                	    	msgDefOut.symbol = symbolName;
                	    	msgDefOut.opsPath = '/' + appID + '/' + symbolName;
                	    }
                    }
            	}
        	}
        } catch ( e ) {
            self.logError('getMsgDefByMsgID: Cannot get definition by msgID.' );
        }
        
        return msgDefOut;
    }
    


    /**
     * Gets field from operation name
     * @param  {Object} msgDef    message def
     * @param  {string} opName    operation name
     * @param  {Number} bitOffset bit offset
     * @return {Object}           field object
     */
    getFieldFromOperationalName( msgDef, opName, bitOffset ) {
//        var self = this;
//        var pbMsg = undefined;
//        try {
//            var op = msgDef.operational_names[ opName ];
//            var fieldPathArray = opName.split( '.' );
//            pbMsg = self.getFieldObjFromPbMsg( msgDef, fieldPathArray, bitOffset );
//        } catch ( e ) {
//            self.logError('getFieldFromOperationalName: Unable to retrive field.  \'' + e + '\'' );
//        }
//        return pbMsg;
    }


    /**
     * Processes binary message
     * @param  {Object} buffer buffer object
     */
    processBinaryMessage( buffer ) {
        var self = this;
        
        try {
            self.hk.content.msgRecvCount++;
            
            var msgID = buffer.readUInt16BE( 0 );

            var message = this.ccsds.parse( buffer );

            var msgTime = this.cfeTimeToJsTime( message.SecHdr.seconds, message.SecHdr.subSeconds );

            var msgDef = this.getMsgDefByMsgID( msgID );

            var tlmObj = {};

            if ( typeof msgDef !== 'undefined' ) {
                var fields = msgDef.fields;

                for(var i = 0; i < fields.length; ++i) {
                    var field = fields[i];
                    var fieldName = field.name;
                    try {
                        tlmObj[ fieldName ] = this.getFieldValue( buffer, field, field.bit_offset, msgDef );

                        self.hk.content.paramsParsed++;
                    } catch ( e ) {
                       /* TODO: Do nothing for now. */
                    }
                }

                var output = {
                        content: tlmObj,
                        opsPath: msgDef.opsPath,
                        symbol: msgDef.symbol,
                        msgID: msgID,
                        msgTime: msgTime
                };
                                        
                this.namespace.send( config.get( 'jsonOutputStreamID' ), output );
            }
        } catch ( e ) {
            self.logError('processBinaryMessage: Unable to decode buffer.  \'' + e + '\'' );
        }
    };
    


    /**
     * Gets the telemetry field value
     * @param  {Object} buffer    buffer object
     * @param  {Object} fieldDef  field definition
     * @param  {Object} bitOffset bit offset
     * @param  {Object} rootDef   root definition
     * @return {Object}           value object
     */
    getFieldValueAsBaseType( buffer, fieldDef, bitOffset, rootDef ) {
        try {
            var value;

            if(fieldDef.array == true) {
                var value = [];
                switch ( fieldDef.type.base_type ) {
	                case 'unsigned char':
	                    if(buffer.length >= (( bitOffset / 8 ) + fieldDef.count )) {
	                        /* This field is not truncated. */
	                        fieldLength = fieldDef.count;
	                    } else {
	                        /* This field is truncated. */
	                        fieldLength = ( buffer.length - ( bitOffset / 8 ) );
	                    }
	
	                    for ( var i = 0; i < fieldLength; ++i ) {
	                        value.push( buffer.readUInt8( ( bitOffset / 8 ) + i ) );
	                    }
	                    break;

	                case 'char':
	                    /* I wish there was a way around this, but we cannot automatically
	                     * determine the difference between a string and char array.  
	                     * Therefore, we will treat everything as a string.  However, some
	                     * messages may really have char arrays containing binary data.
	                     * To make it worse, some of these are actually variable size at 
	                     * runtime.  In other words, the flight software truncates them.  
	                     * So, to accomodate those, we are going to try to read the entire
	                     * length, or up to the end of the buffer.
	                     */
	                    var fieldLength = 0;
	                    if(buffer.length >= (( bitOffset / 8 ) + fieldDef.count )) {
	                        /* This field is not truncated. */
	                        fieldLength = fieldDef.count;
	                    } else {
	                        /* This field is truncated. */
	                        fieldLength = ( buffer.length - ( bitOffset / 8 ) );
	                    }
	
	                    value = buffer.toString( 'utf8', bitOffset / 8, fieldLength );
	                    break;
	
	                case 'short unsigned int':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        if ( this.endian == 'little' ) {
	                            value.push( buffer.readUInt16LE( ( bitOffset / 8 ) + ( i * 2 ) ) );
	                        } else {
	                            value.push( buffer.readUInt16BE( ( bitOffset / 8 ) + ( i * 2 ) ) );
	                        }
	                    }
	                    break;
	
	                case 'short int':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        if ( this.endian == 'little' ) {
	                            value.push( buffer.readInt16LE( ( bitOffset / 8 ) + ( i * 2 ) ) );
	                        } else {
	                            value.push( buffer.readInt16BE( ( bitOffset / 8 ) + ( i * 2 ) ) );
	                        }
	                    }
	                    break;
	
	                case 'unsigned int':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        if ( this.endian == 'little' ) {
	                            value.push( buffer.readUInt32LE( ( bitOffset / 8 ) + ( i * 4 ) ) );
	                        } else {
	                            value.push( buffer.readUInt32BE( ( bitOffset / 8 ) + ( i * 4 ) ) );
	                        }
	                    }
	                    break;
	
	                case 'int':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        if ( this.endian == 'little' ) {
	                            value.push( buffer.readInt32LE( ( bitOffset / 8 ) + ( i * 4 ) ) );
	                        } else {
	                            value.push( buffer.readInt32BE( ( bitOffset / 8 ) + ( i * 4 ) ) );
	                        }
	                    }
	                    break;
	
	                case 'float':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        if ( this.endian == 'little' ) {
	                            value.push( buffer.readFloatLE( ( bitOffset / 8 ) + ( i * 4 ) ) );
	                        } else {
	                            value.push( buffer.readFloatBE( ( bitOffset / 8 ) + ( i * 4 ) ) );
	                        }
	                    }
	                    break;
	
	                case 'double':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        if ( this.endian == 'little' ) {
	                            value.push( buffer.readDoubleLE( ( bitOffset / 8 ) + ( i * 8 ) ) );
	                        } else {
	                            value.push( buffer.readDoubleBE( ( bitOffset / 8 ) + ( i * 8 ) ) );
	                        }
	                    }
	                    break;

	                case 'long unsigned int':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
   	                        if ( this.endian == 'little' ) {
	                            var uint64Value = new Uint64LE( buffer, ( bitOffset / 8 ) + ( i * 8 ) );
	                        } else {
	                            var uint64Value = new Uint64BE( buffer, ( bitOffset / 8 ) + ( i * 8 ) );
	                        }
	                        value.push(uint64Value.toString( 10 ));
	                    }
	                    break;

	                case 'long int':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
   	                        if ( this.endian == 'little' ) {
	                            var int64Value = new Int64LE( buffer, ( bitOffset / 8 ) + ( i * 8 ) );
	                        } else {
	                            var int64Value = new Int64BE( buffer, ( bitOffset / 8 ) + ( i * 8 ) );
	                        }
	                        value.push(int64Value.toString( 10 ));
	                    }
	                    break;
	
	                default:
	                	console.log(fieldDef);
//	                    var nextFieldDef = this.getMsgDefByName( fieldDef.airliner_type );
//	
//		                if ( typeof nextFieldDef === 'undefined' ) {
//		                    value = this.getFieldValueAsPbType( buffer, fieldDef, bitOffset, rootDef );
//		                } else {
//		                    var nextFields = nextFieldDef.fields;
//		                    var value = [];
//		                    for ( var i = 0; i < fieldDef.count; ++i ) {
//		                        var nextValue = {};
//		                        var elementBitSize = fieldDef.bit_size / fieldDef.count;
//		                        var nextBitOffset = bitOffset + ( ( fieldDef.bit_size / fieldDef.count ) * i );
//		
//		                        for ( var fieldName in nextFields ) {
//		                            var nextField = nextFields[ fieldName ];
//		                            nextValue[ fieldName ] = this.getFieldValue( buffer, nextField, nextField.bit_offset + nextBitOffset, nextFieldDef );
//		                        }
//		                        value.push( nextValue );
//		                    }
//		                }
//                    }
                }
            } else {
                switch ( fieldDef.base_type ) {              	
	                case 'unsigned char':
	                case 'char':
	                    value = buffer.readUInt8( bitOffset / 8 );
	                    break;
	
	                case 'int8':
	                    value = buffer.readInt8( bitOffset / 8 );
	                    break;
	
	                case 'short unsigned int':
	                    if ( this.endian == 'little' ) {
	                        value = buffer.readUInt16LE( bitOffset / 8 );
	                    } else {
	                        value = buffer.readUInt16BE( bitOffset / 8 );
	                    }
	                    break;
	
	                case 'short int':
	                    if ( this.endian == 'little' ) {
	                        value = buffer.readInt16LE( bitOffset / 8 );
	                    } else {
	                        value = buffer.readInt16BE( bitOffset / 8 );
	                    }
	                    break;
	
	                case 'unsigned int':
	                    if ( this.endian == 'little' ) {
	                        value = buffer.readUInt32LE( bitOffset / 8 );
	                    } else {
	                        value = buffer.readUInt32BE( bitOffset / 8 );
	                    }
	                    break;
	
	                case 'int':
	                    if ( this.endian == 'little' ) {
	                        value = buffer.readInt32LE( bitOffset / 8 );
	                    } else {
	                        value = buffer.readInt32BE( bitOffset / 8 );
	                    }
	                    break;
	
	                case 'float':
	                    if ( this.endian == 'little' ) {
	                        value = buffer.readFloatLE( bitOffset / 8 );
	                    } else {
	                        value = buffer.readFloatBE( bitOffset / 8 );
	                    }
	                    break;
	
	                case 'double':
	                    if ( this.endian == 'little' ) {
	                        value = buffer.readDoubleLE( bitOffset / 8 );
	                    } else {
	                        value = buffer.readDoubleBE( bitOffset / 8 );
	                    }
	                    break;

	                case 'long unsigned int':
	                    if ( this.endian == 'little' ) {
	                        var uint64Value = new Uint64LE( buffer, bitOffset / 8 );
	                    } else {
	                        var uint64Value = new Uint64BE( buffer, bitOffset / 8 );
	                    }
	                    value = uint64Value.toString( 10 );
	                    break;
	
	                case 'long int':
	                    if ( this.endian == 'little' ) {
	                        var int64Value = new Int64LE( buffer, bitOffset / 8 );
	                    } else {
	                        var int64Value = new Int64BE( buffer, bitOffset / 8 );
	                    }
	                    value = int64Value.toString( 10 );
	                    break;
	
	                default:
	                    console.log('Type ' + fieldDef.base_type + ' not found.');
                }
            }
        } catch ( err ) {
            this.logError('getFieldValue: Unhandled exception. \'' + err + '\'' );
        }
        return value;
    }
    


    /**
     * Gets the telemetry field value
     * @param  {Object} buffer    buffer object
     * @param  {Object} fieldDef  field definition
     * @param  {Object} bitOffset bit offset
     * @param  {Object} rootDef   root definition
     * @return {Object}           value object
     */
    getFieldValue( buffer, fieldDef, bitOffset, rootDef ) {
        try {
            var value;

            if(fieldDef.array == true) {
                var value = [];
                
                switch ( fieldDef.real_type ) {
	                case 'uint8':
	                    if(buffer.length >= (( bitOffset / 8 ) + fieldDef.count )) {
	                        /* This field is not truncated. */
	                        fieldLength = fieldDef.count;
	                    } else {
	                        /* This field is truncated. */
	                        fieldLength = ( buffer.length - ( bitOffset / 8 ) );
	                    }
	
	                    for ( var i = 0; i < fieldLength; ++i ) {
	                        value.push( buffer.readUInt8( ( bitOffset / 8 ) + i ) );
	                    }

	                    break;
	
	                case 'int8':
	                    if(buffer.length >= (( bitOffset / 8 ) + fieldDef.count )) {
	                        /* This field is not truncated. */
	                        fieldLength = fieldDef.count;
	                    } else {
	                        /* This field is truncated. */
	                        fieldLength = ( buffer.length - ( bitOffset / 8 ) );
	                    }
	                    for ( var i = 0; i < fieldLength; ++i ) {
	                        value.push( buffer.readInt8( ( bitOffset / 8 ) + i ) );
	                    }
	                    break;
	
	                case 'string':
	                case 'char':
	                    /* I wish there was a way around this, but we cannot automatically
	                     * determine the difference between a string and char array.  
	                     * Therefore, we will treat everything as a string.  However, some
	                     * messages may really have char arrays containing binary data.
	                     * To make it worse, some of these are actually variable size at 
	                     * runtime.  In other words, the flight software truncates them.  
	                     * So, to accomodate those, we are going to try to read the entire
	                     * length, or up to the end of the buffer.
	                     */
	                    var fieldLength = 0;
	                    if(buffer.length >= (( bitOffset / 8 ) + fieldDef.count )) {
	                        /* This field is not truncated. */
	                        fieldLength = fieldDef.count;
	                    } else {
	                        /* This field is truncated. */
	                        fieldLength = ( buffer.length - ( bitOffset / 8 ) );
	                    }
	
	                    value = buffer.toString( 'utf8', bitOffset / 8, fieldLength ).replace(/\0[\s\S]*$/g,'');
	                    
	                    break;
	
	                case 'uint16':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        if ( this.endian == 'little' ) {
	                            value.push( buffer.readUInt16LE( ( bitOffset / 8 ) + ( i * 2 ) ) );
	                        } else {
	                            value.push( buffer.readUInt16BE( ( bitOffset / 8 ) + ( i * 2 ) ) );
	                        }
	                    }
	                    break;
	
	                case 'int16':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        if ( this.endian == 'little' ) {
	                            value.push( buffer.readInt16LE( ( bitOffset / 8 ) + ( i * 2 ) ) );
	                        } else {
	                            value.push( buffer.readInt16BE( ( bitOffset / 8 ) + ( i * 2 ) ) );
	                        }
	                    }
	                    break;
	
	                case 'uint32':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        if ( this.endian == 'little' ) {
	                            value.push( buffer.readUInt32LE( ( bitOffset / 8 ) + ( i * 4 ) ) );
	                        } else {
	                            value.push( buffer.readUInt32BE( ( bitOffset / 8 ) + ( i * 4 ) ) );
	                        }
	                    }
	                    break;
	
	                case 'int32':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        if ( this.endian == 'little' ) {
	                            value.push( buffer.readInt32LE( ( bitOffset / 8 ) + ( i * 4 ) ) );
	                        } else {
	                            value.push( buffer.readInt32BE( ( bitOffset / 8 ) + ( i * 4 ) ) );
	                        }
	                    }
	                    break;
	
	                case 'float':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        if ( this.endian == 'little' ) {
	                            value.push( buffer.readFloatLE( ( bitOffset / 8 ) + ( i * 4 ) ) );
	                        } else {
	                            value.push( buffer.readFloatBE( ( bitOffset / 8 ) + ( i * 4 ) ) );
	                        }
	                    }
	                    break;
	
	                case 'double':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        if ( this.endian == 'little' ) {
	                            value.push( buffer.readDoubleLE( ( bitOffset / 8 ) + ( i * 8 ) ) );
	                        } else {
	                            value.push( buffer.readDoubleBE( ( bitOffset / 8 ) + ( i * 8 ) ) );
	                        }
	                    }
	                    break;
	
	                case 'boolean':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        value.push( buffer.readUInt8( ( bitOffset / 8 ) + ( i * 4 ) ) );
	                    }
	                    break;
	
	                case 'uint64':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        if ( this.endian == 'little' ) {
	                            var uint64Value = new Uint64LE( buffer, ( bitOffset / 8 ) + ( i * 8 ) );
	                        } else {
	                            var uint64Value = new Uint64BE( buffer, ( bitOffset / 8 ) + ( i * 8 ) );
	                        }
	                        value.push( uint64Value.toString( 10 ) );
	                    }
	                    break;
	
	                case 'int64':
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        if ( this.endian == 'little' ) {
	                            var int64Value = new Int64LE( buffer, ( bitOffset / 8 ) + ( i * 8 ) );
	                        } else {
	                            var int64Value = new Int64BE( buffer, ( bitOffset / 8 ) + ( i * 8 ) );
	                        }
	                        value.push( int64Value.toString( 10 ) );
	                    }
	                    break;
	
	                default:
                        var nextFieldDef = this.getMsgDefByName(fieldDef.real_type);
                        var nextFields = nextFieldDef.fields;
  	                    var value = [];
	                    
	                    for ( var i = 0; i < fieldDef.count; ++i ) {
	                        var nextValue = {};
	                        var elementBitSize = fieldDef.bit_size / fieldDef.count;
	                        var nextBitOffset = bitOffset + ( ( fieldDef.bit_size / fieldDef.count ) * i );
	
	                        for ( var fieldName in nextFields ) {
	                            var nextField = nextFields[ fieldName ];
	                            nextValue[ fieldName ] = this.getFieldValue( buffer, nextField, nextField.bit_offset + nextBitOffset, nextFieldDef );
	                        }
	                        value.push( nextValue );
	                    }

//	                    console.log(value);
//		                if ( typeof nextFieldDef === 'undefined' ) {
//		                    value = this.getFieldValueAsPbType( buffer, fieldDef, bitOffset, rootDef );
//		                } else {
//		                    var nextFields = nextFieldDef.fields;
//		                    var value = [];
//		                    for ( var i = 0; i < fieldDef.count; ++i ) {
//		                        var nextValue = {};
//		                        var elementBitSize = fieldDef.bit_size / fieldDef.count;
//		                        var nextBitOffset = bitOffset + ( ( fieldDef.bit_size / fieldDef.count ) * i );
//		
//		                        for ( var fieldName in nextFields ) {
//		                            var nextField = nextFields[ fieldName ];
//		                            nextValue[ fieldName ] = this.getFieldValue( buffer, nextField, nextField.bit_offset + nextBitOffset, nextFieldDef );
//		                        }
//		                        value.push( nextValue );
//		                    }
//		                }
//                    }
                }
            } else {            	
                if(fieldDef.hasOwnProperty('fields')) {
                    var nextFields = fieldDef.fields;
                    var value = {};
                    for(var i = 0; i < nextFields.length; ++i) {
                    	var nextField = nextFields[i];
                    	var fieldName = nextField.name;
                        value[ fieldName ] = this.getFieldValue( buffer, nextField, nextField.bit_offset + bitOffset, fieldDef );
                    }
                } else {
	            	
	                switch ( fieldDef.real_type ) {        	
		                case 'char':
		                    value = buffer.readUInt8( bitOffset / 8 );
		                    break;
		
		                case 'uint8':
		                    value = buffer.readUInt8( bitOffset / 8 );
		                    break;
		
		                case 'int8':
		                    value = buffer.readInt8( bitOffset / 8 );
		                    break;
		
		                case 'uint16':
		                    if ( this.endian == 'little' ) {
		                        value = buffer.readUInt16LE( bitOffset / 8 );
		                    } else {
		                        value = buffer.readUInt16BE( bitOffset / 8 );
		                    }
		                    break;
		
		                case 'int16':
		                    if ( this.endian == 'little' ) {
		                        value = buffer.readInt16LE( bitOffset / 8 );
		                    } else {
		                        value = buffer.readInt16BE( bitOffset / 8 );
		                    }
		                    break;
		
		                case 'uint32':
		                    if ( this.endian == 'little' ) {
		                        value = buffer.readUInt32LE( bitOffset / 8 );
		                    } else {
		                        value = buffer.readUInt32BE( bitOffset / 8 );
		                    }
		                    break;
		
		                case 'int32':
		                    if ( this.endian == 'little' ) {
		                        value = buffer.readInt32LE( bitOffset / 8 );
		                    } else {
		                        value = buffer.readInt32BE( bitOffset / 8 );
		                    }
		                    break;
		
		                case 'float':
		                    if ( this.endian == 'little' ) {
		                        value = buffer.readFloatLE( bitOffset / 8 );
		                    } else {
		                        value = buffer.readFloatBE( bitOffset / 8 );
		                    }
		                    break;
		
		                case 'double':
		                    if ( this.endian == 'little' ) {
		                        value = buffer.readDoubleLE( bitOffset / 8 );
		                    } else {
		                        value = buffer.readDoubleBE( bitOffset / 8 );
		                    }
		                    break;
		
		                case 'boolean':
		                    value = buffer.readUInt8( bitOffset / 8 );
		                    break;
		
		                case 'uint64':
		                    if ( this.endian == 'little' ) {
		                        var uint64Value = new Uint64LE( buffer, bitOffset / 8 );
		                    } else {
		                        var uint64Value = new Uint64BE( buffer, bitOffset / 8 );
		                    }
		                    value = uint64Value.toString( 10 );
		                    break;
		
		                case 'int64':
		                    if ( this.endian == 'little' ) {
		                        var int64Value = new Int64LE( buffer, bitOffset / 8 );
		                    } else {
		                        var int64Value = new Int64BE( buffer, bitOffset / 8 );
		                    }
		                    value = int64Value.toString( 10 );
		                    break;
		
		                default:
		                    value = this.getFieldValueAsBaseType( buffer, fieldDef, bitOffset, rootDef );
	                }
                }
            }
        } catch ( err ) {
            this.logError('getFieldValue: Unhandled exception. \'' + err + '\'' );
        }
        return value;
    }


    /**
     * Gets telemetry definition by message id
     * @param  {Number} msgID message id
     * @return {Object}       telemetry definition object
     */
    getTlmDefByMsgID( msgID ) {
        for ( var name in this.defs ) {
            var tlm = this.defs[ name ];
            if ( tlm.msgID == msgID ) {
                return tlm;
            }
        }
    }


    /**
     * Convert CFE time to javascript time
     * @param  {Number} seconds    CFE seconds
     * @param  {Number} subseconds CFE subseconds
     * @return {Object}            javascript date object
     */
    cfeTimeToJsTime( seconds, subseconds ) {
        var microseconds;

        /* 0xffffdf00 subseconds = 999999 microseconds, so anything greater
         * than that we set to 999999 microseconds, so it doesn't get to
         * a million microseconds */

        if ( subseconds > 0xffffdf00 ) {
            microseconds = 999999;
        } else {
            /*
             **  Convert a 1/2^32 clock tick count to a microseconds count
             **
             **  Conversion factor is  ( ( 2 ** -32 ) / ( 10 ** -6 ) ).
             **
             **  Logic is as follows:
             **    x * ( ( 2 ** -32 ) / ( 10 ** -6 ) )
             **  = x * ( ( 10 ** 6  ) / (  2 ** 32 ) )
             **  = x * ( ( 5 ** 6 ) ( 2 ** 6 ) / ( 2 ** 26 ) ( 2 ** 6) )
             **  = x * ( ( 5 ** 6 ) / ( 2 ** 26 ) )
             **  = x * ( ( 5 ** 3 ) ( 5 ** 3 ) / ( 2 ** 7 ) ( 2 ** 7 ) (2 ** 12) )
             **
             **  C code equivalent:
             **  = ( ( ( ( ( x >> 7) * 125) >> 7) * 125) >> 12 )
             */

            microseconds = ( ( ( ( ( subseconds >> 7 ) * 125 ) >> 7 ) * 125 ) >> 12 );

            /* if the subseconds % 0x4000000 != 0 then we will need to
             * add 1 to the result. the & is a faster way of doing the % */
            if ( ( subseconds & 0x3ffffff ) != 0 ) {
                microseconds++;
            }

            /* In the Micro2SubSecs conversion, we added an extra anomaly
             * to get the subseconds to bump up against the end point,
             * 0xFFFFF000. This must be accounted for here. Since we bumped
             * at the half way mark, we must "unbump" at the same mark
             */
            if ( microseconds > 500000 ) {
                microseconds--;
            }
        } /* end else */

        /* Get a date with the correct year. */
        var jsDateTime = new Date( "12/1/" + config.get( 'CFE_TIME_EPOCH_YEAR' ) );

        /* Adjust days. */
        jsDateTime.setDate( jsDateTime.getDate() + ( config.get( 'CFE_TIME_EPOCH_DAY' ) - 1 ) );

        /* Adjust hours minutes and seconds. */
        jsDateTime.setTime( jsDateTime.getTime() +
                ( config.get( 'CFE_TIME_EPOCH_HOUR' ) * 3600000 ) +
                ( config.get( 'CFE_TIME_EPOCH_MINUTE' ) * 60000 ) +
                ( config.get( 'CFE_TIME_EPOCH_SECOND' ) * 1000 ) );

        /* Add the CFE seconds. */
        jsDateTime.setTime( jsDateTime.getTime() + ( seconds * 1000 ) );

        /* Finally, add the CFE microseconds. */
        jsDateTime.setMilliseconds( jsDateTime.getMilliseconds() + ( microseconds / 1000 ) );

        return jsDateTime;
    }
    
    
    initConfig(name, configFile) {
    	/* Load environment dependent configuration */
        config.loadFile( configFile );

        /* Perform validation */
        config.validate( {
            allowed: 'strict'
        } );
        
        config.name = name;
    }
    
    
    
    initTelemetry() {
        this.hk = {
            opsPath: '/' + config.name + '/hk',
            content: {
                cmdAcceptCount: 0,
                cmdRejectCount: 0,
                msgRecvCount: 0,
                paramsParsed: 0,
                binaryInputStreamID: config.get( 'binaryInputStreamID' ),
                jsonOutputStreamID: config.get( 'jsonOutputStreamID' ),
                tlmDefReqStreamID: config.get( 'tlmDefReqStreamID' )
            }
        };
        this.addTelemetry(this.hk, 1000);
    }
    
    
    
    initClientInterface() {
        var content = {};
        
        content[config.name] = {
            shortDescription: 'Binary Decoder (' + config.name + ')',
            longDescription: 'Binary Decoder (' + config.name + ')',
            nodes: {
                main: {
                    type: CdrGroundPlugin.ContentType.LAYOUT,
                    shortDescription: 'Main',
                    longDescription: 'Main.',
                    filePath: '/main_layout.lyt',
                    handlebarsContext: {
                        pluginName: config.name
                    }
                },
                hk: {
                    type: CdrGroundPlugin.ContentType.PANEL,
                    shortDescription: 'Housekeeping',
                    longDescription: 'Housekeeping',
                    filePath: '/hk.pug',
                    handlebarsContext: {
                        pluginName: config.name
                    }
                }
            }
        }
        
        this.addContent(content);
    }
    
    
    
    initCommands() {
    	var self = this;
    	
        var cmdReset = {
            opsPath: '/' + config.name + '/reset',
            args: []
        }
        this.addCommand(cmdReset, function(cmd) {
        	self.hk.content.cmdAcceptCount = 0;
        	self.hk.content.cmdRejectCount = 0;
        	self.hk.content.msgRecvCount = 0;
        	self.hk.content.paramsParsed = 0;
        });
    }
}



module.exports = BinaryDecoder;