/*
 Copyright (c) 2010, The Barbarian Group
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

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <set>

#include "cinder/gl/gl.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Matrix.h"
#include "cinder/DataSource.h"
#include "cinder/GeomIo.h"
#include "cinder/Exception.h"

//! Convenience macro that allows one to embed raw glsl code in-line. The \a VERSION parameter will be used for the glsl's '#version' define.
//! \note Some strings will confuse different compilers, most commonly being preprocessor derictives (hence the need for \a VERSION to be a pamaeter).
//! If available on all target platforms, users should use C+11 raw string literals, which do not suffer from the same limitations.
#define CI_GLSL(VERSION,CODE) "#version " #VERSION "\n" #CODE

namespace cinder { namespace gl {
	
typedef std::shared_ptr<class GlslProg> GlslProgRef;

class GlslProg : public std::enable_shared_from_this<GlslProg> {
  public:
	
	struct Attribute {
		std::string		mName;
		GLint			mCount = 0, mLoc = -1;
		GLenum			mType;
		geom::Attrib	mSemantic = geom::Attrib::NUM_ATTRIBS;
	};
	
	struct Uniform {
		std::string		mName;
		GLint			mCount = 0, mLoc = -1, mIndex = -1;
		GLint			mDataSize;
		GLenum			mType;
		UniformSemantic mSemantic = UniformSemantic::USER_DEFINED_UNIFORM;
	};
#if ! defined( CINDER_GL_ES_2 )
	struct UniformBlock {
		std::string mName;
		GLint mSize = 0, mLoc = -1;
		GLint mBlockBinding;
		//! Contains info on each of the active uniforms contained by this Uniform Block
		std::vector<Uniform> mActiveUniforms;
		//! Includes information attached to GL_UNIFORM_OFFSET, GL_UNIFORM_ARRAY_STRIDE, GL_UNIFORM_MATRIX_STRIDE
		std::map<GLenum, std::vector<GLint>> mActiveUniformInfo;
	};
	
	struct TransformFeedbackVaryings {
		std::string		mName;
		GLint			mCount = 0;
		GLenum			mType;
		Attribute*		mAttachedAttrib;
	};
#endif
	
#if ! defined( CINDER_GL_ES )
	struct Subroutine {
		std::string mName;
		GLenum		mStage;
		std::vector<std::pair<GLint, std::string>> mIdName;
	};
#endif

	struct Format {
		//! Defaults to specifying location 0 for the \c geom::Attrib::POSITION semantic
		Format();
		
		//! Supplies the GLSL source for the vertex shader
		Format&		vertex( const DataSourceRef &dataSource );
		//! Supplies the GLSL source for the vertex shader
		Format&		vertex( const std::string &vertexShader );
		//! Supplies the GLSL source for the fragment shader
		Format&		fragment( const DataSourceRef &dataSource );
		//! Supplies the GLSL source for the fragment shader
		Format&		fragment( const std::string &vertexShader );
#if ! defined( CINDER_GL_ES )
		//! Supplies the GLSL source for the geometry shader
		Format&		geometry( const DataSourceRef &dataSource );
		//! Supplies the GLSL source for the geometry shader
		Format&		geometry( const std::string &geometryShader );
		//! Supplies the GLSL source for the tessellation control shader
		Format&		tessellationCtrl( const DataSourceRef &dataSource );
		//! Supplies the GLSL source for the tessellation control shader
		Format&		tessellationCtrl( const std::string &tessellationCtrlShader );
		//! Supplies the GLSL source for the tessellation control shader
		Format&		tessellationEval( const DataSourceRef &dataSource );
		//! Supplies the GLSL source for the tessellation control shader
		Format&		tessellationEval( const std::string &tessellationEvalShader );
#endif
#if ! defined( CINDER_GL_ES_2 )		
		//! Sets the TransformFeedback varyings
		Format&		feedbackVaryings( const std::vector<std::string>& varyings ) { mTransformVaryings = varyings; return *this; }
		//! Sets the TransformFeedback format
		Format&		feedbackFormat( GLenum format ) { mTransformFormat = format; return *this; }
#endif
		
		//! Specifies an attribute name to map to a specific semantic
		Format&		attrib( geom::Attrib semantic, const std::string &attribName );
		//! Specifies a uniform name to map to a specific semantic
		Format&		uniform( UniformSemantic semantic, const std::string &attribName );
		
		//! Specifies a location for a specific named attribute
		Format&		attribLocation( const std::string &attribName, GLint location );
		//! Specifies a location for a semantic
		Format&		attribLocation( geom::Attrib attr, GLint location );

#if ! defined( CINDER_GL_ES )
		//! Specifies a binding for a user-defined varying out variable to a fragment shader color number. Analogous to glBindFragDataLocation.
		Format&		fragDataLocation( GLuint colorNumber, const std::string &name );
#endif

		//! Returns the GLSL source for the vertex shader. Returns an empty string if it isn't present.
		const std::string&	getVertex() const { return mVertexShader; }
		//! Returns the GLSL source for the fragment shader. Returns an empty string if it isn't present.
		const std::string&	getFragment() const { return mFragmentShader; }
#if ! defined( CINDER_GL_ES )
		//! Returns the GLSL source for the geometry shader
		const std::string&	getGeometry() const { return mGeometryShader; }
		const std::string&	getTessellationCtrl() const { return mTessellationCtrlShader; }
		const std::string&	getTessellationEval() const { return mTessellationEvalShader; }
#endif
#if ! defined( CINDER_GL_ES_2 )
		const std::vector<std::string>&  getVaryings() const { return mTransformVaryings; }
		//! Returns the TransFormFeedback format
		GLenum			getTransformFormat() const { return mTransformFormat; }
		//! Returns the map between output variable names and their bound color numbers
		const std::map<std::string,GLuint>&	getFragDataLocations() const { return mFragDataLocations; }
#endif
		
		//! Returns the debugging label associated with the Program.
		const std::string&	getLabel() const { return mLabel; }
		//! Sets the debugging label associated with the Program. Calls glObjectLabel() when available.
		void				setLabel( const std::string &label ) { mLabel = label; }
		//! Sets the debugging label associated with the Program. Calls glObjectLabel() when available.
		Format&				label( const std::string &label ) { setLabel( label ); return *this; }
		const std::vector<Uniform>&		getUniforms() const { return mUniforms; }
		const std::vector<Attribute>&	getAttributes() const { return mAttributes; }
		std::vector<Uniform>&			getUniforms() { return mUniforms; }
		std::vector<Attribute>&			getAttributes() { return mAttributes; }
		
	  protected:
		std::string					mVertexShader;
		std::string					mFragmentShader;
#if ! defined( CINDER_GL_ES )
		std::string								mGeometryShader;
		std::string								mTessellationCtrlShader;
		std::string								mTessellationEvalShader;
#endif
#if ! defined( CINDER_GL_ES_2 )
		GLenum									mTransformFormat;
		std::vector<std::string>				mTransformVaryings;
		std::map<std::string,GLuint>			mFragDataLocations;
#endif
		std::vector<Attribute>					mAttributes;
		std::vector<Uniform>					mUniforms;
		
		
		std::string								mLabel;
	};
  
	typedef std::map<std::string,UniformSemantic>	UniformSemanticMap;
	typedef std::map<std::string,geom::Attrib>		AttribSemanticMap;

	static GlslProgRef create( const Format &format );

#if ! defined( CINDER_GL_ES )
	static GlslProgRef create( DataSourceRef vertexShader,
								   DataSourceRef fragmentShader = DataSourceRef(),
								   DataSourceRef geometryShader = DataSourceRef(),
								   DataSourceRef tessEvalShader = DataSourceRef(),
								   DataSourceRef tessCtrlShader = DataSourceRef() );
	static GlslProgRef create( const std::string &vertexShader,
								   const std::string &fragmentShader = std::string(),
								   const std::string &geometryShader = std::string(),
								   const std::string &tessEvalShader = std::string(),
								   const std::string &tessCtrlShader = std::string() );
#else
	static GlslProgRef create( DataSourceRef vertexShader, DataSourceRef fragmentShader = DataSourceRef() );
	static GlslProgRef create( const std::string &vertexShader, const std::string &fragmentShader = std::string() );
#endif
	~GlslProg();
	
	void			bind() const;
	
	GLuint			getHandle() const { return mHandle; }
	
	//! Possible implementation for uniform buffering with \a name.
	template<typename T>
	inline void	uniformTemp( const std::string &name, const T &data ) const;
	
	void	uniform( const std::string &name, bool data ) const;
	void	uniform( int location, bool data ) const;
	void	uniform( const std::string &name, int data ) const;
	void	uniform( int location, int data ) const;
	void	uniform( const std::string &name, uint32_t data ) const;
	void	uniform( int location, uint32_t data ) const;
	void	uniform( const std::string &name, const ivec2 &data ) const;
	void	uniform( int location, const ivec2 &data ) const;	
	void	uniform( const std::string &name, const int *data, int count ) const;
	void	uniform( int location, const int *data, int count ) const;	
	void	uniform( const std::string &name, const ivec2 *data, int count ) const;
	void	uniform( int location, const ivec2 *data, int count ) const;	
	void	uniform( const std::string &name, float data ) const;
	void	uniform( int location, float data ) const;	
	void	uniform( const std::string &name, const vec2 &data ) const;
	void	uniform( int location, const vec2 &data ) const;	
	void	uniform( const std::string &name, const vec3 &data ) const;
	void	uniform( int location, const vec3 &data ) const;
	void	uniform( const std::string &name, const vec4 &data ) const;
	void	uniform( int location, const vec4 &data ) const;
	void	uniform( const std::string &name, const uvec2 &data ) const;
	void	uniform( int location, const uvec2 &data ) const;
	void	uniform( const std::string &name, const uvec3 &data ) const;
	void	uniform( int location, const uvec3 &data ) const;
	void	uniform( const std::string &name, const uvec4 &data ) const;
	void	uniform( int location, const uvec4 &data ) const;
	void	uniform( const std::string &name, const Color &data ) const;
	void	uniform( int location, const Color &data ) const;
	void	uniform( const std::string &name, const ColorA &data ) const;
	void	uniform( int location, const ColorA &data ) const;
	void	uniform( const std::string &name, const mat3 &data, bool transpose = false ) const;
	void	uniform( int location, const mat3 &data, bool transpose = false ) const;
	void	uniform( const std::string &name, const mat4 &data, bool transpose = false ) const;
	void	uniform( int location, const mat4 &data, bool transpose = false ) const;
	void	uniform( const std::string &name, const float *data, int count ) const;
	void	uniform( int location, const float *data, int count ) const;
	void	uniform( const std::string &name, const vec2 *data, int count ) const;
	void	uniform( int location, const vec2 *data, int count ) const;
	void	uniform( const std::string &name, const vec3 *data, int count ) const;
	void	uniform( int location, const vec3 *data, int count ) const;
	void	uniform( const std::string &name, const vec4 *data, int count ) const;
	void	uniform( int location, const vec4 *data, int count ) const;
	void    uniform( int location, const mat3 *data, int count, bool transpose = false ) const;
	void    uniform( const std::string &name, const mat3 *data, int count, bool transpose = false ) const;
	void    uniform( int location, const mat4 *data, int count, bool transpose = false ) const;
	void    uniform( const std::string &name, const mat4 *data, int count, bool transpose = false ) const;
	
	bool	hasAttribSemantic( geom::Attrib semantic ) const;
	GLint	getAttribSemanticLocation( geom::Attrib semantic ) const;
	GLint	operator[]( geom::Attrib sem ) const { return getAttribSemanticLocation( sem ); }
	
	//! Default mapping from uniform name to semantic. Can be modified via the reference. Not thread-safe.
	static UniformSemanticMap&		getDefaultUniformNameToSemanticMap();
	//! Default mapping from attribute name to semantic. Can be modified via the reference. Not thread-safe.
	static AttribSemanticMap&		getDefaultAttribNameToSemanticMap();
	
	GLint							getAttribLocation( const std::string &name ) const;
	const std::vector<Attribute>&	getActiveAttributes() const { return mAttributes; }
	GLint							getUniformLocation( const std::string &name ) const;
	const std::vector<Uniform>&		getActiveUniforms() const { return mUniforms; }

#if ! defined( CINDER_GL_ES_2 )
	// Uniform blocks
	//! Analogous to glUniformBlockBinding()
	void	uniformBlock( const std::string &name, GLint binding );
	//! Analogous to glUniformBlockBinding()
	void	uniformBlock( GLint loc, GLint binding );
	GLint	getUniformBlockLocation( const std::string &name ) const;
	GLint	getUniformBlockSize( GLint blockIndex ) const;
	const std::vector<UniformBlock>& getActiveUniformBlocks() const { return mUniformBlocks; }
	const std::vector<TransformFeedbackVaryings>& getActiveTransformFeedbackVaryings() const { return mTransformFeedbackVaryings; }
#endif
	
#if ! defined( CINDER_GL_ES )
	const std::vector<Subroutine> getActiveSubroutines() const { return mSubroutines; }
#endif
	
	std::string		getShaderLog( GLuint handle ) const;

	//! Returns the debugging label associated with the Program.
	const std::string&	getLabel() const { return mLabel; }
	//! Sets the debugging label associated with the Program. Calls glObjectLabel() when available.
	void				setLabel( const std::string &label );

  protected:
	GlslProg( const Format &format );

	void			bindImpl();
	void			loadShader( const std::string &shaderSource, GLint shaderType );
	void			attachShaders();
	void			link();
	
	void				cacheActiveAttribs();
	Attribute*			findAttrib( const std::string &name );
	const Attribute*	findAttrib( const std::string &name ) const;
	//! Querys gl to provide all active uniforms outside of uniform blocks and caches them inside \a mUniforms.
	void				cacheActiveUniforms();
	//! Returns a pointer to the uniform named by \a name.
	Uniform*			findUniform( const std::string &name );
	//! Returns a const pointer to the uniform named by \a name.
	const Uniform*		findUniform( const std::string &name ) const;
	//! Logs a message to the user that the name provided doesn't exist in the uniform cache.
	void				logMissingUniform( const std::string &name ) const;
	//! Checks the validity of the settings on this uniform, specifically type and value
	template<typename T>
	inline bool			validateUniform( const Uniform &uniform, const T &val ) const;
	//! Implementing later for CPU Uniform Buffer Cache.
	template<typename T>
	inline bool			checkUniformValue( const Uniform &uniform, const T &val ) const;
	//! Checks the type of the uniform against the provided type of data in validateUniform. If the provided
	//! type, \a uniformType, and the type T match this function returns true, otherwise it returns false.
	template<typename T>
	inline bool			checkUniformType( GLenum uniformType ) const;
#if ! defined( CINDER_GL_ES_2 )
	void				cacheActiveUniformBlocks();
	UniformBlock*		findUniformBlock( const std::string &name );
	const UniformBlock* findUniformBlock( const std::string &name ) const;
	void				cacheActiveTransformFeedbackVaryings();
#endif
	
#if ! defined( CINDER_GL_ES )
	void				cacheActiveSubroutines();
	Subroutine*			findSubroutine( const std::string &name ) const;
#endif
	
	GLuint									mHandle;
	
	static UniformSemanticMap				sDefaultUniformNameToSemanticMap;
	static AttribSemanticMap				sDefaultAttribNameToSemanticMap;
	
	std::vector<Attribute>					mAttributes;
	std::vector<Uniform>					mUniforms;
#if ! defined( CINDER_GL_ES_2 )
	std::vector<TransformFeedbackVaryings>  mTransformFeedbackVaryings;
	std::vector<UniformBlock>				mUniformBlocks;
#endif
	
#if ! defined( CINDER_GL_ES )
	std::vector<Subroutine>					mSubroutines;
#endif
	
	// enumerates the uniforms we've already logged as missing so that we don't flood the log with the same message
	mutable std::set<std::string>			mLoggedUniforms;

	std::string								mLabel; // debug label

	// storage as a work-around for NVidia on MSW driver bug expecting persistent memory in calls to glTransformFeedbackVaryings
#if ! defined( CINDER_GL_ES_2 )
	std::unique_ptr<std::vector<GLchar>>	mTransformFeedbackVaryingsChars;
	std::unique_ptr<std::vector<GLchar*>>	mTransformFeedbackVaryingsCharStarts;
	GLenum									mTransformFeedbackFormat;
#endif

	friend class Context;
	friend std::ostream& operator<<( std::ostream &os, const GlslProg &rhs );
};
	

	
template<typename T>
inline void GlslProg::uniformTemp( const std::string &name, const T &data ) const
{	
	auto found = findUniform( name );
	if( ! found ) {
		logMissingUniform( name );
		return;
	}
	if( validateUniform( found, data ) )
		uniform( found->mLoc, data );
}
	
template<typename T>
inline bool GlslProg::validateUniform( const Uniform &uniform, const T &val ) const
{
	if( ! checkUniformType<T>( uniform.mType ) && mLoggedUniforms.count( uniform.mName ) == 0 ) {
		//		CI_LOG_W("Uniform type mismatch for \"" << found->mName << "\", expected "
		//				 << gl::constantToString(found->mType) << " and received a float.");
		mLoggedUniforms.insert( uniform.mName );
		return false;
	}
	else
		return true;
}
	
template<typename T>
inline bool GlslProg::checkUniformType( GLenum uniformType ) const { return false; }
template<>
inline bool GlslProg::checkUniformType<bool>( GLenum uniformType ) const { return GL_BOOL == uniformType; }
template<>
inline bool GlslProg::checkUniformType<int>( GLenum uniformType ) const
{
	return GL_INT == uniformType ||
			GL_SAMPLER_2D == uniformType ||
			GL_SAMPLER_1D == uniformType ||
			GL_SAMPLER_3D == uniformType ||
			GL_SAMPLER_2D_SHADOW == uniformType ||
			GL_BOOL == uniformType ||
			GL_SAMPLER_CUBE == uniformType;
}
template<>
inline bool GlslProg::checkUniformType<float>( GLenum uniformType ) const { return GL_FLOAT == uniformType; }
template<>
inline bool GlslProg::checkUniformType<uint32_t>( GLenum uniformType ) const { return GL_UNSIGNED_INT == uniformType; }
template<>
inline bool GlslProg::checkUniformType<glm::bvec2>( GLenum uniformType ) const { return GL_BOOL_VEC2 == uniformType; }
template<>
inline bool GlslProg::checkUniformType<ivec2>( GLenum uniformType ) const { return GL_INT_VEC2 == uniformType; }
template<>
inline bool GlslProg::checkUniformType<vec2>( GLenum uniformType ) const { return GL_FLOAT_VEC2 == uniformType; }
template<>
inline bool GlslProg::checkUniformType<uvec2>( GLenum uniformType ) const { return GL_UNSIGNED_INT_VEC2 == uniformType; }
template<>
inline bool GlslProg::checkUniformType<glm::bvec3>( GLenum uniformType ) const { return GL_BOOL_VEC3 == uniformType; }
template<>
inline bool GlslProg::checkUniformType<ivec3>( GLenum uniformType ) const { return GL_INT_VEC3 == uniformType; }
template<>
inline bool GlslProg::checkUniformType<vec3>( GLenum uniformType ) const { return GL_FLOAT_VEC3 == uniformType; }
template<>
inline bool GlslProg::checkUniformType<uvec3>( GLenum uniformType ) const { return GL_UNSIGNED_INT_VEC3 == uniformType; }
template<>
inline bool GlslProg::checkUniformType<glm::bvec4>( GLenum uniformType ) const { return GL_BOOL_VEC4 == uniformType; }
template<>
inline bool GlslProg::checkUniformType<ivec4>( GLenum uniformType ) const { return GL_INT_VEC4 == uniformType; }
template<>
inline bool GlslProg::checkUniformType<vec4>( GLenum uniformType ) const { return GL_FLOAT_VEC4 == uniformType; }
template<>
inline bool GlslProg::checkUniformType<uvec4>( GLenum uniformType ) const { return GL_UNSIGNED_INT_VEC4 == uniformType; }
template<>
inline bool GlslProg::checkUniformType<mat4>( GLenum uniformType ) const { return GL_FLOAT_MAT4 == uniformType; }
template<>
inline bool GlslProg::checkUniformType<mat3>( GLenum uniformType ) const { return GL_FLOAT_MAT3 == uniformType; }


class GlslProgExc : public cinder::gl::Exception {
  public:
	GlslProgExc()	{}
	GlslProgExc( const std::string &description ) : cinder::gl::Exception( description )	{}
};


class GlslProgCompileExc : public GlslProgExc {
  public:
	GlslProgCompileExc( const std::string &log, GLint shaderType );
};

class GlslProgLinkExc : public GlslProgExc {
  public:
	GlslProgLinkExc( const std::string &log ) : GlslProgExc( log ) {}
};

class GlslNullProgramExc : public GlslProgExc {
  public:
	virtual const char* what() const throw()
	{
		return "Glsl: Attempt to use null shader";
	}
	
};

} } // namespace cinder::gl
