#include "Angel.h"
#include "TriMesh.h"
#include "MeshPainter.h"
#include <learnopengl/camera.h>
#include <learnopengl/shader.h>
#include <stb_image.h>
#include <vector>
#include <string>

class MatrixStack {
	int		_index;
	int		_size;
	glm::mat4* _matrices;

public:
	MatrixStack(int numMatrices = 100) :_index(0), _size(numMatrices)
	{
		_matrices = new glm::mat4[numMatrices];
	}

	~MatrixStack()
	{
		delete[]_matrices;
	}

	void push(const glm::mat4& m) {
		assert(_index + 1 < _size);
		_matrices[_index++] = m;
	}

	glm::mat4& pop() {
		assert(_index - 1 >= 0);
		_index--;
		return _matrices[_index];
	}
};

struct Robot
{
	// 关节大小
	float TORSO_HEIGHT = 5.5 / 5;
	float TORSO_WIDTH = 3.0 / 5;
	float UPPER_ARM_HEIGHT = 3.0 / 5;
	float LOWER_ARM_HEIGHT = 2.0 / 5;
	float UPPER_ARM_WIDTH = 1.0 / 5;
	float LOWER_ARM_WIDTH = 0.6 / 5;
	float UPPER_LEG_HEIGHT = 2.8 / 5;
	float LOWER_LEG_HEIGHT = 2.0 / 5;
	float UPPER_LEG_WIDTH = 1.0 / 5;
	float LOWER_LEG_WIDTH = 0.6 / 5;
	float HEAD_HEIGHT = 1.8 / 5;
	float HEAD_WIDTH = 1.8 / 5;

	// 关节角和菜单选项值
	enum {
		Torso,			// 躯干
		Head,			// 头部
		RightUpperArm,	// 右大臂
		RightLowerArm,	// 右小臂
		LeftUpperArm,	// 左大臂
		LeftLowerArm,	// 左小臂
		RightUpperLeg,	// 右大腿
		RightLowerLeg,	// 右小腿
		LeftUpperLeg,	// 左大腿
		LeftLowerLeg,	// 左小腿
	};

	// 关节角大小
	GLfloat theta[10] = {
		0.0,    // Torso
		0.0,    // Head
		0.0,    // RightUpperArm
		0.0,    // RightLowerArm
		0.0,    // LeftUpperArm
		0.0,    // LeftLowerArm
		0.0,    // RightUpperLeg
		0.0,    // RightLowerLeg
		0.0,    // LeftUpperLeg
		0.0     // LeftLowerLeg
	};
};
Robot robot;
// 被选中的物体
int Selected_mesh = robot.Torso;


//创建机器人
TriMesh* Torso = new TriMesh();
TriMesh* Head = new TriMesh();
TriMesh* RightUpperArm = new TriMesh();
TriMesh* RightLowerArm = new TriMesh();
TriMesh* LeftUpperArm = new TriMesh();
TriMesh* LeftLowerArm = new TriMesh();
TriMesh* RightUpperLeg = new TriMesh();
TriMesh* RightLowerLeg = new TriMesh();
TriMesh* LeftUpperLeg = new TriMesh();
TriMesh* LeftLowerLeg = new TriMesh();


//初始让鼠标位于窗口中心
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

int mainWindow;

//鼠标从外面进入窗口,设为true
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera* camera = new Camera(glm::vec3(0.3f, 0.5f, 5.0f));
Light* light = new Light();
MeshPainter* painter = new MeshPainter();

// 这个用来回收和删除我们创建的物体对象
std::vector<TriMesh*> meshList;

float skyboxVertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

vector<std::string> faces
{
	"./assets/skybox/arch3_lf.png",
	"./assets/skybox/arch3_rt.png",
	"./assets/skybox/arch3_up.png",
	"./assets/skybox/arch3_dn.png",
	"./assets/skybox/arch3_ft.png",
	"./assets/skybox/arch3_bk.png"

	//"./assets/skybox/right.jpg",
	//"./assets/skybox/left.jpg",
	//"./assets/skybox/top.jpg",
	//"./assets/skybox/bottom.jpg",
	//"./assets/skybox/front.jpg",
	//"./assets/skybox/back.jpg"
};


// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
GLuint loadCubemap(vector<std::string> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (GLuint i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}


void init()
{
	camera->projMatrix = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

	std::string vshader, fshader;
	// 读取着色器并使用
	vshader = "shaders/vshader_win.glsl";
	fshader = "shaders/fshader_win.glsl";

	// 设置光源位置
	light->setTranslation(glm::vec3(0, 15.0, -15.0));
	light->setAmbient(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 环境光
	light->setDiffuse(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 漫反射
	light->setSpecular(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 镜面反射
	light->setAttenuation(1.0, 0.045, 0.0075); // 衰减系数

	// 杰尼龟
	TriMesh* squirtle = new TriMesh();
	squirtle->setNormalize(true);
	squirtle->readOff("./assets/off/Squirtle.off");

	// 设置物体的旋转位移
	squirtle->setTranslation(glm::vec3(0.5, 0.3, 0.0));
	squirtle->setRotation(glm::vec3(0.0, 0.0, 0.0));
	squirtle->setScale(glm::vec3(1.0, 1.0, 1.0));

	// 设置材质
	squirtle->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	squirtle->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	squirtle->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	squirtle->setShininess(1.0); //高光系数

	painter->addMesh(squirtle, "squirtle", "./assets/Squirtle.png", vshader, fshader);

	// 魔毯
	TriMesh* plane = new TriMesh();

	plane->isPlane = true;
	plane->setNormalize(false);
	// 创建正方形平面，给它一个其他颜色
	plane->generateSquare(glm::vec3(0, 0, 0));
	plane->setTranslation(glm::vec3(0.3, -0.001, 0.3));
	plane->setRotation(glm::vec3(-90.0, 0, 0));
	plane->setScale(glm::vec3(3, 3, 3));

	painter->addMesh(plane, "plane", "./assets/magic_carpet.jpg", "./shaders/plane_vs.glsl", "./shaders/plane_fs.glsl");


	// 设置物体的大小（初始的旋转和位移都为0）
	Torso->generateCube();
	Head->generateCube();
	RightUpperArm->generateCube();
	LeftUpperArm->generateCube();
	RightUpperLeg->generateCube();
	LeftUpperLeg->generateCube();
	RightLowerArm->generateCube();
	LeftLowerArm->generateCube();
	RightLowerLeg->generateCube();
	LeftLowerLeg->generateCube();


	// 设置材质
	Torso->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	Torso->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	Torso->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	Torso->setShininess(1.0); //高光系数

	Head->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	Head->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	Head->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	Head->setShininess(1.0); //高光系数

	RightUpperArm->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	RightUpperArm->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	RightUpperArm->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	RightUpperArm->setShininess(1.0); //高光系数

	LeftUpperArm->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	LeftUpperArm->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	LeftUpperArm->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	LeftUpperArm->setShininess(1.0); //高光系数

	RightUpperLeg->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	RightUpperLeg->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	RightUpperLeg->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	RightUpperLeg->setShininess(1.0); //高光系数

	LeftUpperLeg->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	LeftUpperLeg->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	LeftUpperLeg->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	LeftUpperLeg->setShininess(1.0); //高光系数

	RightLowerArm->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	RightLowerArm->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	RightLowerArm->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	RightLowerArm->setShininess(1.0); //高光系数

	LeftLowerArm->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	LeftLowerArm->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	LeftLowerArm->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	LeftLowerArm->setShininess(1.0); //高光系数

	RightLowerLeg->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	RightLowerLeg->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	RightLowerLeg->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	RightLowerLeg->setShininess(1.0); //高光系数

	LeftLowerLeg->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	LeftLowerLeg->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	LeftLowerLeg->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	LeftLowerLeg->setShininess(1.0); //高光系数

	Torso->isRobot = true;
	Head->isRobot = true;
	RightUpperArm->isRobot = true;;
	LeftUpperArm->isRobot = true;;
	RightUpperLeg->isRobot = true;
	LeftUpperLeg->isRobot = true;
	RightLowerArm->isRobot = true;
	LeftLowerArm->isRobot = true;
	RightLowerLeg->isRobot = true;
	LeftLowerLeg->isRobot = true;

	//opengl_objects[i], i = 2~11
	painter->addMesh(Torso, "Torso", "./assets/body_re.jpg", vshader, fshader);
	painter->addMesh(Head, "Head", "./assets/head.jpg", vshader, fshader);
	painter->addMesh(RightUpperArm, "RightUpperArm", "./assets/torso.jpg", vshader, fshader);
	painter->addMesh(LeftUpperArm, "LeftUpperArm", "./assets/torso.jpg", vshader, fshader);
	painter->addMesh(RightUpperLeg, "RightUpperLeg", "./assets/torso.jpg", vshader, fshader);
	painter->addMesh(LeftUpperLeg, "LeftUpperLeg", "./assets/torso.jpg", vshader, fshader);
	painter->addMesh(RightLowerArm, "RightLowerArm", "./assets/torso.jpg", vshader, fshader);
	painter->addMesh(LeftLowerArm, "LeftLowerArm", "./assets/torso.jpg", vshader, fshader);
	painter->addMesh(RightLowerLeg, "RightLowerLeg", "./assets/torso.jpg", vshader, fshader);
	painter->addMesh(LeftLowerLeg, "LeftLowerLeg", "./assets/torso.jpg", vshader, fshader);


	glClearColor(1.0, 1.0, 1.0, 1.0);
}



// 躯体
void torso(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.5 * robot.TORSO_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.TORSO_WIDTH, robot.TORSO_HEIGHT, robot.TORSO_WIDTH));

	Torso->modelMatrix = modelMatrix * instance;
	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	painter->drawMesh(Torso, painter->opengl_objects[2], light, camera);
}

// 头部
void head(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.5 * robot.HEAD_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.HEAD_WIDTH, robot.HEAD_HEIGHT, robot.HEAD_WIDTH));

	Head->modelMatrix = modelMatrix * instance;
	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	painter->drawMesh(Head, painter->opengl_objects[3], light, camera);
}


