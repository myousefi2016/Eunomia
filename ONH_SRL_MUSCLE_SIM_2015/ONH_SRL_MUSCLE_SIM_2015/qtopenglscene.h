#ifndef QTOPENGLSCENE_H
#define QTOPENGLSCENE_H

#include <string>
#include "cMacros.h"
#include "abstractscene.h"

#include <qdatetime.h>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QObject>
#include <QOpenGLContext>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "cAssimpMesh.h"
#include "cAssimpManager.h"
#include "cTextureManager.h"

using namespace std;

class QtOpenGLScene : public AbstractScene
{
public:
    QtOpenGLScene();

    virtual int initialize();
    virtual void update(float t);
    virtual void render(int rotModelX, int rotModelY, int rotModelZ, int posCamX, int posCamY, int posCamZ, int fovY, int opacity);
    virtual void resize(int width, int height);
	
	virtual int load_ANCONEUS();
	virtual int load_BRACHIALIS();
	virtual int load_BRACHIORDIALIS();
	virtual int load_PRONATOR_TERES();
	virtual int load_BICEPS_BRACHII();
	virtual int load_TRICEPS_BRACHII();
	virtual int load_OTHER();

	virtual void toggle_ANCONEUS(bool v);
	virtual void toggle_BRACHIALIS(bool v);
	virtual void toggle_BRACHIORDIALIS(bool v);
	virtual void toggle_PRONATOR_TERES(bool v);
	virtual void toggle_BICEPS_BRACHII(bool v);
	virtual void toggle_TRICEPS_BRACHII(bool v);
	virtual void toggle_OTHER(bool v);

private:
	void prepareShaderProgram();
	void prepareVertexBuffers();
	void genVAOsAndUniformBuffer(const aiScene* sc, vector<AssimpMesh>&	meshes);
	void set_float4(float f[4], float a, float b, float c, float d);
	void color4_to_float4(const aiColor4D* c, float f[4]);
	void recursive_render(const aiScene* sc, const aiNode* nd, vector<AssimpMesh>&	meshes);
	void pushMatrix();
	void popMatrix();
	void setRotationMatrix(glm::mat4& mat, float angle, float x, float y, float z);
	void setModelMatrix();
	void translate(float x, float y, float z);
	void rotate(float angle, float x, float y, float z);
	void scale(float x, float y, float z);
	void buildProjectionMatrix(float fov, float ratio, float nearp, float farp);
	void setCamera(float posX, float posY, float posZ, float lookAtX, float lookAtY, float lookAtZ);
	void renderScene(int rotModelX, int rotModelY, int rotModelZ, int posCamX, int posCamY, int posCamZ, int fovY, int opacity);

    QOpenGLShaderProgram mShaderProgram;

	//Custom objects:
#ifndef __MODEL_SECTION
#define __MODEL_SECTION
	struct MODEL_SECTION
	{
		bool					model_loaded;
		bool					model_shown;
		std::string				model_path;
		AssimpManager*			model_manager;
		std::vector<AssimpMesh>	model_meshes;
	}MODEL_CONTAINER;
#endif /*__MODEL_SECTION*/

	vector<MODEL_SECTION> arm_parts;

	MODEL_SECTION		axes;

	TextureManager*		tex_manager;

	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;

	// For push and pop matrix
	vector<glm::mat4> matrixStack;

	// Camera Position
	float camX;
	float camY;
	float camZ;
	float ratio;
	float fovY;
	float lastFovY;
	float nearP;
	float farP;
	float opacity;

	// Mouse Tracking Variables
	int startX, startY, tracking = 0;

	// Camera Spherical Coordinates
	float alpha = 0.0f, beta = 0.0f;
	float r = 5.0f;

	// Frame counting and FPS computation
	long time, timebase = 0, frame = 0;
	char s[32];

	int texUnit;
};

#endif // QtOpenGLScene_H
