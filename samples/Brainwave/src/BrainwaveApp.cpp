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
#include "cinder/Path2d.h"
#include "cinder/Utilities.h"
#include "Emotiv.h"

/*
 * Before you run this, make sure you are connected to an active
 * headset. This application does not work with Emotiv control 
 * or panel EmoComposer because they do not emulate brainwaves.
 * 
 * This application demonstrates how to obtain and represent 
 * brainwave channel values as an old school feedback visualizer. 
 * Five Path2D instances represent each brainwave, leaving trails 
 * as you think.
 */
class BrainwaveApp : public ci::app::AppBasic 
{

public:

	// Cinder callbacks
	void draw();
	void prepareSettings( ci::app::AppBasic::Settings * settings );
	void setup();
	void shutdown();
	void update();

	// Emotiv callback
	void onData( EmotivEvent event );

private:

	// Emotiv
	int32_t		mCallbackId;
	EmotivRef	mEmotiv;

	// Brainwaves
	ci::Path2d	mAlpha;
	ci::Path2d	mBeta;
	ci::Path2d	mDelta;
	ci::Path2d	mTheta;

	// Colors
	ci::ColorAf	mColorAlpha;
	ci::Colorf	mColorBackground;
	ci::ColorAf mColorBeta;
	ci::ColorAf mColorDelta;
	ci::ColorAf mColorTheta;

	// Line dimensions
	float		mAmplitude;
	float		mOffset;
	uint32_t	mNumPoints;
	float		mRadius;
	float		mRotation;
	float		mSpeed;
	float		mTrails;
	
	// Writes messages to debug console
	void trace( const std::string &message );

};

// Imports
using namespace ci;
using namespace ci::app;
using namespace std;

// Renders
void BrainwaveApp::draw()
{

	// Set up window
	gl::setViewport( getWindowBounds() );
	gl::setMatricesWindow( getWindowSize() );

	// Draw a box over the entire display instead of clearing
	// the screen. This will leave trails.
	gl::color( ColorAf( mColorBackground, mTrails ) );
	gl::drawSolidRect( Rectf( getWindowBounds() ) );

	// Rotate screen around center
	Vec2f center = getWindowCenter();
	gl::pushMatrices();
	gl::translate( center );
	gl::rotate( mRotation );
	gl::translate( center * -1.0f );
	gl::translate( center );

	// Draw alpha waves
	gl::color( mColorAlpha );
	gl::pushMatrices();
	gl::translate( mOffset, 0.0f );
	gl::draw( mAlpha );
	gl::popMatrices();

	// Beta
	gl::color( mColorBeta );
	gl::pushMatrices();
	gl::rotate( 90.0f );
	gl::translate( mOffset, 0.0f );
	gl::draw( mBeta );
	gl::popMatrices();

	// Delta
	gl::color( mColorDelta );
	gl::pushMatrices();
	gl::rotate( 180.0f );
	gl::translate( mOffset, 0.0f );
	gl::draw( mDelta );
	gl::popMatrices();

	// Theta
	gl::color( mColorTheta );
	gl::pushMatrices();
	gl::rotate( 270.0f );
	gl::translate( mOffset, 0.0f );
	gl::draw( mTheta );
	gl::popMatrices();

	// Stop drawing
	gl::popMatrices();

}

