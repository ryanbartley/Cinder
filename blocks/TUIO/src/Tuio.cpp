//
//  Tuio.cpp
//  TUIOListener
//
//  Created by ryan bartley on 11/5/15.
//
//

#include "Tuio.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace asio;
using namespace asio::ip;
using Connection = boost::signals2::connection;

namespace cinder { namespace  tuio {
	
namespace detail {
	
Profile::Profile( const osc::Message &msg )
: mSessionId( msg[1].int32() )
{
}

template<>
Cursor<ci::vec2>::Cursor( const osc::Message &msg )
: Profile( msg ), mPosition( msg[2].flt(), msg[3].flt() ),
	mVelocity( msg[4].flt(), msg[5].flt() ), mAcceleration( msg[6].flt() )
{
}
	
template<>
Cursor<ci::vec3>::Cursor( const osc::Message &msg )
: Profile( msg ), mPosition( msg[2].flt(), msg[3].flt(), msg[4].flt() ),
mVelocity( msg[5].flt(), msg[6].flt(), msg[7].flt() ), mAcceleration( msg[8].flt() )
{
}
	
template<typename T>
app::TouchEvent::Touch Cursor<T>::convertToTouch() const
{
	app::TouchEvent::Touch ret( getPosition(),
								getPosition() - getVelocity(),
								getSessionId(),
								app::getElapsedSeconds(),
								nullptr );
	return ret;
}
	
template<>
Object<ci::vec2, float>::Object( const osc::Message &msg )
: Profile( msg ), mClassId( msg[2].int32() ), mPosition( msg[3].flt(), msg[4].flt() ),
	mAngle( msg[5].flt() ), mVelocity( msg[6].flt(), msg[7].flt() ),
	mRotationVelocity( msg[8].flt() ), mAcceleration( msg[9].flt() ),
	mRotateAccel( msg[10].flt() )
{
}
	
template<>
Object<ci::vec3, float>::Object( const osc::Message &msg )
: Profile( msg ), mClassId( msg[2].int32() ),
	mPosition( msg[3].flt(), msg[4].flt(), msg[5].flt() ),
	mAngle( msg[6].flt() ), mVelocity( msg[7].flt(), msg[8].flt(), msg[9].flt() ),
	mRotationVelocity( msg[10].flt() ), mAcceleration( msg[11].flt() ),
	mRotateAccel( msg[12].flt() )
{
}
	
template<>
Object<ci::vec3, ci::vec3>::Object( const osc::Message &msg )
: Profile( msg ), mClassId( msg[2].int32() ),
	mPosition(	msg[3].flt(), msg[4].flt(), msg[5].flt() ),
	mAngle(		msg[6].flt(), msg[7].flt(), msg[8].flt() ),
	mVelocity(	msg[9].flt(), msg[10].flt(), msg[11].flt() ),
	mRotationVelocity( msg[12].flt(), msg[13].flt(), msg[14].flt() ),
	mAcceleration( msg[15].flt() ), mRotateAccel( msg[16].flt() )
{
}
	
template<>
Blob<ci::vec2, float, ci::vec2>::Blob( const osc::Message &msg )
: Profile( msg ), mPosition( msg[2].flt(), msg[3].flt() ), mAngle( msg[4].flt() ),
	mDimensions( msg[5].flt(), msg[6].flt() ), mGeometry( msg[7].flt() ),
	mVelocity( msg[8].flt(), msg[9].flt() ), mRotationVelocity( msg[10].flt() ),
	mAcceleration( msg[11].flt() ), mRotateAccel( msg[12].flt() )
{
}
	
template<>
Blob<ci::vec3, float, ci::vec2>::Blob( const osc::Message &msg )
: Profile( msg ), mPosition( msg[2].flt(), msg[3].flt(), msg[4].flt() ),
	mAngle( msg[5].flt() ), mDimensions( msg[6].flt(), msg[7].flt() ),
	mGeometry( msg[8].flt() ), mVelocity( msg[9].flt(), msg[10].flt(), msg[11].flt() ),
	mRotationVelocity( msg[12].flt() ), mAcceleration( msg[13].flt() ),
	mRotateAccel( msg[14].flt() )
{
}
	
template<>
Blob<ci::vec3, ci::vec3, ci::vec3>::Blob( const osc::Message &msg )
: Profile( msg ), mPosition( msg[2].flt(), msg[3].flt(), msg[4].flt() ),
	mAngle( msg[5].flt(), msg[6].flt(), msg[7].flt() ),
	mDimensions( msg[8].flt(), msg[9].flt(), msg[10].flt() ),
	mGeometry( msg[11].flt() ), mVelocity( msg[12].flt(), msg[13].flt(), msg[14].flt() ),
	mRotationVelocity( msg[15].flt(), msg[16].flt(), msg[17].flt() ),
	mAcceleration( msg[18].flt() ), mRotateAccel( msg[19].flt() )
{
}
	
} // namespace detail
	
Blob2D::Blob2D( const osc::Message &msg )
: Blob( msg )
{
}

Blob25D::Blob25D( const osc::Message &msg )
: Blob( msg )
{
}

Blob3D::Blob3D( const osc::Message &msg )
: Blob( msg )
{
}
	
	
//app::TouchEvent::Touch ProfileBase::getTouch( double time, const vec2 &posScale ) const
//{
//	return app::TouchEvent::Touch( getPos() * posScale,
//								   getPrevPos() * posScale,
//								   (uint32_t)getSessionId(),
//								   time, NULL );
//}
	
template<typename T>
const char* Client::getOscAddressFromType()
{
	if( std::is_same<T, Cursor2D>::value ) return "/tuio/2Dcur";
	else if( std::is_same<T, Cursor25D>::value ) return "/tuio/25Dcur";
	else if( std::is_same<T, Cursor3D>::value ) return "/tuio/3Dcur";
	else if( std::is_same<T, Object2D>::value ) return "/tuio/2Dobj";
	else if( std::is_same<T, Object25D>::value ) return "/tuio/25Dobj";
	else if( std::is_same<T, Object3D>::value ) return "/tuio/3Dobj";
	else if( std::is_same<T, Blob2D>::value ) return "/tuio/2Dblb";
	else if( std::is_same<T, Blob25D>::value ) return "/tuio/25Dblb";
	else if( std::is_same<T, Blob3D>::value ) return "/tuio/3Dblb";
	// TODO: decide if this is good. Probably not but lets see
	else if( std::is_same<T, TouchEvent>::value ) return "/tuio/2Dcur";
	else return "Unknown Target";
}

template<typename T>
void Client::setProfileAddedCallback( ProfileFn<T> callback )
{
	auto address = getOscAddressFromType<T>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto profile = dynamic_cast<ProfileHandler<T>*>(found->second.get());
		profile->setAddHandler( callback );
	}
	else {
		// TODO: Add listener to osc here
		auto profile = std::unique_ptr<ProfileHandler<T>>( new ProfileHandler<Cursor2D>() );
		profile->setAddHandler( callback );
		mHandlers.insert( { address, std::move( profile ) } );
	}
}
	
template<typename T>
void Client::setProfileUpdatedCallback( ProfileFn<T> callback )
{
	auto address = getOscAddressFromType<T>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto profile = dynamic_cast<ProfileHandler<T>*>(found->second.get());
		profile->setUpdateHandler( callback );
	}
	else {
		// TODO: Add listener to osc here
		auto profile = std::unique_ptr<ProfileHandler<T>>( new ProfileHandler<Cursor2D>() );
		profile->setUpdateHandler( callback );
		mHandlers.insert( { address, std::move( profile ) } );
	}
}
	
