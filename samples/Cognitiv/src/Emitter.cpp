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
#include "Emitter.h"

// Imports
using namespace ci;
using namespace ci::app;
using namespace std;

// Creates pointer to emitter instance
EmitterRef Emitter::create()
{
	return EmitterRef( new Emitter() );
}

// Constructor
Emitter::Emitter()
{

	// Define properties
	mAcceleration = Vec3f::zero();
	mColor = ColorAf::white(); 
	mColorSetInterval = 0.5;
	mColorSetTime = getElapsedSeconds();
	mFriction = 0.965f;
	mMagnitude = 0.3f; 
	mPosition = Vec3f( app::getWindowCenter().x, app::getWindowCenter().y, 0.0f );
	mPositionPrev = mPosition;
	mPositionWorld = Vec3f::zero();
	mRadius = 1.8f; 
	mVelocity = Vec3f::zero(); 

	// Set up lighting
	mLight = new gl::Light( gl::Light::DIRECTIONAL, 0 );
	mLight->setDirection( Vec3f( 0.0f, 0.1f, 0.3f ).normalized() );
	mLight->setAmbient( ColorAf( 0.2f, 0.2f, 0.2f, 1.0f ) );
	mLight->setDiffuse( ColorAf(1.0f, 1.0f, 1.0f, 1.0f ) );
	mLight->enable();

	// Set up material
	mMaterial.setAmbient( ColorAf( 0.7f, 0.65f, 0.60f, 1.0f ) );
	mMaterial.setDiffuse( ColorAf( 0.0f, 0.0f, 0.0f, 1.0f ) );
	mMaterial.setShininess( 50.0f );
	mMaterial.setSpecular( ColorAf( 1.0f, 1.0f, 1.0f, 1.0f ) );

	// Set up camera
	mCamera.lookAt( Vec3f( 0.0f, 0.0f, 20.0f ), Vec3f::zero() );
	mCamera.setPerspective( 45.0f, getWindowAspectRatio(), 1.0f, 1000.0f );

	// Create sphere
	createSphere( 1.0f, 64 );

}

// Destructor
Emitter::~Emitter()
{

	// Delete light
	if ( mLight != 0 ) {
		delete mLight;
	}

	// Erase ribbons
	mRibbons.clear();

}

// Adds ribbons
void Emitter::addRibbons( int32_t numRibbons, const Vec3f & position, const Vec3f & velocity, float width, const ColorAf & color )
{

	// Set color
	if ( color.a > 0.0f ) {
		mColor = color;
	} else {
		double elapsedSeconds = getElapsedSeconds();
		if ( elapsedSeconds - mColorSetTime > mColorSetInterval ) {
			mColor = ColorAf(
				Rand::randFloat(), 
				Rand::randFloat(), 
				Rand::randFloat(), 
				1.0f );
			mColorSetTime = elapsedSeconds;
		}
	}

	// Iterate through count
	for ( int32_t i = 0; i < numRibbons; i++ ) {
		
		// Add ribbon at random offset from point
		Vec3f offset = Rand::randVec3f() + Vec3f( 0.0f, 0.0f, 0.5f );
		mRibbons.push_back( Ribbon( position + offset, ( -velocity + offset * Rand::randFloat( 6.0f, 10.5f ) * 0.75f + 
			Rand::randVec3f() * Rand::randFloat( 1.0f, 2.0f ) ) * 0.065f, mColor, width ) );

	}

}

// Create sphere VBO
void Emitter::createSphere( float radius, int32_t segments )
{

	// Initialize VBO layout
	mVboLayout.setStaticIndices();
	mVboLayout.setStaticNormals();
	mVboLayout.setStaticPositions();

	// Define steps
	int32_t layers = segments / 2;
	float step = (float)M_PI / (float)layers;
	float delta = ( (float)M_PI * 2.0f ) / (float)segments;

	// Phi
	int32_t p = 0;
	for ( float phi = 0.0f; p <= layers; p++, phi += step ) {

		// Theta
		int32_t t = 0;
		for ( float theta = delta; t < segments; t++, theta += delta ) {

			// Set vertex
			Vec3f position(
				radius * math<float>::sin( phi ) * math<float>::cos( theta ),
				radius * math<float>::sin( phi ) * math<float>::sin( theta ),
				-radius * math<float>::cos( phi ) );
			mVboPositions.push_back( position );

			// Set normal
			Vec3f normal = position.normalized();
			mVboNormals.push_back( normal );

			// Add indices
			int32_t n = t + 1 >= segments ? 0 : t + 1;
			mVboIndices.push_back( p * segments + t );
			mVboIndices.push_back( ( p + 1 ) * segments + t );
			mVboIndices.push_back( p * segments + n );
			mVboIndices.push_back( p * segments + n );
			mVboIndices.push_back( ( p + 1 ) * segments + t );
			mVboIndices.push_back( ( p + 1 ) * segments + n );

		}

	}

	// Set VBO data
	mVboMesh = gl::VboMesh( mVboPositions.size(), mVboIndices.size(), mVboLayout, GL_TRIANGLES );
	mVboMesh.bufferIndices( mVboIndices );
	mVboMesh.bufferNormals( mVboNormals );
	mVboMesh.bufferPositions( mVboPositions );

	// Clear vectors since we don't need them anymore
	mVboIndices.clear();
	mVboNormals.clear();
	mVboPositions.clear();

}