// Handles Emotiv data. The Emotiv block has already analyzed
// the signal into channels. Just get the values from the event.
void BrainwaveApp::onData( EmotivEvent event )
{

	// Shift points over by one
	for ( uint32_t i = mNumPoints - 1; i > 0; i-- ) {
		mAlpha.setPoint( i, Vec2f( mAlpha.getPoint( i ).x, mAlpha.getPoint( i - 1 ).y ) );
		mBeta.setPoint(  i, Vec2f( mBeta.getPoint(  i ).x, mBeta.getPoint(  i - 1 ).y ) );
		mDelta.setPoint( i, Vec2f( mDelta.getPoint( i ).x, mDelta.getPoint( i - 1 ).y ) );
		mTheta.setPoint( i, Vec2f( mTheta.getPoint( i ).x, mTheta.getPoint( i - 1 ).y ) );
	}

	// Set value of brainwave channel in first position of each line
	mAlpha.setPoint(0, Vec2f( mAlpha.getPoint( 0 ).x, -mAmplitude * 0.5f + event.getAlpha() * mAmplitude ) );
	mBeta.setPoint( 0, Vec2f( mBeta.getPoint(  0 ).x, -mAmplitude * 0.5f + event.getBeta() * mAmplitude ) );
	mDelta.setPoint(0, Vec2f( mDelta.getPoint( 0 ).x, -mAmplitude * 0.5f + event.getDelta() * mAmplitude ) );
	mTheta.setPoint(0, Vec2f( mTheta.getPoint( 0 ).x, -mAmplitude * 0.5f + event.getTheta() * mAmplitude ) );
	
}

// Prepare window settings
void BrainwaveApp::prepareSettings( Settings * settings )
{

	// DO IT!
	settings->setWindowSize( 1024, 768 );
	settings->setFrameRate( 60.0f );
	settings->setFullScreen( false );
	settings->setTitle( "Brainwave" );

}

// Setup
void BrainwaveApp::setup()
{

	// Set up OpenGL
	gl::enable( GL_BLEND );
	gl::enable( GL_LINE_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	gl::enableAlphaBlending( true );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	try
	{

		// Start Emotiv
		mEmotiv = Emotiv::create();

		// Connect to the Emotiv engine (requires connected and 
		// active headset).
		if ( mEmotiv->connect( "Emotiv Systems-5" ) ) {
			trace( "Emotiv Engine started" );
		} else {
			trace( "Unable to start Emotiv Engine" );
		}

	} catch ( ... ) {
		trace( "Unable to start Emotiv Engine" );
		quit();
		return;
	}

	// Set line dimensions
	mAmplitude = 0.25f;
	mOffset = 30.0f;
	mNumPoints = 24;
	mRadius = 300.0f;
	mRotation = 0.0f;
	mSpeed = 0.333f;
	mTrails = 0.0333f;

	// Initialize waves
	mAlpha.moveTo( 0.0f, 0.0f );
	mBeta.moveTo( 0.0f, 0.0f );
	mDelta.moveTo( 0.0f, 0.0f );
	mTheta.moveTo( 0.0f, 0.0f );
	for ( uint32_t i = 0; i < mNumPoints; i++ ) {
		float x = ( (float)i / (float)mNumPoints ) * mRadius;
		mAlpha.lineTo( x, 0.0f );
		mBeta.lineTo( x, 0.0f );
		mDelta.lineTo( x, 0.0f );
		mTheta.lineTo( x, 0.0f );
	}
	mNumPoints = mAlpha.getNumPoints();

	// Define colors
	mColorAlpha = ColorAf( 0.207f, 1.0f, 1.0f, 1.0f );
	mColorBackground = Colorf( 0.207f, 0.176f, 0.223f );
	mColorBeta = ColorAf( 1.0f, 0.836f, 0.895f, 1.0f );
	mColorDelta = ColorAf( 0.531f, 0.0f, 0.223f, 1.0f );
	mColorTheta = ColorAf( 0.531f, 0.375f, 0.828f, 1.0f );

	// Add Emotiv callback
	mCallbackId = mEmotiv->addCallback<BrainwaveApp>( &BrainwaveApp::onData, this );

}

// Called on exit
void BrainwaveApp::shutdown()
{
	if ( mEmotiv->connected() ) {
		mEmotiv->disconnect();
	}
}

// Write to console and debug window
void BrainwaveApp::trace( const string &message )
{
#ifdef CINDER_MSW
	OutputDebugStringA( ( message + "\n" ).c_str() );
#else
	console() << message << "\n";
#endif
}

// Runs update logic
void BrainwaveApp::update()
{

	// Update overall rotation
	mRotation += mSpeed;
	if ( mRotation > 360.0f ) {
		mRotation -= 360.0f;
	}

}

// Create the application
CINDER_APP_BASIC( BrainwaveApp, RendererGl )
