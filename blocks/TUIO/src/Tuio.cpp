//
//  Tuio.cpp
//  TUIOReceiver
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
///// Type
	
bool Type::operator<(const Type & other)
{
	return mSource < other.mSource &&
		mSessionId < other.mSessionId;
}

Type::Type( const osc::Message &msg )
: mSessionId( msg[1].int32() )
{
}

Type::Type( Type &&other ) NOEXCEPT
: mSessionId( other.mSessionId ), mSource( std::move( other.mSource ) )
{
}

Type& Type::operator=( Type &&other ) NOEXCEPT
{
	if ( this != &other ) {
		mSessionId = other.mSessionId;
		mSource = std::move( other.mSource );
	}
	return *this;
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Cursor
	
template class Cursor<vec2>;
template class Cursor<vec3>;

template<>
Cursor<vec2>::Cursor( const osc::Message &msg )
: Type( msg ), mPosition( msg[2].flt(), msg[3].flt() ),
	mVelocity( msg[4].flt(), msg[5].flt() ), mAcceleration( msg[6].flt() )
{
}
	
template<>
Cursor<vec3>::Cursor( const osc::Message &msg )
: Type( msg ), mPosition( msg[2].flt(), msg[3].flt(), msg[4].flt() ),
mVelocity( msg[5].flt(), msg[6].flt(), msg[7].flt() ), mAcceleration( msg[8].flt() )
{
}

template<typename VEC_T>
Cursor<VEC_T>::Cursor( Cursor<VEC_T> &&other ) NOEXCEPT
: Type( other ), mPosition( std::move( other.mPosition ) ), 
	mVelocity( std::move( other.mVelocity ) ), mAcceleration( other.mAcceleration )
{
}

template<typename VEC_T>
Cursor<VEC_T>& Cursor<VEC_T>::operator=( Cursor<VEC_T> &&other ) NOEXCEPT
{
	if( this != &other ) {
		Type::operator=( other );
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
	
template class Object<ci::vec2, float>;
template class Object<ci::vec3, float>;
template class Object<ci::vec3, ci::vec3>;
	
template<>
Object<ci::vec2, float>::Object( const osc::Message &msg )
: Type( msg ), mClassId( msg[2].int32() ), mPosition( msg[3].flt(), msg[4].flt() ),
	mAngle( msg[5].flt() ), mVelocity( msg[6].flt(), msg[7].flt() ),
	mRotationVelocity( msg[8].flt() ), mAcceleration( msg[9].flt() ),
	mRotateAccel( msg[10].flt() )
{
}
	
template<>
Object<ci::vec3, float>::Object( const osc::Message &msg )
: Type( msg ), mClassId( msg[2].int32() ),
	mPosition( msg[3].flt(), msg[4].flt(), msg[5].flt() ),
	mAngle( msg[6].flt() ), mVelocity( msg[7].flt(), msg[8].flt(), msg[9].flt() ),
	mRotationVelocity( msg[10].flt() ), mAcceleration( msg[11].flt() ),
	mRotateAccel( msg[12].flt() )
{
}
	
template<>
Object<ci::vec3, ci::vec3>::Object( const osc::Message &msg )
: Type( msg ), mClassId( msg[2].int32() ),
	mPosition(	msg[3].flt(), msg[4].flt(), msg[5].flt() ),
	mAngle(		msg[6].flt(), msg[7].flt(), msg[8].flt() ),
	mVelocity(	msg[9].flt(), msg[10].flt(), msg[11].flt() ),
	mRotationVelocity( msg[12].flt(), msg[13].flt(), msg[14].flt() ),
	mAcceleration( msg[15].flt() ), mRotateAccel( msg[16].flt() )
{
}

template<typename VEC_T, typename ROT_T>
Object<VEC_T, ROT_T>::Object( Object<VEC_T, ROT_T> &&other ) NOEXCEPT
: Type( other ), mClassId( other.mClassId ), mPosition( std::move( other.mPosition ) ),
	mVelocity( std::move( other.mVelocity ) ), mAngle( std::move( other.mAngle ) ),
	mRotationVelocity( std::move( other.mRotationVelocity ) ), mAcceleration( other.mAcceleration ),
	mRotateAccel( other.mRotateAccel )
{
}

template<typename VEC_T, typename ROT_T>
Object<VEC_T, ROT_T>& Object<VEC_T, ROT_T>::operator=( Object<VEC_T, ROT_T> &&other ) NOEXCEPT
{
	if( this != &other ) {
		Type::operator=( other );
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
	
template<>
Blob<ci::vec2, float, ci::vec2>::Blob( const osc::Message &msg )
: Type( msg ), mPosition( msg[2].flt(), msg[3].flt() ), mAngle( msg[4].flt() ),
	mDimensions( msg[5].flt(), msg[6].flt() ), mGeometry( msg[7].flt() ),
	mVelocity( msg[8].flt(), msg[9].flt() ), mRotationVelocity( msg[10].flt() ),
	mAcceleration( msg[11].flt() ), mRotateAccel( msg[12].flt() )
{
}
	
template<>
Blob<ci::vec3, float, ci::vec2>::Blob( const osc::Message &msg )
: Type( msg ), mPosition( msg[2].flt(), msg[3].flt(), msg[4].flt() ),
	mAngle( msg[5].flt() ), mDimensions( msg[6].flt(), msg[7].flt() ),
	mGeometry( msg[8].flt() ), mVelocity( msg[9].flt(), msg[10].flt(), msg[11].flt() ),
	mRotationVelocity( msg[12].flt() ), mAcceleration( msg[13].flt() ),
	mRotateAccel( msg[14].flt() )
{
}
	
template<>
Blob<ci::vec3, ci::vec3, ci::vec3>::Blob( const osc::Message &msg )
: Type( msg ), mPosition( msg[2].flt(), msg[3].flt(), msg[4].flt() ),
	mAngle( msg[5].flt(), msg[6].flt(), msg[7].flt() ),
	mDimensions( msg[8].flt(), msg[9].flt(), msg[10].flt() ),
	mGeometry( msg[11].flt() ), mVelocity( msg[12].flt(), msg[13].flt(), msg[14].flt() ),
	mRotationVelocity( msg[15].flt(), msg[16].flt(), msg[17].flt() ),
	mAcceleration( msg[18].flt() ), mRotateAccel( msg[19].flt() )
{
}

template<typename VEC_T, typename ROT_T, typename DIM_T>
Blob<VEC_T, ROT_T, DIM_T>::Blob( Blob<VEC_T, ROT_T, DIM_T> &&other ) NOEXCEPT
: Type( other ), mPosition( std::move( other.mPosition ) ),
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
		Type::operator=( other );
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
using TypeFn = Receiver::TypeFn<T>;

template<typename CallbackType, typename TuioType = CallbackType>
class TypeHandler : public TypeHandlerBase {
public:
	//! TODO: Need to figure out about "PastFrameThreshold", got rid of it.
	TypeHandler() {}
	
	void setAddHandler( TypeFn<CallbackType> callback );
	void setUpdateHandler( TypeFn<CallbackType> callback );
	void setRemoveHandler( TypeFn<CallbackType> callback );
	void handleMessage( const osc::Message &message ) override;
	void handleAdds( std::vector<int32_t> &deleteIds );
	void handleUpdates();
	void handleRemoves();
	
	std::vector<TuioType> getCurrentActive();
	
	std::string						mCurrentSource;
	std::vector<int32_t>			mAdded, mUpdated;
	std::vector<TuioType>			mSetOfCurrentTouches,
									mRemovedTouches;
	std::map<std::string, int32_t>	mSourceFrameNums;
	// containers for changes which will be propagated upon receipt of 'fseq'
	TypeFn<CallbackType>			mAddCallback, mUpdateCallback, mRemoveCallback;
	std::mutex						mAddMutex, mUpdateMutex, mRemoveMutex;
};

template<>
class TypeHandler<ci::app::TouchEvent, Cursor2D> : public TypeHandlerBase {
public:
	//! TODO: Need to figure out about "PastFrameThreshold", got rid of it.
	TypeHandler() {}
	
	void setAddHandler( TypeFn<ci::app::TouchEvent> callback );
	void setUpdateHandler( TypeFn<ci::app::TouchEvent> callback );
	void setRemoveHandler( TypeFn<ci::app::TouchEvent> callback );
	void handleMessage( const osc::Message &message ) override;
	
	std::vector<Cursor2D> getCurrentActive( const std::string &source );
	
	std::string						mCurrentSource;
	std::vector<int32_t>			mAdded, mUpdated;
	std::vector<Cursor2D>			mSetOfCurrentTouches,
									mRemovedTouches;
	std::map<std::string, int32_t>	mSourceFrameNums;
	// containers for changes which will be propagated upon receipt of 'fseq'
	TypeFn<ci::app::TouchEvent>	mAddCallback, mUpdateCallback, mRemoveCallback;
	std::mutex						mAddMutex, mUpdateMutex, mRemoveMutex;
};
	
template<typename CallbackType, typename TuioType>
void TypeHandler<CallbackType, TuioType>::setAddHandler( TypeFn<CallbackType> callback )
{
	std::lock_guard<std::mutex> lock( mAddMutex );
	mAddCallback = callback;
}

template<typename CallbackType, typename TuioType>
void TypeHandler<CallbackType, TuioType>::setUpdateHandler( TypeFn<CallbackType> callback )
{
	std::lock_guard<std::mutex> lock( mUpdateMutex );
	mUpdateCallback = callback;
}

template<typename CallbackType, typename TuioType>
void TypeHandler<CallbackType, TuioType>::setRemoveHandler( TypeFn<CallbackType> callback )
{
	std::lock_guard<std::mutex> lock( mRemoveMutex );
	mRemoveCallback = callback;
}

template<typename CallbackType, typename TuioType>
void TypeHandler<CallbackType, TuioType>::handleMessage( const osc::Message &message )
{
	auto messageType = message[0].string();
	
	using namespace std;
	if( messageType == "source" ) {
		mCurrentSource = message[1].string();
	}
	else if( messageType == "set" ) {
		auto sessionId = message[1].int32();
		auto it = find_if( begin( mSetOfCurrentTouches ), end( mSetOfCurrentTouches ),
						  [sessionId]( const TuioType &type){
							  return sessionId == type.getSessionId();
						  });
		if( it == mSetOfCurrentTouches.end() ) {
			TuioType type( message );
			auto insert = lower_bound( begin( mSetOfCurrentTouches ),
									  end( mSetOfCurrentTouches ),
									  type.getSessionId(),
			[]( const TuioType &lhs, int32_t value ){
				  return lhs.getSessionId() < value;
			});
			mSetOfCurrentTouches.insert( insert, std::move( message ) );
			mSetOfCurrentTouches.back().setSource( mCurrentSource );
			mAdded.push_back( sessionId );
		}
		else {
			*it = std::move( TuioType( message ) );
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
		[&aliveIds]( const TuioType &type ) {
			return !binary_search( begin(aliveIds), end(aliveIds), type.getSessionId() );
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
						[added]( const TuioType &type ) {
							 return added == type.getSessionId();
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
											 [updated]( const TuioType &type ) {
												 return updated == type.getSessionId();
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
///// Receiver
	
Receiver::Receiver( const app::WindowRef &window,  uint16_t localPort, const asio::ip::udp &protocol, asio::io_service &io )
: mReceiver( new osc::ReceiverUdp( localPort, protocol, io ) ), mWindow( window )
{
	
}
	
Receiver::Receiver( const osc::ReceiverBase *ptr )
{
	
}

void Receiver::bind()
{
	mReceiver->bind();
}
	
void Receiver::listen()
{
	mReceiver->listen();
}

void Receiver::close()
{
	mReceiver->close();
}
	
template<typename TuioType>
void Receiver::setAdded( TypeFn<TuioType> callback )
{
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto typeHandler = dynamic_cast<detail::TypeHandler<TuioType>*>(found->second.get());
		typeHandler->setAddHandler( callback );
	}
	else {
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::TypeHandler<TuioType>>( new detail::TypeHandler<TuioType>() ) );
		auto created = dynamic_cast<detail::TypeHandler<TuioType>*>(inserted.first->second.get());
		created->setAddHandler( callback );
		mReceiver->setListener( address, std::bind( &detail::TypeHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}


template<typename TuioType>
void Receiver::setUpdated( TypeFn<TuioType> callback )
{
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto typeHandler = dynamic_cast<detail::TypeHandler<TuioType>*>(found->second.get());
		typeHandler->setUpdateHandler( callback );
	}
	else {
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::TypeHandler<TuioType>>( new detail::TypeHandler<TuioType>() ) );
		auto created = dynamic_cast<detail::TypeHandler<TuioType>*>(inserted.first->second.get());
		created->setUpdateHandler( callback );
		mReceiver->setListener( address, std::bind( &detail::TypeHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}

template<typename TuioType>
void Receiver::setRemoved( TypeFn<TuioType> callback )
{
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto typeHandler = dynamic_cast<detail::TypeHandler<TuioType>*>(found->second.get());
		typeHandler->setRemoveHandler( callback );
	}
	else {
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::TypeHandler<TuioType>>( new detail::TypeHandler<TuioType>() ) );
		auto created = dynamic_cast<detail::TypeHandler<TuioType>*>(inserted.first->second.get());
		created->setRemoveHandler( callback );
		mReceiver->setListener( address, std::bind( &detail::TypeHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}

template<typename TuioType>
void Receiver::remove()
{
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		mReceiver->removeListener( address );
		mHandlers.erase( found );
	}
}

template<typename T>
const char* Receiver::getOscAddressFromType()
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

template<typename TuioType>
vector<TuioType> Receiver::getActive( const std::string &source ) const
{
	auto found = mHandlers.find( getOscAddressFromType<TuioType>() );
	if( found != mHandlers.end() ) {
		return found->second->getCurrentActive( source );
	}
	else {
		return std::vector<TuioType>();
	}
}


	
template<>
void Receiver::setAdded( TypeFn<TouchEvent> callback )
{
	auto address = getOscAddressFromType<TouchEvent>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto typeHandler = dynamic_cast<detail::TypeHandler<TouchEvent, Cursor2D>*>(found->second.get());
		typeHandler->setAddHandler( callback );
	}
	else {
		// TODO: Add Receiver to osc here
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::TypeHandler<TouchEvent, Cursor2D>>( new detail::TypeHandler<TouchEvent, Cursor2D>() ) );
		auto created = dynamic_cast<detail::TypeHandler<TouchEvent, Cursor2D>*>(inserted.first->second.get());
		created->setAddHandler( callback );
	}
}

	
template<>
void Receiver::setUpdated( TypeFn<TouchEvent> callback )
{
	auto address = getOscAddressFromType<TouchEvent>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto typeHandler = dynamic_cast<detail::TypeHandler<TouchEvent, Cursor2D>*>(found->second.get());
		typeHandler->setAddHandler( callback );
	}
	else {
		// TODO: Add Receiver to osc here
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::TypeHandler<TouchEvent, Cursor2D>>( new detail::TypeHandler<TouchEvent, Cursor2D>() ) );
		auto created = dynamic_cast<detail::TypeHandler<TouchEvent, Cursor2D>*>(inserted.first->second.get());
		created->setUpdateHandler( callback );
	}
}

template<>
void Receiver::setRemoved( TypeFn<TouchEvent> callback )
{
	auto address = getOscAddressFromType<TouchEvent>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto typeHandler = dynamic_cast<detail::TypeHandler<TouchEvent, Cursor2D>*>(found->second.get());
		typeHandler->setAddHandler( callback );
	}
	else {
		// TODO: Add Receiver to osc here
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::TypeHandler<TouchEvent, Cursor2D>>( new detail::TypeHandler<TouchEvent, Cursor2D>() ) );
		auto created = dynamic_cast<detail::TypeHandler<TouchEvent, Cursor2D>*>(inserted.first->second.get());
		created->setRemoveHandler( callback );
	}
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Detail
	
namespace detail {
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// TypeHandler templated
	

	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// TypeHandler specialized (callback - TouchEvent, type - Cursor2D)
	
void TypeHandler<TouchEvent, Cursor2D>::setAddHandler( TypeFn<TouchEvent> callback )
{
	std::lock_guard<std::mutex> lock( mAddMutex );
	mAddCallback = callback;
}
	
void TypeHandler<TouchEvent, Cursor2D>::setUpdateHandler( TypeFn<TouchEvent> callback )
{
	std::lock_guard<std::mutex> lock( mUpdateMutex );
	mUpdateCallback = callback;
}

void TypeHandler<TouchEvent, Cursor2D>::setRemoveHandler( TypeFn<TouchEvent> callback )
{
	std::lock_guard<std::mutex> lock( mRemoveMutex );
	mRemoveCallback = callback;
}
	
void TypeHandler<TouchEvent, Cursor2D>::handleMessage( const osc::Message &message )
{
	auto messageType = message[0].string();
	
	if( messageType == "source" ) {
		mCurrentSource = message[1].string();
	}
	else if( messageType == "set" ) {
		auto sessionId = message[1].int32();
		auto it = find_if( begin( mSetOfCurrentTouches ), end( mSetOfCurrentTouches ),
		[sessionId]( const Cursor2D &type){
			return sessionId == type.getSessionId();
		});
		if( it == mSetOfCurrentTouches.end() ) {
			Cursor2D type( message );
			// TODO: this lambda is wrong.
			auto insert = lower_bound( begin( mSetOfCurrentTouches ), end( mSetOfCurrentTouches ), type.getSessionId(),
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
		[&aliveIds]( const Cursor2D &type ) {
			return binary_search( begin(aliveIds), end(aliveIds), type.getSessionId() );
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
						[added]( const Cursor2D &type ){
							 return added == type.getSessionId();
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
						[updated]( const Cursor2D &type ){
							 return updated == type.getSessionId();
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

template void Receiver::setAdded( TypeFn<tuio::detail::Cursor<ci::vec2>> );
template void Receiver::setRemoved( TypeFn<tuio::detail::Cursor<ci::vec2>> );
template void Receiver::setUpdated( TypeFn<tuio::detail::Cursor<ci::vec2>> );
template void Receiver::remove<tuio::detail::Cursor<ci::vec2>>();
	
template void Receiver::setAdded( TypeFn<tuio::detail::Cursor<ci::vec3>> );
template void Receiver::setRemoved( TypeFn<tuio::detail::Cursor<ci::vec3>> );
template void Receiver::setUpdated( TypeFn<tuio::detail::Cursor<ci::vec3>> );
template void Receiver::remove<tuio::detail::Cursor<ci::vec3>>();
	
template void Receiver::setAdded( TypeFn<tuio::Object2D> );
template void Receiver::setRemoved( TypeFn<tuio::Object2D> );
template void Receiver::setUpdated( TypeFn<tuio::Object2D> );
template void Receiver::remove<tuio::Object2D>();
	
template void Receiver::setAdded( TypeFn<tuio::Object25D> );
template void Receiver::setRemoved( TypeFn<tuio::Object25D> );
template void Receiver::setUpdated( TypeFn<tuio::Object25D> );
template void Receiver::remove<tuio::Object25D>();
	
template void Receiver::setAdded( TypeFn<tuio::Object3D> );
template void Receiver::setRemoved( TypeFn<tuio::Object3D> );
template void Receiver::setUpdated( TypeFn<tuio::Object3D> );
template void Receiver::remove<tuio::Object3D>();
	
template void Receiver::setAdded( TypeFn<tuio::Blob2D> );
template void Receiver::setRemoved( TypeFn<tuio::Blob2D> );
template void Receiver::setUpdated( TypeFn<tuio::Blob2D> );
template void Receiver::remove<tuio::Blob2D>();
	
template void Receiver::setAdded( TypeFn<tuio::Blob25D> );
template void Receiver::setRemoved( TypeFn<tuio::Blob25D> );
template void Receiver::setUpdated( TypeFn<tuio::Blob25D> );
template void Receiver::remove<tuio::Blob25D>();
	
template void Receiver::setAdded( TypeFn<tuio::Blob3D> );
template void Receiver::setRemoved( TypeFn<tuio::Blob3D> );
template void Receiver::setUpdated( TypeFn<tuio::Blob3D> );
template void Receiver::remove<tuio::Blob3D>();
	
template void Receiver::setAdded( TypeFn<ci::app::TouchEvent> );
template void Receiver::setRemoved( TypeFn<ci::app::TouchEvent> );
template void Receiver::setUpdated( TypeFn<ci::app::TouchEvent> );
template void Receiver::remove<ci::app::TouchEvent>();

}}  // namespace tuio // namespace cinder