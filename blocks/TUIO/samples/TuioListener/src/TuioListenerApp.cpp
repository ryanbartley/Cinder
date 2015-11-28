/*
 Copyright (c) 2010, Hector Sanchez-Pajares
 Aer Studio http://www.aerstudio.com
 All rights reserved.
 
 
 This is a block for TUIO Integration for the Cinder framework (http://libcinder.org)
 
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

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#include "Tuio.h"
#include "cinder/Log.h"

class TuioClientApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
//	void draw2d( tuio::Cursor2D cursor, int sourcenum );
//	void draw2d( tuio::Cursor2D cursor );
//	void draw25d( tuio::Cursor25D cursor );
	void draw() override;
	
	void added( const tuio::Cursor2D &cursor );
	void updated( const tuio::Cursor2D &cursor );
	void removed( const tuio::Cursor2D &cursor );
	
	void touchesBegan( TouchEvent event ) override;
	void touchesMoved( TouchEvent event ) override;
	void touchesEnded( TouchEvent event ) override;
	
	std::shared_ptr<tuio::Receiver> tuio;
};

void TuioClientApp::setup()
{
	tuio = std::shared_ptr<tuio::Receiver>( new tuio::Receiver() );
	auto receiver = tuio->getOscReceiver();
	receiver->bind();
	receiver->listen();
	
	tuio->setAddedFn<tuio::Cursor2D>( std::bind( &TuioClientApp::added, this, std::placeholders::_1 ) );
	tuio->setUpdatedFn<tuio::Cursor2D>( std::bind( &TuioClientApp::updated, this, std::placeholders::_1 ) );
	tuio->setRemovedFn<tuio::Cursor2D>( std::bind( &TuioClientApp::removed, this, std::placeholders::_1 ) );
}

void TuioClientApp::touchesBegan( cinder::app::TouchEvent event )
{
	cout << __FUNCTION__ << " size: " << event.getTouches().size() << endl;
}

void TuioClientApp::touchesMoved( cinder::app::TouchEvent event )
{
	cout << __FUNCTION__ << " size: " << event.getTouches().size() << endl;
}

void TuioClientApp::touchesEnded( cinder::app::TouchEvent event )
{
	cout << __FUNCTION__ << " size: " << event.getTouches().size() << endl;
}

void TuioClientApp::added( const tuio::Cursor2D &cursor )
{
	cout << __FUNCTION__ << " " << cursor.getSessionId() << endl;
}

void TuioClientApp::updated( const tuio::Cursor2D &cursor )
{
	cout << __FUNCTION__ << " size: " << cursor.getSessionId() << endl;
}

void TuioClientApp::removed( const tuio::Cursor2D &cursor )
{
	cout << __FUNCTION__ << " size: " << cursor.getSessionId() << endl;
}

void TuioClientApp::mouseDown( cinder::app::MouseEvent event )
{
	static bool window = true;
	if( window ) {
		tuio->setWindowReceiver( getWindow() );
		window = false;
	}
	else {
		tuio->setAddedFn<tuio::Cursor2D>( std::bind( &TuioClientApp::added, this, std::placeholders::_1 ) );
		tuio->setUpdatedFn<tuio::Cursor2D>( std::bind( &TuioClientApp::updated, this, std::placeholders::_1 ) );
		tuio->setRemovedFn<tuio::Cursor2D>( std::bind( &TuioClientApp::removed, this, std::placeholders::_1 ) );
		window = true;
	}
}

//void TuioClientApp::draw2d( tuio::Cursor2D cursor, int sourcenum )
//{	
//	switch( sourcenum % 6 ) {
//		case 0: gl::color(ColorA(1.0f, 0.0f, 0.0f, 0.6f)); break;
//		case 1: gl::color(ColorA(0.0f, 1.0f, 0.0f, 0.6f)); break;
//		case 2: gl::color(ColorA(0.0f, 0.0f, 1.0f, 0.6f)); break;
//		case 3: gl::color(ColorA(1.0f, 1.0f, 0.0f, 0.6f)); break;
//		case 4: gl::color(ColorA(0.0f, 1.0f, 1.0f, 0.6f)); break;
//		case 5: gl::color(ColorA(1.0f, 0.0f, 1.0f, 0.6f)); break;
//	}
//	gl::drawSolidCircle( cursor.getPosition() * vec2(getWindowSize()), 30 );
//}

//void TuioClientApp::draw2d( tuio::Cursor2D cursor )
//{
//	gl::color( Color::white() );
//	gl::drawSolidCircle( cursor.getPosition() * vec2(getWindowSize()), 5.0f );
//}
//
//void TuioClientApp::draw25d( tuio::Cursor25D cursor )
//{
//	gl::color(ColorA(0.0f, 1.0f, 0.0f, 1.0f));
//	float radius = 75.0f * cursor.getPosition().z;
//	gl::drawSolidCircle( vec2(cursor.getPosition()) * vec2(getWindowSize()), radius );
//}

void TuioClientApp::draw()
{
		gl::clear( Color( 0, 0, 0 ) );
//			
//		// Draw a center dot in all the cursors, to test the ability
//		// of tuio.getCursors() to get all cursors in one vector.
//		auto cursors = tuio.getCursors();
//		for( auto cursor : cursors )
//			draw2b( cursor );
//
//		// Draw each source's cursors in a different color, to test the ability
//		// of tuio.getCursors() to get each source's cursors independently.
//		set<string> sources = tuio.getSources();
//		int sourcenum = 0;
//		for( auto source = sources.begin(); source != sources.end(); ++source,++sourcenum ) {
//			vector<tuio::Cursor> cursors = tuio.getCursors(*source);
//			for( auto cursor = cursors.begin(); cursor != cursors.end(); ++cursor ) {
//				draw2( *cursor, sourcenum );
//			}
//		}
//
//		// If there any 25d cursors, draw them with radius proportional to the z value
//		vector<tuio::Cursor25d> cursors25d = tuio.getCursors25d();
//		for( auto cursor25d = cursors25d.begin(); cursor25d != cursors25d.end(); ++cursor25d ) {
//			draw25d( *cursor25d );
//		}
//	}
//	else
//		gl::clear( Color( 0.4f, 0, 0 ) );
}

CINDER_APP( TuioClientApp, RendererGl )