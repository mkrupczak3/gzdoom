#ifndef __QDRAWER_H
#define __QDRAWER_H

#include "gl/data/gl_vertexbuffer.h"


#ifdef NO_VBO

// #define GL_BEGIN 1  This might be quicker due to jwzgles batching.. doesn't seem to be

extern "C" void jwzgles_glBegin_OVERRIDE(int mode);

class FQuadDrawer
{
	static FFlatVertex buffer[4];

	void DoRender(int type);
public:

	FQuadDrawer()
	{
#if GL_BEGIN
	    glBegin(GL_TRIANGLE_FAN);
#endif
	}

	void Set(int ndx, float x, float y, float z, float s, float t)
	{
#if GL_BEGIN
        glTexCoord2f(s,t);
        glVertex3f(x,y,z);
#else
        buffer[ndx].Set(x, y, z, s, t);
#endif
	}
	void Render(int type)
	{
#if GL_BEGIN
	    jwzgles_glBegin_OVERRIDE( type );
	    glEnd();
#else
        glTexCoordPointer(2,GL_FLOAT, sizeof(FFlatVertex),&buffer[0].u);
        glVertexPointer  (3,GL_FLOAT, sizeof(FFlatVertex),&buffer[0].x);

		glEnableClientState (GL_VERTEX_ARRAY);
		glEnableClientState (GL_TEXTURE_COORD_ARRAY);
		glDisableClientState (GL_COLOR_ARRAY);

		glBindBuffer (GL_ARRAY_BUFFER, 0); // NO VBO
        glDrawArrays (type, 0, 4);
#endif
	}
};

#else

class FQuadDrawer
{
	FFlatVertex *p;
	int ndx;
	static FFlatVertex buffer[4];
	
	void DoRender(int type);
public:

	FQuadDrawer()
	{
		if (gl.buffermethod == BM_DEFERRED)
		{
			p = buffer;
		}
		else
		{
			p = GLRenderer->mVBO->Alloc(4, &ndx);
		}
	}
	void Set(int ndx, float x, float y, float z, float s, float t)
	{
		p[ndx].Set(x, y, z, s, t);
	}
	void Render(int type)
	{
		if (gl.buffermethod == BM_DEFERRED)
		{
			DoRender(type);
		}
		else
		{
			GLRenderer->mVBO->RenderArray(type, ndx, 4);
		}
	}
};

#endif

#endif
