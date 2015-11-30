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
: Type( std::move( other ) ), mPosition( std::move( other.mPosition ) ),
	mVelocity( std::move( other.mVelocity ) ), mAcceleration( other.mAcceleration )
{
}

template<typename VEC_T>
Cursor<VEC_T>& Cursor<VEC_T>::operator=( Cursor<VEC_T> &&other ) NOEXCEPT
{
	if( this != &other ) {
		Type::operator=( std::move( other ) );
		mPosition = std::move( other.mPosition );
		mVelocity = std::move( other.mVelocity );
		mAcceleration = other.mAcceleration;
	}
	return *this;
}
	
template<typename T>
app::TouchEvent::Touch Cursor<T>::convertToTouch( const ci::app::WindowRef &window ) const
{
	auto windowSize = vec2(window->getSize());
	app::TouchEvent::Touch ret( vec2(getPosition()) * windowSize,
								vec2(getPosition() - getVelocity()) * windowSize,
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
: Type( std::move( other ) ), mClassId( other.mClassId ), mPosition( std::move( other.mPosition ) ),
	mVelocity( std::move( other.mVelocity ) ), mAngle( std::move( other.mAngle ) ),
	mRotationVelocity( std::move( other.mRotationVelocity ) ), mAcceleration( other.mAcceleration ),
	mRotateAccel( other.mRotateAccel )
{
}

template<typename VEC_T, typename ROT_T>
Object<VEC_T, ROT_T>& Object<VEC_T, ROT_T>::operator=( Object<VEC_T, ROT_T> &&other ) NOEXCEPT
{
	if( this != &other ) {
		Type::operator=( std::move( other ) );
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
: Type( std::move( other ) ), mPosition( std::move( other.mPosition ) ),
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
		Type::operator=( std::move( other ) );
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
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Blob3D

template<typename TuioType>
class TypeHandler : public TypeHandlerBase {
public:
	//! TODO: Need to figure out about "PastFrameThreshold", got rid of it.
	TypeHandler() {}
	virtual ~TypeHandler() {}
	
	std::vector<TuioType>	getCurrentActive();
	void					handleMessage( const osc::Message &message ) override;
	virtual void			handleFseq( int32_t frame ) = 0;
	
	std::string						mCurrentSource;
	std::vector<int32_t>			mAdded, mUpdated;
	std::vector<TuioType>			mSetOfCurrentTouches,
									mRemovedTouches;
	std::map<std::string, int32_t>	mSourceFrameNums;
	// containers for changes which will be propagated upon receipt of 'fseq'
};
	
template<typename TuioType>
class SeperatedTypeHandler : public TypeHandler<TuioType> {
public:
	SeperatedTypeHandler() : TypeHandler<TuioType>() {}
	~SeperatedTypeHandler() {}
	
	void setAddHandler( TypeFn<TuioType> callback );
	void setUpdateHandler( TypeFn<TuioType> callback );
	void setRemoveHandler( TypeFn<TuioType> callback );
	
	void handleFseq( int32_t frame ) override;
	
protected:
	TypeFn<TuioType>	mAddCallback, mUpdateCallback, mRemoveCallback;
	std::mutex			mAddMutex, mUpdateMutex, mRemoveMutex, mFseqMutex;
};
	
class WindowCursorHandler : public TypeHandler<Cursor2D> {
public:
	using FseqFn = std::function<void( const std::vector<Cursor2D> &/*added*/,
									   const std::vector<Cursor2D> &/*updated*/,
									   const std::vector<Cursor2D> &/*removed*/)>;
	WindowCursorHandler() : TypeHandler() {}
	~WindowCursorHandler() {}
	
	void setFseqFn( FseqFn fseqFn );
	void handleFseq( int32_t frame ) override;
	
protected:
	std::mutex	mFseqMutex;
	FseqFn		mFseqFn;
};
	
} // namespace detail
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Receiver
	
Receiver::Receiver( uint16_t localPort, const asio::ip::udp &protocol, asio::io_service &io )
: mReceiver( new osc::ReceiverUdp( localPort, protocol, io ) )
{
	
}
	
Receiver::Receiver( const app::WindowRef &window, uint16_t localPort, const asio::ip::udp &protocol )
: mReceiver( new osc::ReceiverUdp( localPort, protocol, app::App::get()->io_service() ) )
{
	setupWindowReceiver( window );
}
	
Receiver::Receiver( const app::WindowRef &window, osc::ReceiverBase *ptr )
: mReceiver( ptr )
{
	setupWindowReceiver( window );
}
	
Receiver::Receiver( osc::ReceiverBase *ptr )
: mReceiver(  ptr )
{
	
}
	
template<typename TuioType>
void Receiver::setAddedFn( TypeFn<TuioType> callback )
{
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto typeHandler = dynamic_cast<detail::SeperatedTypeHandler<TuioType>*>(found->second.get());
		typeHandler->setAddHandler( callback );
	}
	else {
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::SeperatedTypeHandler<TuioType>>( new detail::SeperatedTypeHandler<TuioType>() ) );
		auto created = dynamic_cast<detail::SeperatedTypeHandler<TuioType>*>(inserted.first->second.get());
		created->setAddHandler( callback );
		mReceiver->setListener( address, std::bind( &detail::TypeHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}


template<typename TuioType>
void Receiver::setUpdatedFn( TypeFn<TuioType> callback )
{
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto typeHandler = dynamic_cast<detail::SeperatedTypeHandler<TuioType>*>(found->second.get());
		typeHandler->setUpdateHandler( callback );
	}
	else {
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::SeperatedTypeHandler<TuioType>>( new detail::SeperatedTypeHandler<TuioType>() ) );
		auto created = dynamic_cast<detail::SeperatedTypeHandler<TuioType>*>(inserted.first->second.get());
		created->setUpdateHandler( callback );
		mReceiver->setListener( address, std::bind( &detail::TypeHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}

template<typename TuioType>
void Receiver::setRemovedFn( TypeFn<TuioType> callback )
{
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		auto typeHandler = dynamic_cast<detail::SeperatedTypeHandler<TuioType>*>(found->second.get());
		typeHandler->setRemoveHandler( callback );
	}
	else {
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::SeperatedTypeHandler<TuioType>>( new detail::SeperatedTypeHandler<TuioType>() ) );
		auto created = dynamic_cast<detail::SeperatedTypeHandler<TuioType>*>(inserted.first->second.get());
		created->setRemoveHandler( callback );
		mReceiver->setListener( address, std::bind( &detail::TypeHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}
	
template<>
void Receiver::setAddedFn( TypeFn<Cursor2D> callback )
{
	auto address = getOscAddressFromType<Cursor2D>();
	auto found = mHandlers.find( address );
	bool create = true;
	if( found != mHandlers.end() ) {
		auto typeHandler = dynamic_cast<detail::SeperatedTypeHandler<Cursor2D>*>(found->second.get());
		// could be WindowCursorHandler
		if( typeHandler ) {
			typeHandler->setAddHandler( callback );
			create = false;
		}
		else {
			// if so first remove listener
			mReceiver->removeListener( address );
			// then destroy and make a new.
			mHandlers.erase( found );
		}
	}
	
	if( create ) {
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::SeperatedTypeHandler<Cursor2D>>( new detail::SeperatedTypeHandler<Cursor2D>() ) );
		auto created = dynamic_cast<detail::SeperatedTypeHandler<Cursor2D>*>(inserted.first->second.get());
		created->setAddHandler( callback );
		mReceiver->setListener( address, std::bind( &detail::TypeHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}
template<>
void Receiver::setUpdatedFn( TypeFn<Cursor2D> callback )
{
	auto address = getOscAddressFromType<Cursor2D>();
	auto found = mHandlers.find( address );
	bool create = true;
	if( found != mHandlers.end() ) {
		auto typeHandler = dynamic_cast<detail::SeperatedTypeHandler<Cursor2D>*>(found->second.get());
		// could be WindowCursorHandler
		if( typeHandler ) {
			typeHandler->setUpdateHandler( callback );
			create = false;
		}
		else {
			// if so first remove listener
			mReceiver->removeListener( address );
			// then destroy and make a new.
			mHandlers.erase( found );
		}
	}
	
	if( create ) {
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::SeperatedTypeHandler<Cursor2D>>( new detail::SeperatedTypeHandler<Cursor2D>() ) );
		auto created = dynamic_cast<detail::SeperatedTypeHandler<Cursor2D>*>(inserted.first->second.get());
		created->setUpdateHandler( callback );
		mReceiver->setListener( address, std::bind( &detail::TypeHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}
template<>
void Receiver::setRemovedFn( TypeFn<Cursor2D> callback )
{
	auto address = getOscAddressFromType<Cursor2D>();
	auto found = mHandlers.find( address );
	bool create = true;
	if( found != mHandlers.end() ) {
		auto typeHandler = dynamic_cast<detail::SeperatedTypeHandler<Cursor2D>*>(found->second.get());
		// could be WindowCursorHandler
		if( typeHandler ) {
			typeHandler->setRemoveHandler( callback );
			create = false;
		}
		else {
			// if so first remove listener
			mReceiver->removeListener( address );
			// then destroy and make a new.
			mHandlers.erase( found );
		}
	}
	
	if( create ) {
		auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::SeperatedTypeHandler<Cursor2D>>( new detail::SeperatedTypeHandler<Cursor2D>() ) );
		auto created = dynamic_cast<detail::SeperatedTypeHandler<Cursor2D>*>(inserted.first->second.get());
		created->setRemoveHandler( callback );
		mReceiver->setListener( address, std::bind( &detail::TypeHandlerBase::handleMessage,
												   created, std::placeholders::_1 ) );
	}
}

template<typename TuioType>
void Receiver::clear()
{
	auto address = getOscAddressFromType<TuioType>();
	auto found = mHandlers.find( address );
	if( found != mHandlers.end() ) {
		mReceiver->removeListener( address );
		mHandlers.erase( found );
	}
}
	
void Receiver::setupWindowReceiver( ci::app::WindowRef window )
{
	auto address = getOscAddressFromType<Cursor2D>();
	auto inserted = mHandlers.emplace( address, std::unique_ptr<detail::WindowCursorHandler>( new detail::WindowCursorHandler() ) );
	auto handler = dynamic_cast<detail::WindowCursorHandler*>(inserted.first->second.get());
	mReceiver->setListener( address, std::bind( &detail::TypeHandlerBase::handleMessage,
												   handler, std::placeholders::_1 ) );
	
	// Take a weak pointer, so that we don't try to access it after it's been destroyed.
	auto weakWindow = std::weak_ptr<app::Window>( window );
	// Set the Fseq lambda
	handler->setFseqFn(
	[weakWindow]( const std::vector<Cursor2D> &added,
					const std::vector<Cursor2D> &updated,
					const std::vector<Cursor2D> &removed ) {
		auto shared = weakWindow.lock();
		if( shared ) {
			std::array<const std::vector<Cursor2D>*, 3> cursors = { &added, &updated, &removed };
			auto transformFn = [shared]( const Cursor2D &cursor ) { return cursor.convertToTouch( shared ); };

			for( int i = 0; i < 3; i++ ) {
				if( ! cursors[i]->empty() ) {
					std::vector<TouchEvent::Touch> touches( cursors[i]->size() );
					std::transform( begin( *cursors[i] ), end( *cursors[i] ), touches.begin(), transformFn );
					ci::app::TouchEvent event( shared, touches );
					switch ( i ) {
						case 0: shared->emitTouchesBegan( &event ); break;
						case 1: shared->emitTouchesMoved( &event ); break;
						case 2: shared->emitTouchesEnded( &event ); break;
					}
				}
			}
		}
	});
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
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// Detail
	
namespace detail {
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///// TypeHandler templated
	
template<typename TuioType>
void SeperatedTypeHandler<TuioType>::setAddHandler( TypeFn<TuioType> callback )
{
	std::lock_guard<std::mutex> lock( mAddMutex );
	mAddCallback = callback;
}

template<typename TuioType>
void SeperatedTypeHandler<TuioType>::setUpdateHandler( TypeFn<TuioType> callback )
{
	std::lock_guard<std::mutex> lock( mUpdateMutex );
	mUpdateCallback = callback;
}

template<typename TuioType>
void SeperatedTypeHandler<TuioType>::setRemoveHandler( TypeFn<TuioType> callback )
{
	std::lock_guard<std::mutex> lock( mRemoveMutex );
	mRemoveCallback = callback;
}
	
void WindowCursorHandler::setFseqFn( FseqFn fseqFn )
{
	std::lock_guard<std::mutex> lock( mFseqMutex );
	mFseqFn = fseqFn;
}

template<typename TuioType>
void TypeHandler<TuioType>::handleMessage( const osc::Message &message )
{
	auto messageType = message[0].string();
	
	using namespace std;
	if( messageType == "source" ) {
		mCurrentSource = message[1].string();
	}
	else if( messageType == "set" ) {
		auto sessionId = message[1].int32();
		auto it = find_if( begin( mSetOfCurrentTouches ), end( mSetOfCurrentTouches ),
		[sessionId]( const TuioType &type ){
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
		handleFseq( message[1].int32() );
	}
}
	
template<typename TuioType>
void SeperatedTypeHandler<TuioType>::handleFseq( int32_t frame )
{
	int32_t prev_frame = this->mSourceFrameNums[this->mCurrentSource];
	int32_t delta_frame = frame - prev_frame;
	// TODO: figure out about past frame threshold updating, this was also in the if condition ( dframe < -mPastFrameThreshold )
	if( frame == -1 || delta_frame > 0 ) {
		auto begin = this->mSetOfCurrentTouches.cbegin();
		auto end = this->mSetOfCurrentTouches.cend();
		if( ! this->mAdded.empty() ) {
			std::lock_guard<std::mutex> lock( mAddMutex );
			if( mAddCallback ) {
				for( auto & added : this->mAdded ) {
					auto found = find_if( begin, end,
					[added]( const TuioType &type ) {
						 return added == type.getSessionId();
					});
					if( found != end )
						mAddCallback( *found );
				}
			}
			this->mAdded.clear();
		}
		if ( ! this->mUpdated.empty() ) {
			std::lock_guard<std::mutex> lock( mUpdateMutex );
			if( mUpdateCallback ) {
				for( auto & updated : this->mUpdated ) {
					auto found = find_if( begin, end,
					[updated]( const TuioType &type ) {
						 return updated == type.getSessionId();
					});
					if( found != end )
						mUpdateCallback( *found );
				}
			}
			this->mUpdated.clear();
		}
		if( ! this->mRemovedTouches.empty() ){
			std::lock_guard<std::mutex> lock( mRemoveMutex );
			if( mRemoveCallback )
				for( auto & removed : this->mRemovedTouches ) {
					mRemoveCallback( removed );
				}
			this->mRemovedTouches.clear();
		}
		this->mSourceFrameNums[this->mCurrentSource] = ( frame == -1 ) ? this->mSourceFrameNums[this->mCurrentSource] : frame;
	}
}
	
void WindowCursorHandler::handleFseq( int32_t frame )
{
	int32_t prev_frame = this->mSourceFrameNums[this->mCurrentSource];
	int32_t delta_frame = frame - prev_frame;
	// TODO: figure out about past frame threshold updating, this was also in the if condition ( dframe < -mPastFrameThreshold )
	if( frame == -1 || delta_frame > 0 ) {
		auto begin = this->mSetOfCurrentTouches.cbegin();
		auto end = this->mSetOfCurrentTouches.cend();
		std::vector<Cursor2D> addedCursors, updatedCursors;
		if( ! this->mAdded.empty() ) {
			for( auto & added : this->mAdded ) {
				auto found = find_if( begin, end,
				[added]( const Cursor2D &type ) {
					 return added == type.getSessionId();
				});
				if( found != end )
					addedCursors.push_back( *found );
			}
			this->mAdded.clear();
		}
		if ( ! this->mUpdated.empty() ) {
			for( auto & updated : this->mUpdated ) {
				auto found = find_if( begin, end,
				[updated]( const Cursor2D &type ) {
					 return updated == type.getSessionId();
				});
				if( found != end )
					updatedCursors.push_back( *found );
			}
			this->mUpdated.clear();
		}
		this->mSourceFrameNums[this->mCurrentSource] = ( frame == -1 ) ? this->mSourceFrameNums[this->mCurrentSource] : frame;
		mFseqFn( addedCursors, updatedCursors, this->mRemovedTouches );
		this->mRemovedTouches.clear();
	}
}

} // namespace detail


template void Receiver::clear<tuio::detail::Cursor<ci::vec2>>();
	
template void Receiver::setAddedFn( TypeFn<tuio::detail::Cursor<ci::vec3>> );
template void Receiver::setUpdatedFn( TypeFn<tuio::detail::Cursor<ci::vec3>> );
template void Receiver::setRemovedFn( TypeFn<tuio::detail::Cursor<ci::vec3>> );
template void Receiver::clear<tuio::detail::Cursor<ci::vec3>>();
	
template void Receiver::setAddedFn( TypeFn<tuio::Object2D> );
template void Receiver::setUpdatedFn( TypeFn<tuio::Object2D> );
template void Receiver::setRemovedFn( TypeFn<tuio::Object2D> );
template void Receiver::clear<tuio::Object2D>();
	
template void Receiver::setAddedFn( TypeFn<tuio::Object25D> );
template void Receiver::setUpdatedFn( TypeFn<tuio::Object25D> );
template void Receiver::setRemovedFn( TypeFn<tuio::Object25D> );
template void Receiver::clear<tuio::Object25D>();
	
template void Receiver::setAddedFn( TypeFn<tuio::Object3D> );
template void Receiver::setUpdatedFn( TypeFn<tuio::Object3D> );
template void Receiver::setRemovedFn( TypeFn<tuio::Object3D> );
template void Receiver::clear<tuio::Object3D>();
	
template void Receiver::setAddedFn( TypeFn<tuio::Blob2D> );
template void Receiver::setUpdatedFn( TypeFn<tuio::Blob2D> );
template void Receiver::setRemovedFn( TypeFn<tuio::Blob2D> );
template void Receiver::clear<tuio::Blob2D>();
	
template void Receiver::setAddedFn( TypeFn<tuio::Blob25D> );
template void Receiver::setUpdatedFn( TypeFn<tuio::Blob25D> );
template void Receiver::setRemovedFn( TypeFn<tuio::Blob25D> );
template void Receiver::clear<tuio::Blob25D>();
	
template void Receiver::setAddedFn( TypeFn<tuio::Blob3D> );
template void Receiver::setUpdatedFn( TypeFn<tuio::Blob3D> );
template void Receiver::setRemovedFn( TypeFn<tuio::Blob3D> );
template void Receiver::clear<tuio::Blob3D>();

}}  // namespace tuio // namespace cinder