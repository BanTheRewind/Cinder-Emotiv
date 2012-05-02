/*
* 
* Copyright (c) 2012, Ban the Rewind
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following 
* conditions are met:
* 
* Redistributions of source code must retain the above copyright 
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright 
* notice, this list of conditions and the following disclaimer in 
* the documentation and/or other materials provided with the 
* distribution.
* 
* Neither the name of the Ban the Rewind nor the names of its 
* contributors may be used to endorse or promote products 
* derived from this software without specific prior written 
* permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

// Include header
#include "Emotiv.h"

// Imports
using namespace ci;
using namespace ci::app;
using namespace std;

// Create pointer to Emotiv instance
EmotivRef Emotiv::create() 
{
	return EmotivRef( new Emotiv() );
}

// Constructor
Emotiv::Emotiv()
{

	// Set up target channel list
	mTargetChannelList[ 0 ] = ED_COUNTER;
	mTargetChannelList[ 1 ] = ED_AF3;
	mTargetChannelList[ 2 ] = ED_F7;
	mTargetChannelList[ 3 ] = ED_F3;
	mTargetChannelList[ 4 ] = ED_FC5;
	mTargetChannelList[ 5 ] = ED_T7;
	mTargetChannelList[ 6 ] = ED_P7;
	mTargetChannelList[ 7 ] = ED_O1;
	mTargetChannelList[ 8 ] = ED_O2;
	mTargetChannelList[ 9 ] = ED_P8;
	mTargetChannelList[ 10 ] = ED_T8;
	mTargetChannelList[ 11 ] = ED_FC6;
	mTargetChannelList[ 12 ] = ED_F4;
	mTargetChannelList[ 13 ] = ED_F8;
	mTargetChannelList[ 14 ] = ED_AF4;
	mTargetChannelList[ 15 ] = ED_GYROX;
	mTargetChannelList[ 16 ] = ED_GYROY;
	mTargetChannelList[ 17 ] = ED_TIMESTAMP;
	mTargetChannelList[ 18 ] = ED_FUNC_ID;
	mTargetChannelList[ 19 ] = ED_FUNC_VALUE;
	mTargetChannelList[ 20 ] = ED_MARKER;
	mTargetChannelList[ 21 ] = ED_SYNC_SIGNAL;

	// Initialize frequency data
	mFreqData = 0;
	mFftEnabled = true;
	mLastSampleTime = 0.0;
	mSampleTime = 1.0;

	// Initialize brainwave frequencies
	mAlpha = 0.0f;
	mBeta = 0.0f;
	mDelta = 0.0f;
	mGamma = 0.0f;
	mTheta = 0.0f;

	// Initialize FFT
	mFft = Kiss::create();

}

// Destructor
Emotiv::~Emotiv()
{

	// Disconnect / clean up
	mCallbacks.clear();
	if ( mConnected ) {
		disconnect();
	}
	if ( mFreqData != 0 ) {
		delete [ ] mFreqData;
	}

}

// Add callback
int32_t Emotiv::addCallback( const boost::function<void ( EmotivEvent event )> &callback )
{

	// Determine return ID
	int32_t mCallbackID = mCallbacks.empty() ? 0 : mCallbacks.rbegin()->first + 1;

	// Create callback and add it to the list
	mCallbacks.insert( std::make_pair( mCallbackID, CallbackRef( new Callback( mSignal.connect( callback ) ) ) ) );

	// Return callback ID
	return mCallbackID;

}

// Connect to Emotiv Engine
bool Emotiv::connect( const string &deviceId, const string &remoteAddress, uint16_t port )
{

	try {

		// Connect to remote engine or composer or the local Emotiv Engine
		if ( remoteAddress.length() > 0 && port > 0 ) {
			mConnected = EE_EngineRemoteConnect( remoteAddress.c_str(), port, deviceId.c_str() ) == EDK_OK;
		} else {
			mConnected = EE_EngineConnect( deviceId.c_str() ) == EDK_OK;
		}

		// Connected
		if ( mConnected ) {

			// Add event listening
			mEvent = EE_EmoEngineEventCreate();
			mState = EE_EmoStateCreate();

			// Set up EEG data sampler
			mData = EE_DataCreate();
			EE_DataSetBufferSizeInSec( (float)mSampleTime );

		}

	} catch ( ... ) {

		// Disconnect if routine fails
		mConnected = false;

	}

	// Start thread
	mRunning = true;
	mThread = std::shared_ptr<boost::thread>( new boost::thread( bind( &Emotiv::update, this ) ) );

	// Return connected flag
	return mConnected;

}

// Disconnect from Emotiv Engine 
bool Emotiv::disconnect()
{

	// Stop thread
	if ( mRunning ) {
		mRunning = false;
		mThread->join();
	}

	// Disconnect and free resources
	try {
		if ( EE_EngineDisconnect() != EDK_OK ) {
			return false;
		}
		EE_EmoStateFree( mState );
		EE_EmoEngineEventFree( mEvent );
		EE_DataFree( mData );
		mConnected = false;
		return true;
	} catch ( ... ) {
		return false;
	}

}

// Get number of connected devices
int32_t Emotiv::getNumUsers()
{

	// Retrieve and return number of connected headsets
	uint32_t users = 0;
	EE_EngineGetNumUser( &users );
	return users;

}

// Load profile onto device
bool Emotiv::loadProfile( const fs::path &profilePath, uint32_t userId )
{

	// Bail if not connected
	if ( !mConnected ) {
		return false;
	}

	// Load profile
	try {
		if ( EE_LoadUserProfile( userId, profilePath.generic_string().c_str() ) != EDK_OK ) {
			return false;
		}
		return true;
	} catch ( ... ) {
		return false;
	}

}

// List profiles
map<fs::path, string> Emotiv::listProfiles( const fs::path &dataPath )
{

	// Create return object
	map<fs::path, string> profiles;
	
	// Iterate through all files in directory, adding all "EMU" files
	fs::path dataDirectory = dataPath.string().length() > 0 ? dataPath : getAppPath() / fs::path( "data" );
	if ( fs::exists( dataDirectory ) ) {
		for ( fs::directory_iterator fileIt( dataDirectory ), mEnd; fileIt != mEnd; ++fileIt ) {
			if ( boost::iequals( fileIt->path().extension().string(), ".emu" ) ) {
				profiles.insert( make_pair( fileIt->path().filename().stem(), fileIt->path().string() ) );
			}
		}
	}

	// Return list
	return profiles;

}

// Removes callback
void Emotiv::removeCallback( int32_t callbackID ) 
{

	// Check if we have callbacks
	if ( mCallbacks.size() > 0 ) {

		// Disconnect the callback from the signal
		mCallbacks.find( callbackID )->second->disconnect();

		// Remove the callback from the list
		mCallbacks.erase( callbackID ); 

	}

}

// Main loop
void Emotiv::update()
{

	// Lock scope
	boost::mutex::scoped_lock lock( mMutex );

	// Check running flag
	while ( mRunning ) {

		// Check connection
		if ( mConnected ) {

			// Get event
			int32_t eventId = EDK_NO_EVENT;
			try {
				eventId = EE_EngineGetNextEvent( mEvent );
			} catch ( ... ) {
			}

			// Verify event
			if ( eventId == EDK_OK ) {

				// Get user ID
				uint32_t userId;
				if ( EE_EmoEngineEventGetUserId( mEvent, &userId ) == EDK_OK ) {

					// Get event type
					EE_Event_t eventType = EE_EmoEngineEventGetType( mEvent );

					// Enable data acquisition for new users
					if ( eventType == EE_UserAdded ) {
						EE_DataAcquisitionEnable( userId, true);
					}

					// Status update
					if ( eventType == EE_EmoStateUpdated ) {

						// Get Emotiv state
						EE_EmoEngineEventGetEmoState( mEvent, mState );
						if ( mState != 0 ) {

							// Create map of Expresiv Suite results
							std::map<EE_ExpressivAlgo_t, float> expressivStates;
							EE_ExpressivAlgo_t upperFaceAction = ES_ExpressivGetUpperFaceAction( mState );
							float upperFacePower = ES_ExpressivGetUpperFaceActionPower( mState );
							EE_ExpressivAlgo_t lowerFaceAction = ES_ExpressivGetLowerFaceAction( mState );
							float lowerFacePower = ES_ExpressivGetLowerFaceActionPower( mState );
							expressivStates[ upperFaceAction ] = upperFacePower;
							expressivStates[ lowerFaceAction ] = lowerFacePower;

							// FFT analysis enabled and data has been sampled
							if ( mFftEnabled && getElapsedSeconds() - mLastSampleTime >= mSampleTime ) {

								// Reset brainwave frequencies
								mAlpha = 0.0f;
								mBeta = 0.0f;
								mDelta = 0.0f;
								mGamma = 0.0f;
								mTheta = 0.0f;

								// Update sample time
								mLastSampleTime = getElapsedSeconds();

								// Get raw EEG data
								EE_DataUpdateHandle( userId, mData );
								uint32_t samplesTaken = 0;
								EE_DataGetNumberOfSample( mData, &samplesTaken );
								if ( samplesTaken != 0 ) {

									// Create buffers for raw and target data
									float * channelData = new float[ samplesTaken ];
									double * data = new double[ samplesTaken ];

									// Iterate through channels
									for ( uint32_t i = 0; i < sizeof( mTargetChannelList ) / sizeof( EE_DataChannel_t ); i++ ) {

										// Get channel data and record as float
										EE_DataGet( mData, mTargetChannelList[ i ], data, samplesTaken );
										channelData[ i ] = static_cast<float>( data[ i ] );
										
									}

									// Clean up data
									delete [ ] data;

									// Get FFT data
									mFft->setDataSize( samplesTaken );
									mFft->setData( channelData );
									mFreqData = mFft->getAmplitude();

									// Clean up
									delete [ ] channelData;

									// Need at least thirty windows
									int32_t dataSize = mFft->getBinSize();
									if ( dataSize > 30 ) {

										// Delta
										for ( uint32_t i = 0; i < 4; i++ ) {
											mDelta += mFreqData[ i ];
										}
										mDelta *= 0.25f;

										// Theta
										for ( uint32_t i = 4; i < 8; i++ ) {
											mTheta += mFreqData[ i ];
										}
										mTheta *= 0.25f;

										// Alpha
										for ( uint32_t i = 8; i <= 13; i++ ) {
											mAlpha += mFreqData[ i ];
										}
										mAlpha *= 0.1666667f;

										// Beta
										for ( uint32_t i = 14; i < 30; i++ ) {
											mBeta += mFreqData[ i ];
										}
										mBeta *= 0.1666667f;

										// Gamma
										for ( uint32_t i = 30; i < samplesTaken; i++ ) {
											mGamma += mFreqData[ i ];
										}
										mGamma /= (float)( samplesTaken - 30 );

									}

								}

							}

							// Dispatch event
							mSignal( EmotivEvent(
								ES_GetTimeFromStart( mState ), 
								userId, 
								ES_GetWirelessSignalStatus( mState ), 
								ES_ExpressivIsBlink( mState ), 
								ES_ExpressivIsLeftWink( mState ), 
								ES_ExpressivIsRightWink( mState ), 
								ES_ExpressivIsLookingLeft( mState ), 
								ES_ExpressivIsLookingRight( mState ), 
								expressivStates[ EXP_EYEBROW ], 
								expressivStates[ EXP_FURROW ], 
								expressivStates[ EXP_SMILE ], 
								expressivStates[ EXP_CLENCH ], 
								expressivStates[ EXP_SMIRK_LEFT ], 
								expressivStates[ EXP_SMIRK_RIGHT ], 
								expressivStates[ EXP_LAUGH ], 
								ES_AffectivGetExcitementShortTermScore( mState ), 
								ES_AffectivGetExcitementLongTermScore( mState ), 
								ES_AffectivGetEngagementBoredomScore( mState ), 
								ES_CognitivGetCurrentAction( mState ), 
								ES_CognitivGetCurrentActionPower( mState ), 
								mAlpha, 
								mBeta, 
								mDelta, 
								mGamma, 
								mTheta
								) );

							// Clean up
							expressivStates.clear();

						}

					}

				}

			}

		}

	}

	// Unlock scope
	mMutex.unlock();

}