template<typename T>
void Client::setProfileRemovedCallback( ProfileFn<T> callback )
{
	auto address = getOscAddressFromType<T>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto profile = dynamic_cast<ProfileHandler<T>*>(found->second.get());
		profile->setRemoveHandler( callback );
	}
	else {
		// TODO: Add listener to osc here
		auto profile = std::unique_ptr<ProfileHandler<T>>( new ProfileHandler<Cursor2D>() );
		profile->setRemoveHandler( callback );
		mHandlers.insert( { address, std::move( profile ) } );
	}
}
	
template<typename T>
void Client::ProfileHandler<T>::setAddHandler( ProfileFn<T> callback )
{
	std::lock_guard<std::mutex> lock( mAddMutex );
	mAddCallback = callback;
}
	
template<typename T>
void Client::ProfileHandler<T>::setUpdateHandler( ProfileFn<T> callback )
{
	std::lock_guard<std::mutex> lock( mUpdateMutex );
	mUpdateCallback = callback;
}
	
template<typename T>
void Client::ProfileHandler<T>::setRemoveHandler( ProfileFn<T> callback )
{
	std::lock_guard<std::mutex> lock( mRemoveMutex );
	mRemoveCallback = callback;
}
	
template<typename T>
void Client::ProfileHandler<T>::handleMessage( const osc::Message &message )
{
	auto messageType = message[0].string();
	
	if( messageType == "source" ) {
		mCurrentSource = message[1].string();
	}
	else if( messageType == "set" ) {
		auto inserted = mNextFrameTouches.emplace( message );
		if( inserted.second ) {
			
		}
		else {
			inserted.first = std::move( message );
		}
	}
	else if( messageType == "alive" ) {
		for( int i = 1; i < message.getNumArgs(); i++ ) {
			mCurrentAliveIds.insert( message[i].int32() );
		}
	}
	else if( messageType == "fseq" ) {
		auto frame = message[1].int32();
		int32_t prev_frame = mSourceFrameNums[mCurrentSource];
		int32_t delta_frame = frame - prev_frame;
		// TODO: figure out about past frame threshold updating, this was also in the if condition ( dframe < -mPastFrameThreshold )
		if( frame == -1 || delta_frame > 0 ) {
			std::vector<int32_t> deleteIds;
			handleAdds( deleteIds );
			handleUpdates();
			handleRemoves();
		}
	}
}
	
