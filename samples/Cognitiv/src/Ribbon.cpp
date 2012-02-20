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
#include "Ribbon.h"

// Imports
using namespace ci;
using namespace std;

// Constructor
Ribbon::Ribbon( const Vec3f & position, const Vec3f & velocity, const ColorAf & color, float width )
{
	
	// Set color
	mColor = color;

	// Create point list
	mNumPoints = Rand::randInt( 15, 35 );
	for ( int32_t i = 0; i < mNumPoints; ++i ) {
		mPoints.push_back( position );
	}

	// Set up velocity
	mVelocity = velocity;
	mAcceleration = Vec3f::zero();

	// Set up life span
	mDead = false;
	mAge = 0.0f;
	mLifeSpan = Rand::randFloat( 5.0f, 70.0f );
	if ( Rand::randFloat() < 0.15f ) {
		mLifeSpan += 100.0f;
	}
	mAgeTheta = 1.0f;

	// Set width
	mWidth = width;

}

// Destructor
Ribbon::~Ribbon()
{

	// Clean up
	mPoints.clear();

}

// Draw
void Ribbon::draw() const
{

	// Start quad strip
	glBegin( GL_QUAD_STRIP );
	for ( int32_t i = 0; i < mNumPoints - 2; i++ ) {

		// Calculate dimensions of quad
		float theta = i / (float)( mNumPoints - 1 );
		Vec3f vert0 = Vec3f( mPoints[ i ].x, mPoints[ i ].y, 0.0f ) - Vec3f( mPoints[ i + 1 ].x, mPoints[ i + 1 ].y, 0.0f );
		Vec3f vert1 = vert0.cross( Vec3f::zAxis() );
		Vec3f vert2 = vert0.cross( vert1 );
		vert1 = vert0.cross( vert2 ).normalized();
		Vec3f offset = vert1 * ( 1.0f - theta ) * mWidth;

		// Calculate color
		ColorAf color = mColor;
		color.a = ( 1.0f - theta ) * mAgeTheta;
		gl::color( color );

		// Draw segment
		gl::vertex( Vec3f( mPoints[ i ] - offset ) );
		gl::vertex( Vec3f( mPoints[ i ] + offset ) );

	}

	// Stop drawing
	glEnd();

}

// Runs update logic
void Ribbon::update()
{

	// Apply acceleration
	mVelocity += mAcceleration;

	// Move this point to next point to leave trail
	for ( int32_t i = mNumPoints - 1; i > 0; i-- ) {
		mPoints[ i ] = mPoints[ i - 1 ];
	}

	// Add velocity to leading point
	mPoints[ 0 ] += mVelocity;

	// Apply friction
	mVelocity *= 0.975f;
	mAcceleration.set( 0.0f, 0.0f, 0.0f );
	
	// Age particle
	mAge++;
	if ( mAge > mLifeSpan ) {
		mDead = true;
	} else {
		mAgeTheta = 1.0f - mAge / mLifeSpan;
	}

}
