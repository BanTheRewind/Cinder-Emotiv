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

// Includes
#include "cinder/app/AppBasic.h"
#include "cinder/Camera.h"
#include "cinder/Utilities.h"
#include "Emotiv.h"
#include "Emitter.h"

/*
 * Before you run this, make sure the Emotiv control panel 
 * is open and connected to either an Epoc or the EmoComposer.
 * Use the Emotiv control panel to create and save a user named 
 * "default". Check in Emotiv's data directory (ie, 
 * "c:\ProgramData\Emotiv\" on Windows) for a file named "default.emu".
 * 
 * This application demonstrates how to read data from the 
 * Emotiv Cognitiv suite to move a cursor around the screen 
 * with your mind! This also shows you how to load a profile that 
 * you've trained using teh control panel. To keep things interesting, 
 * moving the cursor fires off some pretty ribbons. The code in the main 
 * application focuses just on reading the Epoc's data. To play with 
 * the visuals, dig into the Emitter and Ribbon classes.
 * 
 * Try playing with other parameters from the EmotivEvent in
 * the "onData" method. Move the cursor around with your mouth or
 * eyes. Control the color of ribbons with your emotions or 
 * their size with your brainwaves.
 * 
 */
class CognitivApp : public ci::app::AppBasic 
{

public:

	// Cinder callbacks
	void draw();
	void prepareSettings( ci::app::AppBasic::Settings * settings );
	void setup();
	void update();

	// Emotiv callback
	void onData( EmotivEvent event );

private:

	// Emotiv
	bool		loadProfile( const std::string & profileName );
	int32_t		mCallbackId;
	EmotivRef	mEmotiv;
	
	// Emitter (all the visual code is in here)
	EmitterRef	mEmitter;

	// Writes messages to debug console
	void trace( const std::string & message );

};

// Imports
using namespace ci;
using namespace ci::app;
using namespace std;

// Renders
void CognitivApp::draw()
{

	// To keep the focus on using the Emotiv block, all 
	// the rendering code is inside the Emitter class
	mEmitter->draw();

}

// Load a profile by name
bool CognitivApp::loadProfile( const string & profileName )
{

	// Load profile from default location
	try {
		map<string, string> profiles = Emotiv::listProfiles( "c:\\ProgramData\\Emotiv" );
		for ( map<string, string>::iterator profileIt = profiles.begin(); profileIt != profiles.end(); ++profileIt ) {
			if ( boost::to_lower_copy( profileIt->first ) == boost::to_lower_copy( profileName ) && mEmotiv->loadProfile( profileIt->second ) ) {
				profiles.clear();
				return true;
			}
		}
		profiles.clear();
	} catch ( ... ) {
	}

	// Nothing loaded
	return false;

}

// Handles Emotiv data. Check out the getters available
// in the Emotiv event for access to all the data the Epoc
// has to offer.
void CognitivApp::onData( EmotivEvent event )
{

	// Initialize acceleration
	Vec3f acceleration = Vec3f::zero();

	// Get power of Cognitiv command
	float power = event.getCognitivPower();

	// Key on Cognitiv action to get direction
	switch ( event.getCognitivAction() ) {
	case EmotivEvent::COG_DROP:
		acceleration.y += power;
		break;
	case EmotivEvent::COG_LEFT:
		acceleration.x -= power;	
		break;
	case EmotivEvent::COG_LIFT:
		acceleration.y -= power;
		break;
	case EmotivEvent::COG_RIGHT:
		acceleration.x += power;
		break;
	}

	// Update emitter's acceleration
	mEmitter->setAcceleration( acceleration );

}

// Prepare app settings
void CognitivApp::prepareSettings( Settings * settings )
{

	// Do settings
	settings->setWindowSize( 1024, 768 );
	settings->setFrameRate( 60.0f );
	settings->setFullScreen( false );
	settings->setTitle( "Cognitiv" );

}

// Setup
void CognitivApp::setup()
{

	// Set up OpenGL
	gl::enable( GL_BLEND );
	gl::enable( GL_DEPTH_TEST );
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	glShadeModel( GL_SMOOTH );
	gl::enable( GL_NORMALIZE );
	gl::enableAlphaBlending( true );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	try {

		// Start Emotiv
		mEmotiv = Emotiv::create();

		// Connect to the Emotiv control panel. Remove the last two arguments
		// to connect directly to the Emotiv engine (requires connected and 
		// active headset).
		if ( mEmotiv->connect( "Emotiv Systems-5", "127.0.0.1", Emotiv::REMOTE_PORT ) ) {
			trace( "Emotiv Engine started" );
		} else {
			trace( "Unable to start Emotiv Engine" );
		}

	} catch ( ... ) {
		trace( "Unable to start Emotiv Engine" );
		quit();
		return;
	}

	// Load profile
	if ( loadProfile( "default" ) ) {
		trace( "Profile loaded" );
	} else {
		trace( "Unable to load profile" );
	}

	// Add Emotiv callback
	mCallbackId = mEmotiv->addCallback<CognitivApp>( & CognitivApp::onData, this );

	// Create emitter
	mEmitter = Emitter::create();

}

// Write to console and debug window
void CognitivApp::trace( const string & message )
{

	// DO IT!
#ifdef CINDER_MSW
	OutputDebugStringA( ( message + "\n" ).c_str() );
#else
	console() << message << "\n";
#endif

}

// Runs update logic
void CognitivApp::update()
{

	// Update emitter, ribbons...
	// You can pass a color as a second argument to control the
	// color of new ribbons.
	mEmitter->update( (float)getElapsedFrames() );

}

// Create the application
CINDER_APP_BASIC( CognitivApp, RendererGl )