template<typename T>
void Client::ProfileHandler<T>::handleAdds( std::vector<int32_t> &deleteIds )
{
	
}
	
template<typename T>
void Client::ProfileHandler<T>::handleMessage( const osc::Message &message )
{
	auto messageType = message[0].string();
	auto currentTime = app::getElapsedSeconds();
	std::string source;// = message.getRemoteIp();
	
	mSources.insert( source );
	
	if( messageType == "source" ) {
		
	}
	if( messageType == "set" ) {
		T inst( message );
		
		if( mInstances[source].find( inst.getSessionId() ) == mInstances[source].end() )
			mProfileAdds[source].push_back( inst );
		else
			mProfileUpdates[source].push_back( inst );
	}
	else if( messageType == "alive" ) {
		set<int32_t> aliveInstances;
		for( int i = 1; i < message.getNumArgs(); i++ )
			aliveInstances.insert( message[i].int32() );
		
		// anything not in 'aliveInstances' has been removed
		
		// We look at all (and only) the instances owned by the source of the message
		auto instanceMap = mInstances.find( source );
		if ( instanceMap != mInstances.end() ) {
			for( auto & inst : instanceMap->second ) {
				if( aliveInstances.find( inst.first ) == aliveInstances.end() )
					mProfileDeletes[source].push_back( inst.second );
			}
		}
	}
	else if( messageType == "fseq" ) {
		auto frame = message[1].int32();
		
		// due to UDP's unpredictability, it is possible to receive messages from "the past". Don't process these updates if that's true here
		// note that a frame of -1 implies that this is just an update, but doesn't represent a new time so we'll just process it
		
		// If the frame is "too far" in the past, we assume that the source has
		// been reset/restarted, or it's a different source, and we accept it.
		int32_t prev_frame = mPreviousFrame[source];
		int32_t dframe = frame - prev_frame;
		
		if( ( frame == -1 ) || ( dframe > 0 ) || ( dframe < -mPastFrameThreshold ) ) {
			auto windowSize = app::getWindowSize();
			auto window = app::WindowRef();
			// propagate the newly added instances
			vector<app::TouchEvent::Touch> beganTouches;
			for( auto & added : mProfileAdds[source] ) {
				mAddedSignal( added );
				mInstances[source][added.getSessionId()] = added;
				beganTouches.push_back( added.getTouch( currentTime, windowSize ) );
			}
			
			// send a touchesBegan
			if( ! beganTouches.empty() )
				mTouchesBeganSignal( app::TouchEvent( window, beganTouches ) );
			
			// propagate the updated instances
			vector<app::TouchEvent::Touch> movedTouches;
			for( auto & updated : mProfileUpdates[source] ) {
				mUpdatedSignal( updated );
				mInstances[source][updated.getSessionId()] = updated;
				movedTouches.push_back( updated.getTouch( currentTime, windowSize ) );
			}
			
			// send a touchesMoved
			if( ! movedTouches.empty() )
				mTouchesMovedSignal( app::TouchEvent( window, movedTouches ) );
			
			// propagate the deleted instances
			vector<app::TouchEvent::Touch> endedTouches;
			auto endIt = mProfileDeletes[source].end();
			for( auto removedIt = mProfileDeletes[source].begin(); removedIt != endIt; ++removedIt ) {
				mRemovedSignal( *removedIt );
				auto removedId = removedIt->getSessionId();
				auto removedTouch = mInstances[source][removedId].getTouch( currentTime, windowSize );
				endedTouches.push_back( removedTouch );
				
				// call this last - we're using it in the callbacks
				mInstances[source].erase( removedId );
			}
			
			// send a touchesEnded
			if( ! endedTouches.empty() )
				mTouchesEndedSignal( app::TouchEvent( app::WindowRef(), endedTouches ) );
			
			mPreviousFrame[source] = ( frame == -1 ) ? mPreviousFrame[source] : frame;
		}
		
		mProfileUpdates[source].clear();
		mProfileAdds[source].clear();
		mProfileDeletes[source].clear();
	}
}
//
//template<typename T>
//vector<T> Client::ProfileHandler<T>::getInstancesAsVector( const std::string &source ) const
//{
//	vector<T> result;
//	
//	if( source == "" ) {
//		// Get instances across all sources
//		for( auto & s : mSources ) {
//			auto instanceMap = mInstances.find(s);
//			if ( instanceMap != mInstances.end() ) {
//				for ( auto & inst : instanceMap->second )
//					result.push_back( inst.second );
//			}
//		}
//	}
//	else {
//		// We collect only the instances owned by the specified source
//		auto instanceMap = mInstances.find(source);
//		if ( instanceMap != mInstances.end() ) {
//			for ( auto & inst : instanceMap->second )
//				result.push_back( inst.second );
//		}
//	}
//	return result;
//}
//
//Client::Client( uint16_t port, asio::io_service &io )
//: mListener( port, udp::v4(), io ),
//mHandlerObject( new ProfileHandler<Object>( DEFAULT_PAST_FRAME_THRESHOLD ) ),
//mHandlerCursor( new ProfileHandler<Cursor>( DEFAULT_PAST_FRAME_THRESHOLD ) ),
//mHandlerCursor25d( new ProfileHandler<Cursor25d>( DEFAULT_PAST_FRAME_THRESHOLD ) )
//{
//}
//
//void Client::connect()
//{
//	mListener.bind();
//	mListener.setListener( "/tuio/2Dobj", std::bind( &ProfileHandler<Object>::handleMessage, mHandlerObject.get(), std::placeholders::_1 ) );
//	mListener.setListener( "/tuio/2Dcur", std::bind( &ProfileHandler<Cursor>::handleMessage, mHandlerCursor.get(), std::placeholders::_1 ) );
//	mListener.setListener( "/tuio/25Dcur", std::bind( &ProfileHandler<Cursor25d>::handleMessage, mHandlerCursor25d.get(), std::placeholders::_1 ) );
//	mListener.listen();
//	mConnected = true;
//}
//
//void Client::disconnect()
//{
//	lock_guard<mutex> lock( mMutex );
//	mListener.close();
//	mConnected = false;
//}
//
//
//vector<app::TouchEvent::Touch> Client::getActiveTouches( std::string source ) const
//{
//	lock_guard<mutex> lock( mMutex );
//	
//	double currentTime = app::getElapsedSeconds();
//	vector<app::TouchEvent::Touch> result;
//	if ( source == "" ) {
//		// Get cursors from all sources
//		auto & sources = getSources();
//		int sourcenum = 0;
//		for( auto & source :sources ) {
//			vector<Cursor> cursors = mHandlerCursor->getInstancesAsVector( source );
//			for( auto & inst : cursors ) {
//				result.push_back( inst.getTouch( currentTime, app::getWindowSize() ) );
//			}
//			++sourcenum;
//		}
//	} else {
//		// Get cursors from one source
//		auto cursors = mHandlerCursor->getInstancesAsVector(source);
//		for( auto & inst : cursors ) {
//			result.push_back( inst.getTouch( currentTime, app::getWindowSize() ) );
//		}
//	}
//	
//	return result;	
//}
//
}} // namespace tuio // namespace cinder