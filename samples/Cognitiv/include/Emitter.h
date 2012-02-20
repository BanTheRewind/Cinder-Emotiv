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

#pragma once

// Includes
#include "cinder/app/App.h"
#include "cinder/Camera.h"
#include "cinder/Color.h"
#include "cinder/gl/Light.h"
#include "cinder/gl/Material.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Perlin.h"
#include "cinder/Rand.h"
#include "cinder/Rect.h"
#include "Ribbon.h"

// Pointer alias
typedef std::shared_ptr<class Emitter> EmitterRef;

// Particle emitter
class Emitter 
{

public:

	// Creates pointer to emitter instance
	static EmitterRef	create();

	// Destructor
	~Emitter();

	// Methods
	void				addRibbons( int32_t numRibbons, const ci::Vec3f & position, const ci::Vec3f & velocity, 
									float width = 0.7f, const ci::ColorAf & color = ci::ColorAf( 0.0f, 0.0f, 0.0f, 0.0f ) );
	void				draw() const;
	void				update( float counter, const ci::ColorAf & color = ci::ColorAf( 0.0f, 0.0f, 0.0f, 0.0f ) );

	// Setters
	void				setAcceleration( const ci::Vec3f & acceleration ) { mAcceleration = acceleration; }

private:

	// Constructor
	Emitter();

	// Positioning properties
	ci::Vec3f				mAcceleration;
	float					mFriction;
	ci::Vec3f				mPosition;
	ci::Vec3f				mPositionPrev;
	ci::Vec3f				mPositionWorld;
	ci::Vec3f				mVelocity;

	// Camera
	ci::CameraPersp			mCamera;

	// Sphere 
	void					createSphere( float radius = 1.0f, int32_t segments = 24 );
	ci::gl::Material		mMaterial;
	float					mRadius;
	std::vector<uint32_t>	mVboIndices;
	ci::gl::VboMesh::Layout mVboLayout;
	ci::gl::VboMesh			mVboMesh;
	std::vector<ci::Vec3f>	mVboNormals;
	std::vector<ci::Vec3f>	mVboPositions;

	// Light
	ci::gl::Light *			mLight;
		
	// Ribbon color
	ci::ColorAf				mColor;
	double					mColorSetInterval;
	double					mColorSetTime;

	// The ribbons
	std::vector<Ribbon>		mRibbons;

	// Perlin noise to warp path
	float					mMagnitude;
	ci::Perlin				mPerlin;

	// Convert screen space to world space
	ci::Matrix44f			mModelView;
	ci::Matrix44f			mProjection;
	ci::Area				mViewport;
	ci::Rectf				mWindowSize;
	ci::Vec3f				screenToWorld(const ci::Vec3f & point);
	ci::Vec3f				unproject(const ci::Vec3f & point);

};
