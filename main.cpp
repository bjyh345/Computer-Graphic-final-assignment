#include "Angel.h"
#include "TriMesh.h"
#include "MeshPainter.h"
#include <learnopengl/camera.h>
#include <learnopengl/shader.h>
#include <stb_image.h>
#include <vector>
#include <string>

// 窗口大小
const GLuint SCR_WIDTH = 800;
const GLuint SCR_HEIGHT = 800;

//初始让鼠标位于窗口中心
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

int mainWindow;

//鼠标从外面进入窗口,设为true
bool firstMouse = true;

Camera* camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));
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
	light->setTranslation(glm::vec3(1.0, 15.0, 15.0));
	light->setAmbient(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 环境光
	light->setDiffuse(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 漫反射
	light->setSpecular(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 镜面反射
	light->setAttenuation(1.0, 0.045, 0.0075); // 衰减系数

	TriMesh* table = new TriMesh();

	// @TODO: Task2 读取桌子模型
	table->setNormalize(true);
	table->readObj("./assets/table.obj");

	// 设置物体的旋转位移
	table->setTranslation(glm::vec3(-0.5, 0.2, 0.0));
	table->setRotation(glm::vec3(-90.0, 0.0, 0.0));
	table->setScale(glm::vec3(1.0, 1.0, 1.0));

	// 设置材质
	table->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	table->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	table->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	table->setShininess(1.0); //高光系数

	// 加到painter中
	painter->addMesh(table, "table_a", "./assets/table.png", vshader, fshader); 	// 指定纹理与着色器

	TriMesh* wawa = new TriMesh();

	// @TODO: Task2 读取娃娃模型
	wawa->setNormalize(true);
	wawa->readObj("./assets/wawa.obj");

	// 设置物体的旋转位移
	wawa->setTranslation(glm::vec3(0.5, 0.3, 0.0));
	wawa->setRotation(glm::vec3(-90.0, 0.0, 0.0));
	wawa->setScale(glm::vec3(1.0, 1.0, 1.0));

	// 设置材质
	wawa->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	wawa->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	wawa->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	wawa->setShininess(1.0); //高光系数

	// 加到painter中
	painter->addMesh(wawa, "wawa_a", "./assets/wawa.png", vshader, fshader); 	// 指定纹理与着色器

	glClearColor(1.0, 1.0, 1.0, 1.0);
	// glClearColor(0.0, 0.0, 0.0, 1.0);
}


//true: 正交投影，false: 透视投影
bool isOrtho = true;

void display()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	painter->drawMeshes(light, camera);

	//glutSwapBuffers();
}


void printHelp()
{
	std::cout << "Keyboard Usage" << std::endl;
	std::cout <<
		"[Window]" << std::endl <<
		"ESC:		Exit" << std::endl <<
		"h:			Print help message" << std::endl << std::endl <<
		"[Camera]" << std::endl <<
		"Cursor:    Move your cursor to look around\n" <<
		"Scroll:	Zoom the object" << std::endl <<
		"SPACE:		Reset camera parameters" << std::endl;

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

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// 配置窗口属性
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2019152091", NULL, NULL);
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
		display();
		//reshape();

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