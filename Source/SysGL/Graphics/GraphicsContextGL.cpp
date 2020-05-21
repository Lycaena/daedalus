
#include "Base/Daedalus.h"

#include <stdio.h>

#include "SysGL/GL.h"

#include "Graphics/GraphicsContext.h"

#include "Graphics/ColourValue.h"


static u32 SCR_WIDTH = 640;
static u32 SCR_HEIGHT = 480;
#define MAX_INDEXES 0xFFFF
uint16_t *gIndexes;
float *gVertexBuffer;
uint32_t *gColorBuffer;
float *gTexCoordBuffer;
float *gVertexBufferPtr;
uint32_t *gColorBufferPtr;
float *gTexCoordBufferPtr;
SDL_Window * gWindow = NULL;
bool new_frame = true;


class GraphicsContextGL : public CGraphicsContext
{
public:
	GraphicsContextGL();
	virtual ~GraphicsContextGL();


	virtual bool Initialise();
	virtual bool IsInitialised() const { return true; }

	virtual void ClearAllSurfaces();
	virtual void ClearZBuffer();
	virtual void ClearColBuffer(const c32 & colour);
	virtual void ClearToBlack();
	virtual void ClearColBufferAndDepth(const c32 & colour);
	virtual	void BeginFrame();
	virtual void EndFrame();
	virtual void UpdateFrame( bool wait_for_vbl );

	virtual void GetScreenSize(u32 * width, u32 * height) const;
	virtual void ViewportType(u32 * width, u32 * height) const;

	virtual void SetDebugScreenTarget( ETargetSurface buffer ) {}
	virtual void DumpNextScreen() {}
	virtual void DumpScreenShot() {}
};

template<> bool CSingleton< CGraphicsContext >::Create()
{
	DAEDALUS_ASSERT_Q(mpInstance == NULL);

	mpInstance = new GraphicsContextGL();
	return mpInstance->Initialise();
}

GraphicsContextGL::GraphicsContextGL()
{
	uint16_t i;
	gIndexes = (uint16_t*)malloc(sizeof(uint16_t)*MAX_INDEXES);
	for (i = 0; i < MAX_INDEXES; i++){
		gIndexes[i] = i;
	}
	gVertexBufferPtr = (float*)malloc(0x800000);
	gColorBufferPtr = (uint32_t*)malloc(0x600000);
	gTexCoordBufferPtr = (float*)malloc(0x600000);
	gVertexBuffer = gVertexBufferPtr;
	gColorBuffer = gColorBufferPtr;
	gTexCoordBuffer = gTexCoordBufferPtr;
}

GraphicsContextGL::~GraphicsContextGL()
{
		SDL_DestroyWindow(gWindow);
		gWindow = NULL;
		SDL_Quit();
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %d - %s\n", error, description);
}


extern bool initgl();
bool GraphicsContextGL::Initialise()
{

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		return false;
	}
		//Use OpenGL 3.3 core
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		//Create window
		gWindow = SDL_CreateWindow( "Daedalus", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_OPENGL );

		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			return false;
		}

			//Create context
	SDL_GLContext	gContext = SDL_GL_CreateContext( gWindow );

	SDL_GL_SetSwapInterval(1);

	GLenum err = glewInit();
	if (err != GLEW_OK || !GLEW_VERSION_3_2)
	{
	SDL_DestroyWindow(gWindow);

		gWindow = NULL;
		SDL_Quit();
		return false;
	}
//ClearColBufferAndDepth(0,0,0,0);
UpdateFrame(false);
return initgl();
}


void GraphicsContextGL::GetScreenSize(u32 * width, u32 * height) const
{
	int window_width, window_height;
	SDL_GL_GetDrawableSize(gWindow, &window_width, &window_height);

	*width  = window_width;
	*height = window_height;
}

void GraphicsContextGL::ViewportType(u32 * width, u32 * height) const
{
	GetScreenSize(width, height);
}

void GraphicsContextGL::ClearAllSurfaces()
{
	// FIXME: this should clear/flip a couple of times to ensure the front and backbuffers are cleared.
	// Not sure if it's necessary...
	ClearToBlack();
}


void GraphicsContextGL::ClearToBlack()
{
	glDepthMask(GL_TRUE);
	glClearDepth( 1.0f );
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void GraphicsContextGL::ClearZBuffer()
{
	glDepthMask(GL_TRUE);
	glClearDepth( 1.0f );
	glClear( GL_DEPTH_BUFFER_BIT );
}

void GraphicsContextGL::ClearColBuffer(const c32 & colour)
{
	glClearColor( colour.GetRf(), colour.GetGf(), colour.GetBf(), colour.GetAf() );
	glClear( GL_COLOR_BUFFER_BIT );
}

void GraphicsContextGL::ClearColBufferAndDepth(const c32 & colour)
{
	glDepthMask(GL_TRUE);
	glClearDepth( 1.0f );
	glClearColor( colour.GetRf(), colour.GetGf(), colour.GetBf(), colour.GetAf() );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void GraphicsContextGL::BeginFrame()
{
	// Get window size (may be different than the requested size)
	u32 width, height;
	GetScreenSize(&width, &height);
		glEnableClientState(GL_VERTEX_ARRAY);
	gVertexBuffer = gVertexBufferPtr;
gColorBuffer = gColorBufferPtr;
gTexCoordBuffer = gTexCoordBufferPtr;

	// Special case: avoid division by zero below
	height = height > 0 ? height : 1;

if (new_frame)
{
	CGraphicsContext::Get()->ClearToBlack();
new_frame = false;
}

}

void GraphicsContextGL::EndFrame()
{
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glScissor( 0, 0, SCR_WIDTH, SCR_HEIGHT);
}

void GraphicsContextGL::UpdateFrame( bool wait_for_vbl )
{
	SDL_GL_SwapWindow(gWindow);
	new_frame = true;

//	if( gCleanSceneEnabled ) //TODO: This should be optional
//	{
	//	ClearColBuffer( c32(0xff000000) ); // ToDo : Use gFillColor instead?
	//}
}
