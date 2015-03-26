#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/TransformFeedbackObj.h"
#include "cinder/gl/Texture.h"

#include "CellTriangulationTable.h"
#include "cinder/MayaCamUI.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TransformFeedbackMetaballsApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
	void update() override;
	void draw() override;
	
	void setupBuffers();
	void setupTextures();
	void setupGlsl();
	
	gl::GlslProgRef		mSphereUpdate, mScalarFieldUpdate, mMarchingCubeCells, mMarchingCubesRender;
	gl::VboRef			mSphereVbo, mScalarFieldVbo, mMarchingCubesCellsVbo;
	gl::TransformFeedbackObjRef mSphereTfo, mScalarFieldTfo, mMarchingCubesCellsTfo;
	gl::VaoRef			mDummyVao;
	gl::Texture3dRef	mScalarFieldTex, mMarchingCubesCellsTex;
	gl::Texture2dRef	mMarchingCubesLookupTex;
	CameraPersp			mCamera;
	MayaCamUI			mMayaCam;
};

void TransformFeedbackMetaballsApp::setup()
{
	setupTextures();
	setupBuffers();
	setupGlsl();
	
	/* Specify one byte alignment for pixels rows in memory for pack and unpack buffers. */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT,   1);
	
	mCamera.setPerspective( 45.0f, getWindowAspectRatio(), 0.01f, 100.0f );
	mCamera.lookAt( vec3( 0, 0, 5 ), vec3( 0 ) );
	mMayaCam.setCurrentCam( mCamera );
	
	/* Enable facet culling, depth testing and specify front face for polygons. */
	gl::enable   (GL_DEPTH_TEST);
	gl::enable   (GL_CULL_FACE );
	glFrontFace  (GL_CW        );
}

void TransformFeedbackMetaballsApp::setupGlsl()
{
	/* 1. Calculate sphere positions stage. */
	/* Create sphere updater program object. */
	mSphereUpdate = gl::GlslProg::create( gl::GlslProg::Format()
#if ! defined( CINDER_GL_ES )
										 .vertex( loadAsset( "SphereUpdate_osx.vert" ) )
										 .fragment( "#version 330 core void main(){}" )
#else
										 .vertex( loadAsset( "SphereUpdate_ios.vert" ) )
										 .fragment( "#version 300 es void main(){}" )
#endif
										 .feedbackFormat( GL_SEPARATE_ATTRIBS )
										 .feedbackVaryings( { "sphere_position" } ) );
	
	/* 2. Scalar field generation stage. */
	/* Create scalar field generator program object. */
	mScalarFieldUpdate = gl::GlslProg::create( gl::GlslProg::Format()
#if ! defined( CINDER_GL_ES )
											  .vertex( loadAsset( "ScalarFieldUpdate_osx.vert" ) )
											  .fragment( "#version 330 core void main(){}" )
#else
											  .vertex( loadAsset( "ScalarFieldUpdate_ios.vert" ) )
											  .fragment( "#version 300 es void main(){}" )
#endif
											  .feedbackFormat( GL_SEPARATE_ATTRIBS )
											  .feedbackVaryings( { "scalar_field_value" } ) );
	
	mScalarFieldUpdate->uniform( "samples_per_axis", (int)samples_per_axis );
	mScalarFieldUpdate->uniformBlock( "spheres_uniform_block", 0 );
	
	/* 3. Marching Cubes cell-splitting stage. */
	/* Create a program object to execute Marching Cubes algorithm cell splitting stage. */
	mMarchingCubeCells = gl::GlslProg::create( gl::GlslProg::Format()
#if ! defined( CINDER_GL_ES )
											  .vertex( loadAsset( "MarchingCubesCells_osx.vert" ) )
											  .fragment( "#version 330 core void main(){}" )
#else
											  .vertex( loadAsset( "MarchingCubesCells_ios.vert" ) )
											  .fragment( "#version 300 es void main(){}" )
#endif
											  .feedbackFormat( GL_SEPARATE_ATTRIBS )
											  .feedbackVaryings( { "cell_type_index" } ) );
	
	mMarchingCubeCells->uniform( "cells_per_axis", (int)cells_per_axis );
	mMarchingCubeCells->uniform( "iso_level", isosurface_level );
	mMarchingCubeCells->uniform( "scalar_field", 1 );
	
	/* 4. Marching Cubes algorithm triangle generation and rendering stage. */
	/* Create a program object that we will use for triangle generation and rendering stage. */
	mMarchingCubesRender = gl::GlslProg::create( gl::GlslProg::Format()
#if ! defined( CINDER_GL_ES )
												.vertex( loadAsset( "MarchingCubesRender_osx.vert" ) )
												.fragment( loadAsset( "MarchingCubesRender_osx.frag" ) )
#else
												.vertex( loadAsset( "MarchingCubesRender_ios.vert" ) )
												.fragment( loadAsset( "MarchingCubesRender_ios.frag" ) )
#endif
												.uniform( gl::UNIFORM_MODEL_VIEW_PROJECTION, "mvp" ) );
	
	
	mMarchingCubesRender->uniform( "iso_level", isosurface_level );
	mMarchingCubesRender->uniform( "samples_per_axis", (int)samples_per_axis );
	mMarchingCubesRender->uniform( "tri_table", 4 );
	mMarchingCubesRender->uniform( "cell_types", 2 );
	mMarchingCubesRender->uniform( "scalar_field", 1 );
}