// 左大臂
void left_upper_arm(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_ARM_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_ARM_WIDTH, robot.UPPER_ARM_HEIGHT, robot.UPPER_ARM_WIDTH));

	LeftUpperArm->modelMatrix = modelMatrix * instance;
	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	painter->drawMesh(LeftUpperArm, painter->opengl_objects[4], light, camera);
}


// @TODO: 左小臂
void left_lower_arm(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.4 * robot.LOWER_ARM_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_ARM_WIDTH, robot.LOWER_ARM_HEIGHT, robot.LOWER_ARM_WIDTH));

	LeftLowerArm->modelMatrix = modelMatrix * instance;
	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	painter->drawMesh(LeftLowerArm, painter->opengl_objects[5], light, camera);
}

// @TODO: 右大臂
void right_upper_arm(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_ARM_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_ARM_WIDTH, robot.UPPER_ARM_HEIGHT, robot.UPPER_ARM_WIDTH));

	RightUpperArm->modelMatrix = modelMatrix * instance;
	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	painter->drawMesh(RightUpperArm, painter->opengl_objects[6], light, camera);
}

// @TODO: 右小臂
void right_lower_arm(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.4 * robot.LOWER_ARM_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_ARM_WIDTH, robot.LOWER_ARM_HEIGHT, robot.LOWER_ARM_WIDTH));

	RightLowerArm->modelMatrix = modelMatrix * instance;
	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	painter->drawMesh(RightLowerArm, painter->opengl_objects[7], light, camera);
}

// @TODO: 左大腿
void left_upper_leg(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_LEG_WIDTH, robot.UPPER_LEG_HEIGHT, robot.UPPER_LEG_WIDTH));

	LeftUpperLeg->modelMatrix = modelMatrix * instance;
	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	painter->drawMesh(LeftUpperLeg, painter->opengl_objects[8], light, camera);
}

