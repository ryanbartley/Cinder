/*
 Copyright (c) 2015, The Cinder Project: http://libcinder.org
 All rights reserved.
 
 Portions Copyright (c) 2010, Hector Sanchez-Pajares
 Aer Studio http://www.aerstudio.com
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "cinder/Cinder.h"
#include "cinder/app/App.h"
#include "Osc.h"
#include "cinder/Log.h"

namespace cinder { namespace tuio {

namespace detail {
template<typename VEC_T> class Cursor;
template<typename VEC_T, typename ROT_T> class Object;
template<typename VEC_T, typename ROT_T, typename DIM_T> class Blob;
	
class Blob2D;
class Blob25D;
class Blob3D;
	
class TypeHandlerBase  {
public:
	virtual ~TypeHandlerBase() = default;
	virtual void handleMessage( const osc::Message &message ) = 0;
};
}

using Cursor2D = detail::Cursor<ci::vec2>;
using Cursor25D = detail::Cursor<ci::vec3>;
using Cursor3D	= detail::Cursor<ci::vec3>;
	
using Object2D	= detail::Object<ci::vec2, float>;
using Object25D = detail::Object<ci::vec3, float>;
using Object3D	= detail::Object<ci::vec3, ci::vec3>;
	
using Blob2D = detail::Blob2D;
using Blob25D = detail::Blob25D;
using Blob3D = detail::Blob3D;
	
//! Implements a Receiver for the TUIO 1.1 protocol, described here: http://www.tuio.org/?specification
class Receiver {
public:
	template<typename Type>
	using TypeFn = std::function<void(const Type&)>;
	using TypeHandlers = std::map<std::string, std::unique_ptr<detail::TypeHandlerBase>>;
	
	Receiver( uint16_t localPort = DEFAULT_TUIO_PORT,
		      const asio::ip::udp &protocol = asio::ip::udp::v4(),
		      asio::io_service &io = ci::app::App::get()->io_service() );
	Receiver( const app::WindowRef &window,
			 uint16_t localPort = DEFAULT_TUIO_PORT,
			 const asio::ip::udp &protocol = asio::ip::udp::v4() );
	Receiver( osc::ReceiverBase *ptr );
	Receiver( const app::WindowRef &window, osc::ReceiverBase *ptr );
	
	//! Registers an async callback which fires when a new cursor is added
	template<typename TuioType>
	void	setAddedFn( TypeFn<TuioType> callback );
	//! Registers an async callback which fires when a cursor is updated
	template<typename TuioType>
	void	setUpdatedFn( TypeFn<TuioType> callback );
	//! Registers an async callback which fires when a cursor is removed
	template<typename TuioType>
	void	setRemovedFn( TypeFn<TuioType> callback );
	
	//! Removes Receivers for TuioType
	template<typename TuioType>
	void	clear();
	
	//! Returns a std::vector of all active touches, derived from \c 2Dcur (Cursor) messages
	template<typename TuioType>
	std::vector<TuioType> getActive( const std::string &source = "" ) const;
	
	//! Returns the threshold for a frame ID being old enough to imply a new source
	int32_t	getPastFrameThreshold() const;
	//! Sets the threshold for a frame ID being old enough to imply a new source
	void	setPastFrameThreshold( int32_t pastFrameThreshold );
	
	osc::ReceiverBase* getOscReceiver() const { return mReceiver.get(); }
	
	static const uint16_t DEFAULT_TUIO_PORT = 3333;
	// default threshold for a frame ID being old enough to imply a new source
	static const uint32_t DEFAULT_PAST_FRAME_THRESHOLD = 10;
	
private:
	template<typename T>
	static const char*	getOscAddressFromType();
	void				setupWindowReceiver( ci::app::WindowRef window );
	
	std::unique_ptr<osc::ReceiverBase>	mReceiver;
	TypeHandlers						mHandlers;
};
	
namespace detail {
	
//! Represents the base info of a tuio type.
class Type {
public:
	//! Returns the sessionId for this type.
	int32_t getSessionId() const { return mSessionId; }
	//! Returns the source of this type.
	const std::string& getSource() const { return mSource; }
	//! Sets the source of this type.
	void setSource( const std::string &source ) { mSource = source; }
	//!
	bool operator<( const Type &other );
protected:
	Type( const osc::Message &msg );
	Type( Type &&other ) NOEXCEPT;
	Type& operator=( Type &&other ) NOEXCEPT;
	Type( const Type &other ) = default;
	Type& operator=( const Type &other ) = default;
	~Type() = default;
	
	int32_t		mSessionId;
	std::string mSource;
};

template<typename VEC_T>
class Cursor : public Type {
public:
	Cursor( const osc::Message &msg );
	Cursor( Cursor &&other ) NOEXCEPT;
	Cursor& operator=( Cursor &&other ) NOEXCEPT;
	Cursor( const Cursor &other ) = default;
	Cursor& operator=( const Cursor &other ) = default;
	~Cursor() = default;
	
	const VEC_T&	getPosition() const { return mPosition; }
	const VEC_T&	getVelocity() const { return mVelocity; }
	float			getAcceleration() const { return mAcceleration; }
	
	app::TouchEvent::Touch	convertToTouch( const ci::app::WindowRef &window ) const;
	
protected:
	VEC_T		mPosition, mVelocity;
	float		mAcceleration;
};

template<typename VEC_T, typename ROT_T>
class Object : public Type {
public:
	Object( const osc::Message &msg );
	Object( Object &&other ) NOEXCEPT;
	Object& operator=( Object &&other ) NOEXCEPT;
	~Object() = default;
	
	int32_t			getClassId() const { return mClassId; }
	const VEC_T&	getPosition() const { return mPosition; }
	const VEC_T&	getVelocity() const { return mVelocity; }
	const ROT_T&	getAngle() const { return mAngle; }
	const ROT_T&	getRotationVelocity() const { return mRotationVelocity; }
	float			getAcceleration() const { return mAcceleration; }
	float			getRotationAcceleration() const { return mRotateAccel; }
protected:
	int32_t		mClassId;
	VEC_T		mPosition, mVelocity;
	ROT_T		mAngle, mRotationVelocity;
	float		mAcceleration, mRotateAccel;
};

template<typename VEC_T, typename ROT_T, typename DIM_T>
class Blob : public Type {
public:
	Blob( const osc::Message &msg );
	Blob( Blob &&other ) NOEXCEPT;
	Blob& operator=( Blob &&other ) NOEXCEPT;
	~Blob() = default;
	
	const VEC_T&	getPosition() const { return mPosition; }
	const VEC_T&	getVelocity() const { return mVelocity; }
	const ROT_T&	getAngle() const { return mAngle; }
	const ROT_T&	getRotationVelocity() const { return mRotationVelocity; }
	float			getAcceleration() const { return mAcceleration; }
	float			getRotationAcceleration() const { return mRotateAccel; }
	const DIM_T&	getDimension() { return mDimensions; }
	
protected:
	VEC_T		mPosition, mVelocity;
	ROT_T		mAngle, mRotationVelocity;
	DIM_T		mDimensions;
	float		mAcceleration, mRotateAccel;
	float		mGeometry;
};
	
class Blob2D : public detail::Blob<ci::vec2, float, ci::vec2> {
public:
	Blob2D( const osc::Message &msg );
	float getArea() const { return mGeometry; }
};
class Blob25D : public detail::Blob<ci::vec3, float, ci::vec2> {
public:
	Blob25D( const osc::Message &msg );
	float getArea() const { return mGeometry; }
};
class Blob3D : public detail::Blob<ci::vec3, ci::vec3, ci::vec3> {
public:
	Blob3D( const osc::Message &msg );
	float getVolume() const { return mGeometry; }
};
	
} // namespace detail

} } // namespace tuio // namespace cinder
