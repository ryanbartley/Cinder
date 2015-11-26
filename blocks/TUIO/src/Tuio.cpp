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

namespace cinder { namespace  tuio {
	
namespace detail {
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Profile
	
class Profile {
public:
	int32_t getSessionId() const { return mSessionId; }
	const std::string& getSource() const { return mSource; }
	void setSource( const std::string &source ) { mSource = source; }
	bool operator<( const Profile &other );
	
protected:
	Profile( const osc::Message &msg );
	Profile( const Profile &other ) = default;
	Profile( Profile &&other ) NOEXCEPT;
	Profile& operator=( const Profile &other ) = default;
	Profile& operator=( Profile &&other ) NOEXCEPT;
	~Profile() = default;
	
	int32_t		mSessionId;
	std::string mSource;
};
	
bool Profile::operator<(const Profile & other)
{
	return mSource < other.mSource &&
		mSessionId < other.mSessionId;
}

Profile::Profile( const osc::Message &msg )
: mSessionId( msg[1].int32() )
{
}

Profile::Profile( Profile &&other ) NOEXCEPT
: mSessionId( other.mSessionId ), mSource( std::move( other.mSource ) )
{
}

Profile& Profile::operator=( Profile &&other ) NOEXCEPT
{
	if ( this != &other ) {
		mSessionId = other.mSessionId;
		mSource = std::move( other.mSource );
	}
	return *this;
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Cursor
	
template<typename VEC_T>
class Cursor : public Profile {
public:
	Cursor( const osc::Message &msg );
	
	Cursor( const Cursor &other ) = default;
	Cursor( Cursor &&other ) NOEXCEPT;
	Cursor& operator=( const Cursor &other ) = default;
	Cursor& operator=( Cursor &&other ) NOEXCEPT;
	~Cursor() = default;
	
	const VEC_T&	getPosition() const { return mPosition; }
	const VEC_T&	getVelocity() const { return mVelocity; }
	float			getAcceleration() const { return mAcceleration; }
	
	app::TouchEvent::Touch	convertToTouch() const;
	
protected:
	VEC_T		mPosition,
	mVelocity;
	float		mAcceleration;
};

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

template<typename VEC_T>
Cursor<VEC_T>::Cursor( Cursor<VEC_T> &&other ) NOEXCEPT
: Profile( other ), mPosition( std::move( other.mPosition ) ), 
	mVelocity( std::move( other.mVelocity ) ), mAcceleration( other.mAcceleration )
{
}

template<typename VEC_T>
Cursor<VEC_T>& Cursor<VEC_T>::operator=( Cursor<VEC_T> &&other ) NOEXCEPT
{
	if( this != &other ) {
		Profile::operator=( other );
		mPosition = std::move( other.mPosition );
		mVelocity = std::move( other.mVelocity );
		mAcceleration = other.mAcceleration;
	}
	return *this;
}
	
template<typename T>
app::TouchEvent::Touch Cursor<T>::convertToTouch() const
{
	app::TouchEvent::Touch ret( vec2(getPosition()),
								vec2(getPosition() - getVelocity()),
								getSessionId(),
								app::getElapsedSeconds(),
								nullptr );
	return ret;
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Object
	
template<typename VEC_T, typename ROT_T>
class Object : public Profile {
public:
	Object( const osc::Message &msg );
	
	Object( const Object &other ) = default;
	Object( Object &&other ) NOEXCEPT;
	Object& operator=( const Object &other ) = default;
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

template<typename VEC_T, typename ROT_T>
Object<VEC_T, ROT_T>::Object( Object<VEC_T, ROT_T> &&other ) NOEXCEPT
: Profile( other ), mClassId( other.mClassId ), mPosition( std::move( other.mPosition ) ),
	mVelocity( std::move( other.mVelocity ) ), mAngle( std::move( other.mAngle ) ),
	mRotationVelocity( std::move( other.mRotationVelocity ) ), mAcceleration( other.mAcceleration ),
	mRotateAccel( other.mRotateAccel )
{
}

template<typename VEC_T, typename ROT_T>
Object<VEC_T, ROT_T>& Object<VEC_T, ROT_T>::operator=( Object<VEC_T, ROT_T> &&other ) NOEXCEPT
{
	if( this != &other ) {
		Profile::operator=( other );
		mClassId = other.mClassId;
		mPosition = std::move( other.mPosition );
		mVelocity = std::move( other.mVelocity );
		mAngle = std::move( other.mAngle );
		mRotationVelocity = std::move( other.mRotationVelocity );
		mAcceleration = other.mAcceleration;
		mRotateAccel = other.mRotateAccel;
	}
	return *this;
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Blob

template<typename VEC_T, typename ROT_T, typename DIM_T>
class Blob : public Profile {
public:
	Blob( const osc::Message &msg );
	
	Blob( const Blob &other ) = default;
	Blob( Blob &&other ) NOEXCEPT;
	Blob& operator=( const Blob &other ) = default;
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

template<typename VEC_T, typename ROT_T, typename DIM_T>
Blob<VEC_T, ROT_T, DIM_T>::Blob( Blob<VEC_T, ROT_T, DIM_T> &&other ) NOEXCEPT
: Profile( other ), mPosition( std::move( other.mPosition ) ),
	mVelocity( std::move( other.mVelocity ) ), mAngle( std::move( other.mAngle ) ),
	mRotationVelocity( std::move( other.mRotationVelocity ) ), 
	mDimensions( std::move( other.mDimensions ) ), mAcceleration( other.mAcceleration ),
	mRotateAccel( other.mRotateAccel ), mGeometry( other.mGeometry )
{
}

template<typename VEC_T, typename ROT_T, typename DIM_T>
Blob<VEC_T, ROT_T, DIM_T>& Blob<VEC_T, ROT_T, DIM_T>::operator=( Blob<VEC_T, ROT_T, DIM_T> &&other ) NOEXCEPT
{
	if( this != &other ) {
		Profile::operator=( other );
		mPosition = std::move( other.mPosition );
		mVelocity = std::move( other.mVelocity );
		mAngle = std::move( other.mAngle );
		mRotationVelocity = std::move( other.mRotationVelocity );
		mDimensions = std::move( other.mDimensions );
		mAcceleration = other.mAcceleration;
		mRotateAccel = other.mRotateAccel;
		mGeometry = other.mGeometry;
	}
	return *this;
}
	
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
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Blob2D

Blob2D::Blob2D( const osc::Message &msg )
: Blob( msg )
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///// Blob25D

Blob25D::Blob25D( const osc::Message &msg )
: Blob( msg )
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///// Blob3D

Blob3D::Blob3D( const osc::Message &msg )
: Blob( msg )
{
}

template<typename T>
using ProfileFn = Listener::ProfileFn<T>;

template<typename CallbackType, typename ProfileType = CallbackType>
class ProfileHandler : public ProfileHandlerBase {
public:
	//! TODO: Need to figure out about "PastFrameThreshold", got rid of it.
	ProfileHandler() {}
	
	void setAddHandler( ProfileFn<CallbackType> callback );
	void setUpdateHandler( ProfileFn<CallbackType> callback );
	void setRemoveHandler( ProfileFn<CallbackType> callback );
	void handleMessage( const osc::Message &message ) override;
	void handleAdds( std::vector<int32_t> &deleteIds );
	void handleUpdates();
	void handleRemoves();
	
	std::vector<ProfileType> getCurrentActiveProfiles();
	
	std::string						mCurrentSource;
	std::vector<int32_t>			mAdded, mUpdated;
	std::vector<ProfileType>		mSetOfCurrentTouches,
	mRemovedTouches;
	std::map<std::string, int32_t>	mSourceFrameNums;
	// containers for changes which will be propagated upon receipt of 'fseq'
	ProfileFn<CallbackType>			mAddCallback, mUpdateCallback, mRemoveCallback;
	std::mutex						mAddMutex, mUpdateMutex, mRemoveMutex;
};

template<>
class ProfileHandler<ci::app::TouchEvent, Cursor2D> : public ProfileHandlerBase {
	//! TODO: Need to figure out about "PastFrameThreshold", got rid of it.
	ProfileHandler() {}
	
	void setAddHandler( ProfileFn<ci::app::TouchEvent> callback );
	void setUpdateHandler( ProfileFn<ci::app::TouchEvent> callback );
	void setRemoveHandler( ProfileFn<ci::app::TouchEvent> callback );
	void handleMessage( const osc::Message &message ) override;
	
	std::vector<Cursor2D> getCurrentActiveProfiles();
	
	std::string						mCurrentSource;
	std::vector<int32_t>			mAdded, mUpdated;
	std::vector<Cursor2D>			mSetOfCurrentTouches,
	mRemovedTouches;
	std::map<std::string, int32_t>	mSourceFrameNums;
	// containers for changes which will be propagated upon receipt of 'fseq'
	ProfileFn<ci::app::TouchEvent>	mAddCallback, mUpdateCallback, mRemoveCallback;
	std::mutex						mAddMutex, mUpdateMutex, mRemoveMutex;
};
	
template<typename CallbackType, typename ProfileType>
void ProfileHandler<CallbackType, ProfileType>::setAddHandler( ProfileFn<CallbackType> callback )
{
	std::lock_guard<std::mutex> lock( mAddMutex );
	mAddCallback = callback;
}

template<typename CallbackType, typename ProfileType>
void ProfileHandler<CallbackType, ProfileType>::setUpdateHandler( ProfileFn<CallbackType> callback )
{
	std::lock_guard<std::mutex> lock( mUpdateMutex );
	mUpdateCallback = callback;
}

template<typename CallbackType, typename ProfileType>
void ProfileHandler<CallbackType, ProfileType>::setRemoveHandler( ProfileFn<CallbackType> callback )
{
	std::lock_guard<std::mutex> lock( mRemoveMutex );
	mRemoveCallback = callback;
}

template<typename CallbackType, typename ProfileType>
void ProfileHandler<CallbackType, ProfileType>::handleMessage( const osc::Message &message )
{
	auto messageType = message[0].string();
	
	using namespace std;
	if( messageType == "source" ) {
		mCurrentSource = message[1].string();
	}
	else if( messageType == "set" ) {
		auto sessionId = message[1].int32();
		auto it = find_if( begin( mSetOfCurrentTouches ), end( mSetOfCurrentTouches ),
						  [sessionId]( const ProfileType &profile){
							  return sessionId == profile.getSessionId();
						  });
		if( it == mSetOfCurrentTouches.end() ) {
			ProfileType profile( message );
			auto insert = lower_bound( begin( mSetOfCurrentTouches ),
									  end( mSetOfCurrentTouches ),
									  profile.getSessionId(),
									  []( const ProfileType &lhs, int32_t value ){
										  return lhs.getSessionId() < value;
									  });
			mSetOfCurrentTouches.insert( insert, std::move( message ) );
			mSetOfCurrentTouches.back().setSource( mCurrentSource );
			mAdded.push_back( sessionId );
		}
		else {
			*it = std::move( ProfileType( message ) );
			it->setSource( mCurrentSource );
			mUpdated.push_back( sessionId );
		}
	}
	else if( messageType == "alive" ) {
		std::vector<int32_t> aliveIds( message.getNumArgs() - 1 );
		int i = 1;
		for( auto & aliveId : aliveIds ) {
			aliveId = message[i++].int32();
		}
		std::sort( aliveIds.begin(), aliveIds.end() );
		auto remove = remove_if( begin( mSetOfCurrentTouches ), end( mSetOfCurrentTouches ),
								[&aliveIds]( const ProfileType &profile ) {
									return !binary_search( begin(aliveIds), end(aliveIds), profile.getSessionId() );
								});
		if( remove != mSetOfCurrentTouches.end() ) {
			std::move( remove, mSetOfCurrentTouches.end(), std::back_inserter( mRemovedTouches ) );
			mSetOfCurrentTouches.erase( remove, mSetOfCurrentTouches.end() );
		}
	}
	else if( messageType == "fseq" ) {
		auto frame = message[1].int32();
		int32_t prev_frame = mSourceFrameNums[mCurrentSource];
		int32_t delta_frame = frame - prev_frame;
		// TODO: figure out about past frame threshold updating, this was also in the if condition ( dframe < -mPastFrameThreshold )
		if( frame == -1 || delta_frame > 0 ) {
			auto begin = mSetOfCurrentTouches.cbegin();
			auto end = mSetOfCurrentTouches.cend();
			if( ! mAdded.empty() ) {
				std::lock_guard<std::mutex> lock( mAddMutex );
				if( mAddCallback ) {
					for( auto & added : mAdded ) {
						auto found = find_if( begin, end,
											 [added]( const ProfileType &profile ) {
												 return added == profile.getSessionId();
											 });
						if( found != end )
							mAddCallback( *found );
					}
				}
				mAdded.clear();
			}
			if ( ! mUpdated.empty() ) {
				std::lock_guard<std::mutex> lock( mUpdateMutex );
				if( mUpdateCallback ) {
					for( auto & updated : mUpdated ) {
						auto found = find_if( begin, end,
											 [updated]( const ProfileType &profile ) {
												 return updated == profile.getSessionId();
											 });
						if( found != end )
							mUpdateCallback( *found );
					}
				}
				mUpdated.clear();
			}
			if( ! mRemovedTouches.empty() ){
				std::lock_guard<std::mutex> lock( mRemoveMutex );
				if( mRemoveCallback )
					for( auto & removed : mRemovedTouches ) {
						mRemoveCallback( removed );
					}
				mRemovedTouches.clear();
			}
			mSourceFrameNums[mCurrentSource] = ( frame == -1 ) ? mSourceFrameNums[mCurrentSource] : frame;
		}
	}
}

	
} // namespace detail
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Listener
	
Listener::Listener( const app::WindowRef &window,  uint16_t localPort, const asio::ip::udp &protocol, asio::io_service &io )
: mListener( new osc::ReceiverUdp( localPort, protocol, io ) ), mWindow( window )
{
	
}
	
Listener::Listener( const osc::ReceiverBase *ptr )
{
	
}

void Listener::bind()
{
	mListener->bind();
}
	
void Listener::listen()
{
	mListener->listen();
}

void Listener::close()
{
	mListener->close();
}
	
template<typename TuioType>
void Listener::setAdded( ProfileFn<TuioType> callback )
{
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto profile = dynamic_cast<detail::ProfileHandler<TuioType>*>(found->second.get());
		profile->setAddHandler( callback );
	}
	else {
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::ProfileHandler<TuioType>>( new detail::ProfileHandler<TuioType>() ) );
		auto created = dynamic_cast<detail::ProfileHandler<TuioType>*>(inserted.first->second.get());
		created->setAddHandler( callback );
		mListener->setListener( address, std::bind( &detail::ProfileHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}


template<typename TuioType>
void Listener::setUpdated( ProfileFn<TuioType> callback )
{
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto profile = dynamic_cast<detail::ProfileHandler<TuioType>*>(found->second.get());
		profile->setUpdateHandler( callback );
	}
	else {
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::ProfileHandler<TuioType>>( new detail::ProfileHandler<TuioType>() ) );
		auto created = dynamic_cast<detail::ProfileHandler<TuioType>*>(inserted.first->second.get());
		created->setUpdateHandler( callback );
		mListener->setListener( address, std::bind( &detail::ProfileHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}

template<typename TuioType>
void Listener::setRemoved( ProfileFn<TuioType> callback )
{
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto profile = dynamic_cast<detail::ProfileHandler<TuioType>*>(found->second.get());
		profile->setRemoveHandler( callback );
	}
	else {
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::ProfileHandler<TuioType>>( new detail::ProfileHandler<TuioType>() ) );
		auto created = dynamic_cast<detail::ProfileHandler<TuioType>*>(inserted.first->second.get());
		created->setRemoveHandler( callback );
		mListener->setListener( address, std::bind( &detail::ProfileHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}

template<typename TuioType>
void Listener::remove()
{
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		mListener->removeListener( address );
		mHandlers.erase( found );
	}
}

template<typename T>
const char* Listener::getOscAddressFromType()
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
	// TODO: decide if this is good. Probably not but let's see
	else if( std::is_same<T, cinder::app::TouchEvent>::value ) return "/tuio/2Dcur";
	else return "Unknown Target";
}

template<typename ProfileType>
vector<ProfileType> Listener::getActiveProfiles() const
{
//	double currentTime = app::getElapsedSeconds();
	vector<app::TouchEvent::Touch> result;
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
	return result;	
}


	
//template<>
//void Listener::setProfileAddedCallback( ProfileFn<TouchEvent> callback )
//{
//	auto address = getOscAddressFromType<TouchEvent>();
//	auto found = mHandlers.find( address );
//	if( found != mHandlers.end() ) {
//		auto profile = dynamic_cast<detail::ProfileHandler<TouchEvent, Cursor2D>*>(found->second.get());
//		profile->setAddHandler( callback );
//	}
//	else {
//		// TODO: Add listener to osc here
//		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::ProfileHandler<TouchEvent, Cursor2D>>( new detail::ProfileHandler<TouchEvent, Cursor2D>() ) );
//		auto created = dynamic_cast<detail::ProfileHandler<TouchEvent, Cursor2D>*>(inserted.first->second.get());
//		created->setAddHandler( callback );
//	}
//}
//
//	
//template<>
//void Listener::setProfileUpdatedCallback( ProfileFn<TouchEvent> callback )
//{
//	auto address = getOscAddressFromType<TouchEvent>();
//	auto found = mHandlers.find( address );
//	if( found != mHandlers.end() ) {
//		auto profile = dynamic_cast<detail::ProfileHandler<TouchEvent, Cursor2D>*>(found->second.get());
//		profile->setAddHandler( callback );
//	}
//	else {
//		// TODO: Add listener to osc here
//		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::ProfileHandler<TouchEvent, Cursor2D>>( new detail::ProfileHandler<TouchEvent, Cursor2D>() ) );
//		auto created = dynamic_cast<detail::ProfileHandler<TouchEvent, Cursor2D>*>(inserted.first->second.get());
//		created->setUpdateHandler( callback );
//	}
//}
//
//template<>
//void Listener::setProfileRemovedCallback( ProfileFn<TouchEvent> callback )
//{
//	auto address = getOscAddressFromType<TouchEvent>();
//	auto found = mHandlers.find( address );
//	if( found != mHandlers.end() ) {
//		auto profile = dynamic_cast<detail::ProfileHandler<TouchEvent, Cursor2D>*>(found->second.get());
//		profile->setAddHandler( callback );
//	}
//	else {
//		// TODO: Add listener to osc here
//		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::ProfileHandler<TouchEvent, Cursor2D>>( new detail::ProfileHandler<TouchEvent, Cursor2D>() ) );
//		auto created = dynamic_cast<detail::ProfileHandler<TouchEvent, Cursor2D>*>(inserted.first->second.get());
//		created->setRemoveHandler( callback );
//	}
//}
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Detail
	
namespace detail {
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// ProfileHandler templated
	

	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// ProfileHandler specialized (callback - TouchEvent, profile - Cursor2D)
	
void ProfileHandler<TouchEvent, Cursor2D>::setAddHandler( ProfileFn<TouchEvent> callback )
{
	std::lock_guard<std::mutex> lock( mAddMutex );
	mAddCallback = callback;
}
	
void ProfileHandler<TouchEvent, Cursor2D>::setUpdateHandler( ProfileFn<TouchEvent> callback )
{
	std::lock_guard<std::mutex> lock( mUpdateMutex );
	mUpdateCallback = callback;
}

void ProfileHandler<TouchEvent, Cursor2D>::setRemoveHandler( ProfileFn<TouchEvent> callback )
{
	std::lock_guard<std::mutex> lock( mRemoveMutex );
	mRemoveCallback = callback;
}
	
void ProfileHandler<TouchEvent, Cursor2D>::handleMessage( const osc::Message &message )
{
	auto messageType = message[0].string();
	
	if( messageType == "source" ) {
		mCurrentSource = message[1].string();
	}
	else if( messageType == "set" ) {
		auto sessionId = message[1].int32();
		auto it = find_if( begin( mSetOfCurrentTouches ), end( mSetOfCurrentTouches ),
		[sessionId]( const Cursor2D &profile){
			return sessionId == profile.getSessionId();
		});
		if( it == mSetOfCurrentTouches.end() ) {
			Cursor2D profile( message );
			// TODO: this lambda is wrong.
			auto insert = lower_bound( begin( mSetOfCurrentTouches ), end( mSetOfCurrentTouches ), profile.getSessionId(),
			[]( const Cursor2D &lhs, const int val ){
				return lhs.getSessionId() < val;
			});
			mSetOfCurrentTouches.insert( insert, std::move( message ) );
			mSetOfCurrentTouches.back().setSource( mCurrentSource );
			mAdded.push_back( sessionId );
		}
		else {
			*it = std::move( Cursor2D( message ) );
			it->setSource( mCurrentSource );
			mUpdated.push_back( sessionId );
		}
	}
	else if( messageType == "alive" ) {
		std::vector<int32_t> aliveIds( message.getNumArgs() - 1 );
		int i = 1;
		for( auto & aliveId : aliveIds ) {
			aliveId = message[i++].int32();
		}
		std::sort( aliveIds.begin(), aliveIds.end() );
		auto remove = remove_if( begin( mSetOfCurrentTouches ), end( mSetOfCurrentTouches ),
		[&aliveIds]( const Cursor2D &profile ) {
			return binary_search( begin(aliveIds), end(aliveIds), profile.getSessionId() );
		});
		std::move( remove, mSetOfCurrentTouches.end(),
				  std::inserter( mRemovedTouches, mRemovedTouches.begin() ) );
	}
	else if( messageType == "fseq" ) {
		auto frame = message[1].int32();
		int32_t prev_frame = mSourceFrameNums[mCurrentSource];
		int32_t delta_frame = frame - prev_frame;
		// TODO: figure out about past frame threshold updating, this was also in the if condition ( dframe < -mPastFrameThreshold )
		if( frame == -1 || delta_frame > 0 ) {
			auto begin = mSetOfCurrentTouches.cbegin();
			auto end = mSetOfCurrentTouches.cend();
			if( ! mAdded.empty() ) {
				std::lock_guard<std::mutex> lock( mAddMutex );
				if( mAddCallback ) {
					std::vector<TouchEvent::Touch> mAddTouches;
					for( auto & added : mAdded ) {
						auto found = find_if( begin, end,
						[added]( const Cursor2D &profile ){
							 return added == profile.getSessionId();
						});
						if( found != end )
							mAddTouches.emplace_back( found->convertToTouch() );
					}
					mAddCallback( TouchEvent( ci::app::getWindow(), mAddTouches ) );
				}
			}
			if ( ! mUpdated.empty() ) {
				std::lock_guard<std::mutex> lock( mUpdateMutex );
				if( mUpdateCallback ) {
					std::vector<TouchEvent::Touch> mUpdateTouches;
					for( auto & updated : mUpdated ) {
						auto found = find_if( begin, end,
						[updated]( const Cursor2D &profile ){
							 return updated == profile.getSessionId();
						});
						if( found != end )
							mUpdateTouches.emplace_back( found->convertToTouch() );
					}
					mUpdateCallback( TouchEvent( ci::app::getWindow(), mUpdateTouches ) );
				}
			}
			if( ! mRemovedTouches.empty() ){
				std::lock_guard<std::mutex> lock( mRemoveMutex );
				if( mRemoveCallback ) {
					std::vector<TouchEvent::Touch> mRemoveTouches;
					for( auto & removed : mRemovedTouches ) {
						mRemoveTouches.emplace_back(  removed.convertToTouch() );
					}
					mRemoveCallback( TouchEvent( ci::app::getWindow(), mRemoveTouches ) );
				}
			}
			
			//mPreviousFrame[source] = ( frame == -1 ) ? mPreviousFrame[source] : frame;
		}
	}
}
	
	
		
} // namespace detail

template void Listener::setAdded( ProfileFn<tuio::Cursor2D> );
template void Listener::setRemoved( ProfileFn<tuio::Cursor2D> );
template void Listener::setUpdated( ProfileFn<tuio::Cursor2D> );
	
template class detail::Cursor<vec2>;
template class detail::Cursor<vec3>;

}}  // namespace tuio // namespace cinder