// Draw ribbons
void Emitter::draw() const
{

	// Clear screen
	gl::clear( ColorAf( 0.7f, 0.65f, 0.6f, 1.0f ) );
	gl::setMatrices( mCamera );

	// Draw ribbons
	for ( vector<Ribbon>::const_iterator ribbonIt = mRibbons.cbegin(); ribbonIt != mRibbons.cend(); ++ribbonIt ) {
		ribbonIt->draw();
	}
	
	// Draw sphere
	gl::enable( GL_LIGHTING );
	gl::enable( GL_COLOR_MATERIAL );
	mMaterial.apply();
	gl::color( ColorAf::white() );
	gl::pushMatrices();
	gl::translate( mPositionWorld );
	gl::draw( mVboMesh );
	gl::popMatrices();
	gl::disable( GL_COLOR_MATERIAL );
	gl::disable( GL_LIGHTING );

}

// Convert screen position to world coordinates
Vec3f Emitter::screenToWorld( const Vec3f & point )
{

	// Find near and far plane intersections
	Vec3f point3f = Vec3f( point.x, mWindowSize.getHeight() * 0.5f - point.y, 0.0f );
	Vec3f nearPlane = unproject( point3f );
	Vec3f farPlane = unproject( Vec3f( point3f.x, point3f.y, 1.0f ) );

	// Calculate X, Y and return point
	float theta = ( 0.0f - nearPlane.z ) / ( farPlane.z - nearPlane.z );
	return Vec3f(
		nearPlane.x + theta * ( farPlane.x - nearPlane.x ), 
		nearPlane.y + theta * ( farPlane.y - nearPlane.y ), 
		0.0f
		);

}

// Unproject a coordinate back to to camera
Vec3f Emitter::unproject( const Vec3f & point )
{

	// Find the inverse Modelview-Projection-Matrix
	Matrix44f mInvMVP = mProjection * mModelView;
	mInvMVP.invert();

	// Ttransform to normalized coordinates in the range [-1, 1]
	Vec4f pointNormal;
	pointNormal.x = ( point.x - mViewport.getX1() ) / mViewport.getWidth() * 2.0f - 1.0f;
	pointNormal.y = ( point.y - mViewport.getY1() ) / mViewport.getHeight() * 2.0f;
	pointNormal.z = 2.0f * point.z - 1.0f;
	pointNormal.w = 1.0f;

	// Find the object's coordinates
	Vec4f pointCoord = mInvMVP * pointNormal;
	if ( pointCoord.w != 0.0f ) {
		pointCoord.w = 1.0f / pointCoord.w;
	}

	// Return coordinate
	return Vec3f(
		pointCoord.x * pointCoord.w, 
		pointCoord.y * pointCoord.w, 
		pointCoord.z * pointCoord.w
		);

}

// Runs update logic
void Emitter::update( float counter, const ColorAf & color )
{

	// Update view dimensions
	gl::setMatrices( mCamera );
	mModelView = gl::getModelView();
	mProjection = gl::getProjection();
	mViewport = gl::getViewport();
	mWindowSize = Rectf( getWindowBounds() );

	// Make emitter "float"
	float elapsedSeconds = (float)getElapsedSeconds();
	mPosition += Vec3f( math<float>::cos( elapsedSeconds ), math<float>::sin( elapsedSeconds ), 0.0f ) * 0.1f;

	// Add acceleration to velocity
	mVelocity += mAcceleration;
	mAcceleration = Vec3f::zero();

	// Clamp cursor position
	mPosition.x = math<float>::clamp( (float)mPosition.x, 0.0f, mWindowSize.getWidth() );
	mPosition.y = math<float>::clamp( (float)mPosition.y, 0.0f, mWindowSize.getHeight() );
	if ( mPosition.x == 0.0f || mPosition.x == mWindowSize.getWidth() ) {
		mVelocity.x = -mVelocity.x;
	}
	if ( mPosition.y == 0.0f || mPosition.y == mWindowSize.getHeight() ) {
		mVelocity.y = -mVelocity.y;
	}

	// Update position, velocity
	mVelocity *= mFriction;
	mPosition += mVelocity;

	// Set position in world coordinates
	mPositionWorld = screenToWorld( mPosition );

	// Cursor has moved
	if ( mPosition.distance(mPositionPrev) > 3.0f ) {

		// Add ribbons 
		int32_t count = 1;
		if ( Rand::randFloat() < 0.02f ) {
			count *= 5;
		}
		addRibbons( count, mPositionWorld, mVelocity.normalized() * 0.001f, Rand::randFloat( 0.0667f, 0.333f ), color );

		// Update last cursor position
		mPositionPrev = mPosition;

	} else {

		// Add small ribbon randomly when not moving
		if ( Rand::randFloat() < 0.15f ) {
			addRibbons( 1, mPositionWorld, screenToWorld( mVelocity ) * 0.0000001f, 0.025f, ColorAf::white() );
		}

	}

	// Iterate through ribbons
	for ( vector<Ribbon>::iterator ribbonIt = mRibbons.begin(); ribbonIt != mRibbons.end(); )
	{

		// Apply noise to each ribbon's acceleration
		Vec3f acceleration = mPerlin.dfBm( ribbonIt->getPoints()[ 0 ] * 0.01f + 
			Vec3f( 0.0f, 0.0f, counter / 100.0f ) ).normalized() * mMagnitude;
		acceleration.z = -math<float>::abs( acceleration.z );
		ribbonIt->setAcceleration( acceleration );

		// Update ribbon
		ribbonIt->update();

		// Advance to next ribbon or remove this one if dead
		if ( ribbonIt->isDead() ) {
			ribbonIt = mRibbons.erase( ribbonIt );
		} else {
			++ribbonIt;
		}

	}
	
}