// @TODO: 左小腿
void left_lower_leg(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_LEG_WIDTH, robot.LOWER_LEG_HEIGHT, robot.LOWER_LEG_WIDTH));

	LeftLowerLeg->modelMatrix = modelMatrix * instance;
	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	painter->drawMesh(LeftLowerLeg, painter->opengl_objects[9], light, camera);
}

// @TODO: 右大腿
void right_upper_leg(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_LEG_WIDTH, robot.UPPER_LEG_HEIGHT, robot.UPPER_LEG_WIDTH));

	RightUpperLeg->modelMatrix = modelMatrix * instance;
	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	painter->drawMesh(RightUpperLeg, painter->opengl_objects[10], light, camera);
}

// @TODO: 右小腿
void right_lower_leg(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_LEG_WIDTH, robot.LOWER_LEG_HEIGHT, robot.LOWER_LEG_WIDTH));

	RightLowerLeg->modelMatrix = modelMatrix * instance;
	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	painter->drawMesh(RightLowerLeg, painter->opengl_objects[11], light, camera);
}


void display()
{
	painter->drawMeshes(light, camera);


	//绘制机器人
	// 物体的变换矩阵
	glm::mat4 modelMatrix = glm::mat4(1.0);

	// 保持变换矩阵的栈
	MatrixStack mstack;

	// 躯干（这里我们希望机器人的躯干只绕Y轴旋转，所以只计算了RotateY）
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.26, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.Torso]), glm::vec3(0.0, 1.0, 0.0));
	torso(modelMatrix);

	mstack.push(modelMatrix); // 保存躯干变换矩阵
	// 头部（这里我们希望机器人的头部只绕Y轴旋转，所以只计算了RotateY）
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.71 * robot.TORSO_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.Head]), glm::vec3(0.0, 1.0, 0.0));
	head(modelMatrix);
	modelMatrix = mstack.pop(); // 恢复躯干变换矩阵


	// =========== 左臂 ===========
	mstack.push(modelMatrix);   // 保存躯干变换矩阵
	// 左大臂（这里我们希望机器人的左大臂只绕Z轴旋转，所以只计算了RotateZ，后面同理）
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-robot.UPPER_ARM_WIDTH, 0.9 * robot.TORSO_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftUpperArm]), glm::vec3(0.0, 0.0, 1.0));
	left_upper_arm(modelMatrix);

	// @TODO: 左小臂
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -robot.LOWER_ARM_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftLowerArm]), glm::vec3(0.0, 0.0, 1.0));
	left_lower_arm(modelMatrix);
	modelMatrix = mstack.pop(); // 恢复躯干变换矩阵

	// =========== 右臂 ===========
	mstack.push(modelMatrix);   // 保存躯干变换矩阵
	// @TODO: 右大臂
	modelMatrix = glm::translate(modelMatrix, glm::vec3(robot.UPPER_ARM_WIDTH, 0.9 * robot.TORSO_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightUpperArm]), glm::vec3(0.0, 0.0, 1.0));
	right_upper_arm(modelMatrix);

	// @TODO: 右小臂
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -robot.LOWER_ARM_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightLowerArm]), glm::vec3(0.0, 0.0, 1.0));
	right_lower_arm(modelMatrix);
	modelMatrix = mstack.pop(); // 恢复躯干变换矩阵

	// =========== 左腿 ===========
	mstack.push(modelMatrix);   // 保存躯干变换矩阵
	// @TODO: 左大腿
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-robot.UPPER_LEG_WIDTH / 2, 0.7* robot.UPPER_LEG_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftUpperLeg]), glm::vec3(0.0, 0.0, 1.0));
	left_upper_leg(modelMatrix);

	// @TODO: 左小腿
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -robot.LOWER_LEG_HEIGHT / 1.2, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftLowerLeg]), glm::vec3(0.0, 0.0, 1.0));
	left_lower_leg(modelMatrix);
	modelMatrix = mstack.pop(); // 恢复躯干变换矩阵

	// =========== 右腿 ===========
	mstack.push(modelMatrix);   // 保存躯干变换矩阵
	// @TODO: 右大腿
	modelMatrix = glm::translate(modelMatrix, glm::vec3(robot.UPPER_LEG_WIDTH / 2, 0.7* robot.UPPER_LEG_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightUpperLeg]), glm::vec3(0.0, 0.0, 1.0));
	right_upper_leg(modelMatrix);

	// @TODO: 右小腿
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -robot.LOWER_LEG_HEIGHT / 1.2, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightLowerLeg]), glm::vec3(0.0, 0.0, 1.0));
	right_lower_leg(modelMatrix);

	modelMatrix = mstack.pop(); // 恢复躯干变换矩阵
}