void TransformFeedbackMetaballsApp::setupBuffers()
{
	/* [Stage 1 Allocate buffer for output values] */
	mSphereVbo = gl::Vbo::create( GL_TRANSFORM_FEEDBACK_BUFFER, n_spheres * n_sphere_position_components * sizeof(GLfloat), NULL, GL_STATIC_DRAW );
	
	/* [Stage 1 Transform feedback object initialization] */
	/* Generate and bind transform feedback object. */
	mSphereTfo = gl::TransformFeedbackObj::create();
	mSphereTfo->bind();
	 /* Bind buffers to store calculated sphere positions. */
	gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, mSphereVbo );
	mSphereTfo->unbind();
	
//	/* Allocate memory for buffer */
//	glBindBuffer( GL_UNIFORM_BUFFER, mSphereVbo->getId() );
	
	gl::bindBufferBase( GL_UNIFORM_BUFFER, 0, mSphereVbo );
	
	/* Generate buffer object id. Define required storage space sufficient to hold scalar field data. */
	mScalarFieldVbo = gl::Vbo::create( GL_TRANSFORM_FEEDBACK_BUFFER, samples_in_3d_space * sizeof(GLfloat), NULL, GL_STATIC_DRAW );
	
	mScalarFieldTfo = gl::TransformFeedbackObj::create();
	mScalarFieldTfo->bind();
	/* Bind buffer to store calculated scalar field values. */
	gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, mScalarFieldVbo );
	mScalarFieldTfo->unbind();
	
	mMarchingCubesCellsVbo = gl::Vbo::create( GL_TRANSFORM_FEEDBACK_BUFFER, cells_in_3d_space * sizeof(GLint), NULL, GL_STATIC_DRAW );
	mMarchingCubesCellsTfo = gl::TransformFeedbackObj::create();
	mMarchingCubesCellsTfo->bind();
	/* Bind buffer to store calculated cell type data. */
	gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, mMarchingCubesCellsVbo );
	mMarchingCubesCellsTfo->unbind();
	
	
	/* Generate a vertex array object. We'll go with the explanation later. */
	mDummyVao = gl::Vao::create();
	
	/* In OpenGL ES, draw calls require a bound vertex array object.
	 * Even though we're not using any per-vertex attribute data, we still need to bind a vertex array object.
	 */
	mDummyVao->bind();
}

void TransformFeedbackMetaballsApp::setupTextures()
{
	/* [Stage 3 Creating texture] */
	
	gl::Texture3d::Format format3d;
	format3d.minFilter( GL_NEAREST )
		.magFilter( GL_NEAREST )
		.wrapS( GL_CLAMP_TO_EDGE )
		.wrapT( GL_CLAMP_TO_EDGE )
		.internalFormat( GL_R32F )
		.immutableStorage()
		.mipmap( false )
		.setBaseMipmapLevel( 0 );
	format3d.setMaxMipmapLevel( 0 );
	
	mScalarFieldTex = gl::Texture3d::create( samples_per_axis, samples_per_axis, samples_per_axis, format3d );
	/* Scalar field uses GL_TEXTURE_3D target of texture unit 1. */
	mScalarFieldTex->bind( 1 );
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,     GL_CLAMP_TO_EDGE);
	/* Generate a texture object to hold cell type data. (We will explain why the texture later). */
	/* Prepare texture storage for marching cube cell type data. */
	format3d.internalFormat( GL_R32I );
	mMarchingCubesCellsTex = gl::Texture3d::create( cells_per_axis, cells_per_axis, cells_per_axis, format3d );
	/* Marching cubes cell type data uses GL_TEXTURE_3D target of texture unit 2. */
	mMarchingCubesCellsTex->bind( 2 );
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,     GL_CLAMP_TO_EDGE);
	gl::Texture2d::Format format2d;
	format2d.minFilter( GL_NEAREST )
		.magFilter( GL_NEAREST )
		.wrap( GL_CLAMP_TO_EDGE )
		.internalFormat( GL_R32I )
		.dataType( GL_INT )
		.immutableStorage()
		.mipmap( false )
		.setBaseMipmapLevel( 0 );
	format2d.setMaxMipmapLevel( 0 );
	mMarchingCubesLookupTex = gl::Texture2d::create( mc_vertices_per_cell, mc_cells_types_count, format2d );
	{
	gl::ScopedActiveTexture scopeActive( 4 );
	/* Lookup array (tri_table) uses GL_TEXTURE_2D target of texture unit 4. */
	mMarchingCubesLookupTex->bind( 4 );
	
	/* Load lookup table (tri_table) into texture. */
	glTexSubImage2D(GL_TEXTURE_2D,        /* Use texture bound to GL_TEXTURE_2D               */
					0,                    /* Base image level                                 */
					0,                    /* From the texture origin                          */
					0,                    /* From the texture origin                          */
					mc_vertices_per_cell, /* Width will represent vertices in all 5 triangles */
					mc_cells_types_count, /* Height will represent cell type                  */
					GL_RED_INTEGER,       /* Texture will have only one component             */
					GL_INT,               /* ... of type int                                  */
					tri_table             /* Data will be copied directly from tri_table      */
					);
	}

}

