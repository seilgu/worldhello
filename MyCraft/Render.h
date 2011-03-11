
#ifndef _RENDER_H_
#define _RENDER_H_

#include "common.h"
#include <Windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include "glext.h"
#include "Map.h"
#include "texture.h"


class World;
class TextureMgr;
extern class World *s_World;
extern class TextureMgr *s_Texture;


/*************************************************************************
	A singleton class that handles graphics drawing, 
	given the player's position, it decides how far it will draw
*************************************************************************/
struct render_chunk {
	int3 id;
	int num_faces;
	GLuint vbo;
	unsigned short loaded:1;
	unsigned short failed:1;
	unsigned short unneeded:1;
};

typedef std::map<int3, render_chunk *, id_compare> render_list;

class Render {
public :
	static const int PZ = 2;
	static const int NZ = 3;
	static const int PY = 0;
	static const int NY = 1;
	static const int PX = 4;
	static const int NX = 5;


	render_list r_chunks;

	Render() {
		glGenBuffersARB = 0;                     // VBO Name Generation Procedure
		glBindBufferARB = 0;                     // VBO Bind Procedure
		glBufferDataARB = 0;                     // VBO Data Loading Procedure
		glBufferSubDataARB = 0;               // VBO Sub Data Loading Procedure
		glDeleteBuffersARB = 0;               // VBO Deletion Procedure
		glGetBufferParameterivARB = 0; // return various parameters of VBO
		glMapBufferARB = 0;                       // map VBO procedure
		glUnmapBufferARB = 0;                   // unmap VBO procedure

		CheckVBOSupport();

		m_Thread.SetRenderInstance(this);

		//m_Thread.Start();              // Doesn't WORK

	}
	~Render();

	void CheckVBOSupport();
	void DeleteChunk(render_chunk *chk);
	void LoadChunk(render_chunk *ren_chk, map_chunk *map_chk, int urgent);

	void CalculateVisible(int3 id);
	void LoadNeededChunks(float3 pos, float3 dir, World *world);
	void DiscardUnneededChunks(float3 pos, float3 dir, World *world);
	// Draw to screen
	void DrawScene(float3 pos, float3 dir, float dist);
	void DrawScene2(float3 pos, float3 dir, float dist);
	void RenderChunk(map_chunk *chk, float3 pos, float3 dir);
	void RenderChunk2(render_chunk *tmp, float3 pos, float3 dir);
	void DrawFaceSimple(int i, int j, int k, int type, int dir);
	void PrintChunkStatistics(char *buffer);

	typedef std::pair<render_chunk *, map_chunk *> render_pair;
	class RenderChunkThread {
	private:
		Render *render;
		bool active;
	public:
		std::queue<render_pair> jobs;
		//std::queue<render_chunk *> jobs;
		HANDLE handle;

		Render *GetRenderInstance() {
			return render;
		}

		void SetRenderInstance(Render *inst) {
			if (inst != 0)
				render = inst;
		}

		RenderChunkThread() {
			active = false;
		}

		~RenderChunkThread() {
			// kill the thread
			active = false;
			// wait for thread to complete
			WaitForSingleObject(handle, INFINITE);
			CloseHandle(handle);
		}

		void Start() {
			if (active == true)
				return;

			active = true;
			// create thread and run it
			handle = (HANDLE)_beginthread(RenderChunkThread::threadLoop, 0, this);
		}

		void PushJobs(render_pair pair) {
			jobs.push(pair);
		}

		void threadLoadChunk(render_pair pair, RenderChunkThread *self);
		static void threadLoop(void *param);
	}m_Thread;

public:
	PFNGLGENBUFFERSARBPROC glGenBuffersARB;                     // VBO Name Generation Procedure
	PFNGLBINDBUFFERARBPROC glBindBufferARB;                     // VBO Bind Procedure
	PFNGLBUFFERDATAARBPROC glBufferDataARB;                     // VBO Data Loading Procedure
	PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB;               // VBO Sub Data Loading Procedure
	PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;               // VBO Deletion Procedure
	PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB; // return various parameters of VBO
	PFNGLMAPBUFFERARBPROC glMapBufferARB;                       // map VBO procedure
	PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB;                   // unmap VBO procedure
};


#endif