void printHelp()
{
	std::cout << "Keyboard and Mouse Usage" << std::endl;
	std::cout <<
		"[Window]" << std::endl <<
		"ESC:       Exit" << std::endl <<
		"h:         Print help message" << std::endl << std::endl <<

		"[Camera]" << std::endl <<
		"Mouse:     Move your mouse to look around\n" <<
		"Scroll:    Zoom the object" << std::endl <<
		"SPACE:     Back to the original position\n" << std::endl;

}

// 键盘响应函数
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	float tmp;
	glm::vec4 ambient;
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_ESCAPE: exit(EXIT_SUCCESS); break;
		case GLFW_KEY_H: printHelp(); break;
		default:
			camera->keyboard(key, action, mode);
			break;
		}
	}
}

// 鼠标响应函数，只要鼠标移动，该函数就会被调用
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	//float yoffset = ypos - lastY;

	// reversed since y-coordinates range from bottom to top
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera->processMouseMovement(xoffset, yoffset);
}

// 滚轮响应函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera->processMouseScroll(yoffset);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera->processKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera->processKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera->processKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera->processKeyboard(RIGHT, deltaTime);
}

void cleanData() {
	// 释放内存

	delete camera;
	camera = NULL;

	delete light;
	light = NULL;

	painter->cleanMeshes();

	delete painter;
	painter = NULL;

	for (int i = 0; i < meshList.size(); i++) {
		delete meshList[i];
	}
	meshList.clear();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main(int argc, char** argv)
{
	// 初始化GLFW库，必须是应用程序调用的第一个GLFW函数
	glfwInit();

	// 配置GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2019152091陆和淇期末大作业", NULL, NULL);

	// 设置窗口属性
/*	void glfwSetWindowMonitor	(	GLFWwindow * 	window,
					GLFWmonitor * 	monitor,
					int 	xpos,
					int 	ypos,
					int 	width,
					int 	height,
					int 	refreshRate
					)	*/
	glfwSetWindowMonitor(window, NULL, 0, 0, SCR_WIDTH, SCR_HEIGHT, GLFW_DONT_CARE);

	//全屏--无窗口
	//GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2019152091", glfwGetPrimaryMonitor(), NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// 让鼠标定于窗口中心，并隐藏光标
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// 调用任何OpenGL的函数之前初始化GLAD
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// 启用深度测试
	glEnable(GL_DEPTH_TEST);

	camera->projMatrix = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

	// Init mesh, shaders, buffer
	init();

	// 输出帮助信息
	printHelp();

	// skybox VAO
	GLuint skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	GLuint cubemapTexture = loadCubemap(faces);

	Shader skyboxShader("./shaders/skybox_vs.glsl", "./shaders/skybox_fs.glsl");
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		display();

		// draw skybox as last
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
		glm::mat4 view = glm::mat4(glm::mat3(camera->getViewMatrix())); // remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", camera->projMatrix);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default

		// 交换颜色缓冲 以及 检查有没有触发什么事件（比如键盘输入、鼠标移动等）
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	cleanData();
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);

	return 0;
}

// 每当窗口改变大小，GLFW会调用这个函数并填充相应的参数供你处理。
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}