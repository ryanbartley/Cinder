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
template<typename VEC_T>
class Cursor;

template<typename VEC_T, typename ROT_T>
class Object;

template<typename VEC_T, typename ROT_T, typename DIM_T>
class Blob;
	
class Blob2D;
class Blob25D;
class Blob3D;
	
class ProfileHandlerBase  {
public:
	virtual ~ProfileHandlerBase() = default;
	virtual void handleMessage( const osc::Message &message ) = 0;
};
	
}

using Cursor2D	= detail::Cursor<ci::vec2>;
using Cursor25D = detail::Cursor<ci::vec3>;
using Cursor3D	= detail::Cursor<ci::vec3>;
	
using Object2D	= detail::Object<ci::vec2, float>;
using Object25D = detail::Object<ci::vec3, float>;
using Object3D	= detail::Object<ci::vec3, ci::vec3>;
	
using Blob2D = detail::Blob2D;
using Blob25D = detail::Blob25D;
using Blob3D = detail::Blob3D;
	

//! Implements a Listener for the TUIO 1.1 protocol, described here: http://www.tuio.org/?specification
class Listener {
public:
	template<typename Profile>
	using ProfileFn = std::function<void(const Profile&)>;
	
	Listener( const app::WindowRef &window,
		    uint16_t localPort = DEFAULT_TUIO_PORT,
		    const asio::ip::udp &protocol = asio::ip::udp::v4(),
		    asio::io_service &io = ci::app::App::get()->io_service() );
	Listener( const osc::ReceiverBase *ptr );
	
	void bind();
	void listen();
	void close();
	
	//! Registers an async callback which fires when a new cursor is added
	template<typename TuioType>
	void	setAdded( ProfileFn<TuioType> callback );
	//! Registers an async callback which fires when a cursor is updated
	template<typename TuioType>
	void	setUpdated( ProfileFn<TuioType> callback );
	//! Registers an async callback which fires when a cursor is removed
	template<typename TuioType>
	void	setRemoved( ProfileFn<TuioType> callback );
	
	//! Removes listeners for TuioType
	template<typename TuioType>
	void	remove();
	
	//! Returns a std::vector of all active touches, derived from \c 2Dcur (Cursor) messages
	template<typename ProfileType>
	std::vector<ProfileType> getActiveProfiles() const;
	
	//! Returns the threshold for a frame ID being old enough to imply a new source
	int32_t	getPastFrameThreshold() const;
	//! Sets the threshold for a frame ID being old enough to imply a new source
	void	setPastFrameThreshold( int32_t pastFrameThreshold );
	
	static const uint16_t DEFAULT_TUIO_PORT = 3333;
	// default threshold for a frame ID being old enough to imply a new source
	static const uint32_t DEFAULT_PAST_FRAME_THRESHOLD = 10;
	
private:
	template<typename T>
	static const char* getOscAddressFromType();
	
	using ProfileHandlers = std::map<std::string, std::unique_ptr<detail::ProfileHandlerBase>>;
	
	std::unique_ptr<osc::ReceiverBase>	mListener;
	ProfileHandlers						mHandlers;
	app::WindowRef						mWindow;
};

} } // namespace tuio // namespace cinder