void TransformFeedbackMetaballsApp::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void TransformFeedbackMetaballsApp::mouseDrag( MouseEvent event )
{
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void TransformFeedbackMetaballsApp::update()
{
}

void TransformFeedbackMetaballsApp::draw()
{
	float modalTime = getElapsedSeconds();
	gl::clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	gl::setMatrices( mMayaCam.getCamera() );
	
	/* [Stage 1 Calculate sphere positions stage] */
	/* 1. Calculate sphere positions stage.
	 *
	 * At this stage we calculate new sphere positions in space
	 * according to current time moment.
	 */
	/* [Stage 1 Bind buffers to store calculated sphere position values] */
	{
		/* Bind buffers to store calculated sphere position values. */
		mSphereTfo->bind();
		/* [Stage 1 Enable GL_RASTERIZER_DISCARD] */
		/* Shorten GL pipeline: we will use vertex shader only. */
		gl::ScopedState scopeState( GL_RASTERIZER_DISCARD, true );
		/* [Stage 1 Enable GL_RASTERIZER_DISCARD] */
		{
			/* Select program for sphere positions generation stage. */
			gl::ScopedGlslProg scopeGlsl( mSphereUpdate );
			mSphereUpdate->uniform( "time", modalTime );
			
			/* [Stage 1 Activate transform feedback mode] */
			/* Activate transform feedback mode. */
			gl::beginTransformFeedback( GL_POINTS );
			{
				/* [Stage 1 Execute n_spheres times vertex shader] */
				/* Run sphere positions calculation. */
				gl::drawArrays( GL_POINTS, 0, n_spheres );
				/* [Stage 1 Execute n_spheres times vertex shader] */
			}
			/* [Stage 1 Deactivate transform feedback mode] */
			gl::endTransformFeedback();
		}
	}
	/* [Stage 1 Calculate sphere positions stage] */
	
	
	/* [Stage 2 Scalar field generation stage] */
	/* 2. Scalar field generation stage.
	 *
	 * At this stage we calculate scalar field and store it in buffer
	 * and later copy from buffer to texture.
	 */
	{
		/* Bind sphere positions data buffer to GL_UNIFORM_BUFFER. */
		gl::ScopedBuffer scopeBuffer( GL_UNIFORM_BUFFER, mSphereVbo->getId() );
		
		/* Bind buffer object to store calculated scalar field values. */
		mScalarFieldTfo->bind();
		
		/* Shorten GL pipeline: we will use vertex shader only. */
		gl::ScopedState scopeState( GL_RASTERIZER_DISCARD, true );
		{
			/* Select program for scalar field generation stage. */
			gl::ScopedGlslProg scopeGlsl( mScalarFieldUpdate );
			
			/* Activate transform feedback mode. */
			gl::beginTransformFeedback( GL_POINTS );
			{
				/* Run scalar field calculation for all vertices in space. */
				gl::drawArrays( GL_POINTS, 0, samples_in_3d_space );
			}
			gl::endTransformFeedback();
		}
		mScalarFieldTfo->unbind();
	}
	
	/* Copy scalar field values from buffer into texture bound to target GL_TEXTURE_3D of texture unit 1.
	 * We need to move this data to a texture object, as there is no way we could access data
	 * stored within a buffer object in an OpenGL ES 3.0 shader.
	 */
	/* [Stage 2 Scalar field generation stage move data to texture] */
	{
		gl::ScopedActiveTexture scopeActiveTexture( 1 );
		gl::ScopedBuffer		scopeBuffer( GL_PIXEL_UNPACK_BUFFER, mScalarFieldVbo->getId() );
		glTexSubImage3D(GL_TEXTURE_3D,    /* Use texture bound to GL_TEXTURE_3D                                     */
							 0,                /* Base image level                                                       */
							 0,                /* From the texture origin                                                */
							 0,                /* From the texture origin                                                */
							 0,                /* From the texture origin                                                */
							 samples_per_axis, /* Texture have same width as scalar field in buffer                      */
							 samples_per_axis, /* Texture have same height as scalar field in buffer                     */
							 samples_per_axis, /* Texture have same depth as scalar field in buffer                      */
							 GL_RED,           /* Scalar field gathered in buffer has only one component                 */
							 GL_FLOAT,         /* Scalar field gathered in buffer is of float type                       */
							 NULL              /* Scalar field gathered in buffer bound to GL_PIXEL_UNPACK_BUFFER target */
							 );
	}
	/* [Stage 2 Scalar field generation stage move data to texture] */
	
	
	/* 3. Marching cube algorithm cell splitting stage.
	 *
	 * At this stage we analyze isosurface in each cell of space and
	 * assign one of 256 possible types to each cell. Cell type data
	 * for each cell is stored in attached buffer.
	 */
	{
		/* Bind buffer to store cell type data. */
		mMarchingCubesCellsTfo->bind();
		
		/* Shorten GL pipeline: we will use vertex shader only. */
		gl::ScopedState scopeState( GL_RASTERIZER_DISCARD, true );
		{
			/* Select program for Marching Cubes algorthim's cell splitting stage. */
			gl::ScopedGlslProg scopeGlslProg( mMarchingCubeCells );
			
			/* Activate transform feedback mode. */
			gl::beginTransformFeedback( GL_POINTS );
			{
				/* [Stage 3 Execute vertex shader] */
				/* Run Marching Cubes algorithm cell splitting stage for all cells. */
				gl::drawArrays( GL_POINTS, 0, cells_in_3d_space );
				/* [Stage 3 Execute vertex shader] */
			}
			gl::endTransformFeedback();
		}
		
		/* Unbind buffers used at this stage. */
		mMarchingCubesCellsTfo->unbind();
	}
	
	/* Copy data from buffer into texture bound to target GL_TEXTURE2 in texture unit 2.
	 * We need to move this data to a texture object, as there is no way we could access data
	 * stored within a buffer object in a OpenGL ES 3.0 shader.
	 */
	{
		gl::ScopedActiveTexture scopeActiveTexture( 2 );
		gl::ScopedBuffer		scopeBuffer( GL_PIXEL_UNPACK_BUFFER, mMarchingCubesCellsVbo->getId() );
		glTexSubImage3D(GL_TEXTURE_3D,  /* Use texture bound to GL_TEXTURE_3D                                   */
							 0,              /* Base image level                                                     */
							 0,              /* From the texture origin                                              */
							 0,              /* From the texture origin                                              */
							 0,              /* From the texture origin                                              */
							 cells_per_axis, /* Texture have same width as cells by width in buffer                  */
							 cells_per_axis, /* Texture have same height as cells by height in buffer                */
							 cells_per_axis, /* Texture have same depth as cells by depth in buffer                  */
							 GL_RED_INTEGER, /* Cell types gathered in buffer have only one component                */
							 GL_INT,         /* Cell types gathered in buffer are of int type                        */
							 NULL            /* Cell types gathered in buffer bound to GL_PIXEL_UNPACK_BUFFER target */
							 );
	
	}
	/* 4. Marching Cubes algorithm triangle generation stage.
	 *
	 * At this stage, we render exactly (3 vertices * 5 triangles per cell *
	 * amount of cells the scalar field is split to) triangle vertices.
	 * Then render triangularized geometry.
	 */
	{
		gl::ScopedActiveTexture scopeActiveTexture( 0 );
		
		/* Activate triangle generating and rendering program. */
		gl::ScopedGlslProg scopeGlslProg( mMarchingCubesRender );
		mMarchingCubesRender->uniform( "time", modalTime );
		gl::setDefaultShaderVars();
		
		/* [Stage 4 Run triangle generating and rendering program] */
		/* Run triangle generating and rendering program. */
		gl::drawArrays( GL_TRIANGLES, 0, cells_in_3d_space * triangles_per_cell * vertices_per_triangle );
		/* [Stage 4 Run triangle generating and rendering program] */
	}
}

CINDER_APP( TransformFeedbackMetaballsApp, RendererGl )
