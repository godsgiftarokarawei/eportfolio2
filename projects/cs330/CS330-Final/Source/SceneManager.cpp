///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/


/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// Load the necessary meshes
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadPlaneMesh();

	// Load textures
	CreateGLTexture("textures/wood.png", "wood");
	CreateGLTexture("textures/metal.png", "metal");
	CreateGLTexture("textures/brick.png", "brick");

	// Define materials
	OBJECT_MATERIAL woodMaterial;
	woodMaterial.tag = "wood";
	woodMaterial.ambientColor = glm::vec3(0.5f, 0.35f, 0.2f);
	woodMaterial.ambientStrength = 0.5f;
	woodMaterial.diffuseColor = glm::vec3(0.6f, 0.4f, 0.3f);
	woodMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	woodMaterial.shininess = 8.0f;
	m_objectMaterials.push_back(woodMaterial);

	OBJECT_MATERIAL metalMaterial;
	metalMaterial.tag = "metal";
	metalMaterial.ambientColor = glm::vec3(0.3f, 0.3f, 0.3f);
	metalMaterial.ambientStrength = 0.5f;
	metalMaterial.diffuseColor = glm::vec3(0.6f, 0.6f, 0.6f);
	metalMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	metalMaterial.shininess = 32.0f;
	m_objectMaterials.push_back(metalMaterial);

	// Add directional light
	if (m_pShaderManager != nullptr)
	{
		// Define 4 directional lights
		glm::vec3 lightDirections[4] = {
			glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f)),
			glm::normalize(glm::vec3(1.0f, -1.0f, -1.0f)),
			glm::normalize(glm::vec3(-1.0f, -1.0f, 1.0f)),
			glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f))
		};

		glm::vec3 ambientColor(0.2f);
		glm::vec3 diffuseColor(0.6f);
		glm::vec3 specularColor(1.0f);
		float focalStrength = 32.0f;
		float specularIntensity = 0.5f;

		
		for (int i = 0; i < 4; ++i)
		{
			std::string prefix = "lightSources[" + std::to_string(i) + "]";
			m_pShaderManager->setVec3Value((prefix + ".direction").c_str(), lightDirections[i]);
			m_pShaderManager->setVec3Value((prefix + ".ambientColor").c_str(), ambientColor);
			m_pShaderManager->setVec3Value((prefix + ".diffuseColor").c_str(), diffuseColor);
			m_pShaderManager->setVec3Value((prefix + ".specularColor").c_str(), specularColor);
			m_pShaderManager->setFloatValue((prefix + ".focalStrength").c_str(), focalStrength);
			m_pShaderManager->setFloatValue((prefix + ".specularIntensity").c_str(), specularIntensity);
		}
		// Set object color for the shader (you can use material color or a test color)
		m_pShaderManager->setVec3Value("objectColor", glm::vec3(1.0f, 1.0f, 1.0f)); // pure white

		// Set camera/view position (replace with your actual camera position variable)
		m_pShaderManager->setVec3Value("viewPos", glm::vec3(0.0f, 0.0f, 5.0f)); // example position

		

	}

}



/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// ========== LIGHTING SETUP ==========
	m_pShaderManager->setVec3Value("lightSources[0].direction", glm::vec3(-1.0f, -1.0f, -1.0f)); // Key
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", glm::vec3(0.3f));
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", glm::vec3(0.8f));
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", glm::vec3(1.0f));
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.5f);

	m_pShaderManager->setVec3Value("lightSources[1].direction", glm::vec3(1.0f, -1.0f, 0.5f)); // Fill
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", glm::vec3(0.2f));
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", glm::vec3(0.5f));
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", glm::vec3(0.7f));
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 16.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.3f);

	m_pShaderManager->setVec3Value("lightSources[2].direction", glm::vec3(0.0f, -0.5f, 1.0f)); // Back
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", glm::vec3(0.15f));
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", glm::vec3(0.4f));
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", glm::vec3(0.6f));
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 8.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.25f);

	m_pShaderManager->setVec3Value("lightSources[3].direction", glm::vec3(0.0f, -1.0f, 0.0f)); // Overhead
	m_pShaderManager->setVec3Value("lightSources[3].ambientColor", glm::vec3(0.1f));
	m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", glm::vec3(0.3f));
	m_pShaderManager->setVec3Value("lightSources[3].specularColor", glm::vec3(0.4f));
	m_pShaderManager->setFloatValue("lightSources[3].focalStrength", 4.0f);
	m_pShaderManager->setFloatValue("lightSources[3].specularIntensity", 0.2f);

	// Function to quickly apply material from tag
	auto applyMaterial = [&](const std::string& tag)
		{
			for (const auto& mat : m_objectMaterials)
			{
				if (mat.tag == tag)
				{
					SetShaderMaterial(tag);                     // Apply shader uniforms
					m_pShaderManager->setVec3Value("objectColor", mat.diffuseColor); // Set base object color
					return;
				}
			}
		};

	// ========== DESK SURFACE ==========
	SetTransformations({ 10.0f, 0.2f, 6.0f }, 0.0f, 0.0f, 0.0f, { 0.0f, -0.1f, 0.0f });
	SetShaderTexture("wood");
	applyMaterial("wood");
	m_basicMeshes->DrawBoxMesh();

	// ========== MONITOR ==========
	SetTransformations({ 2.0f, 1.2f, 0.1f }, 0.0f, 0.0f, 0.0f, { 0.0f, 1.5f, -1.5f });
	SetShaderTexture("metal");
	applyMaterial("metal");
	m_basicMeshes->DrawBoxMesh();

	// Stand
	SetTransformations({ 0.2f, 0.8f, 0.2f }, 0.0f, 0.0f, 0.0f, { 0.0f, 0.8f, -1.5f });
	m_basicMeshes->DrawBoxMesh();

	// Base
	SetTransformations({ 1.0f, 0.1f, 0.5f }, 0.0f, 0.0f, 0.0f, { 0.0f, 0.35f, -1.5f });
	m_basicMeshes->DrawBoxMesh();

	// ========== LAMP ==========
	SetTransformations({ 0.6f, 0.1f, 0.6f }, 0.0f, 0.0f, 0.0f, { -3.0f, 0.05f, -1.5f });
	m_basicMeshes->DrawBoxMesh();

	SetTransformations({ 0.1f, 1.0f, 0.1f }, 0.0f, 0.0f, 0.0f, { -3.0f, 0.6f, -1.5f });
	m_basicMeshes->DrawBoxMesh();

	SetTransformations({ 0.4f, 0.2f, 0.6f }, -45.0f, 0.0f, 0.0f, { -3.0f, 1.3f, -1.3f });
	m_basicMeshes->DrawBoxMesh();

	// ========== COFFEE MUG ==========
	SetTransformations({ 0.3f, 0.4f, 0.3f }, 0.0f, 0.0f, 0.0f, { 2.5f, 0.2f, -1.5f });
	SetShaderTexture("brick");
	applyMaterial("metal");
	m_basicMeshes->DrawCylinderMesh();

	SetTransformations({ 0.05f, 0.2f, 0.3f }, 0.0f, 0.0f, 0.0f, { 2.8f, 0.2f, -1.5f });
	m_basicMeshes->DrawCylinderMesh();

	// ========== NOTEBOOK ==========
	SetTransformations({ 1.0f, 0.05f, 1.5f }, 0.0f, 0.0f, 0.0f, { -1.5f, 0.05f, -1.5f });
	SetShaderTexture("wood");
	applyMaterial("wood");
	m_basicMeshes->DrawBoxMesh();

	// ========== PEN ==========
	SetTransformations({ 0.05f, 0.05f, 0.8f }, 0.0f, 0.0f, 0.0f, { -1.5f, 0.08f, -1.5f });
	m_pShaderManager->setVec3Value("objectColor", glm::vec3(0.1f)); // Dark gray
	m_basicMeshes->DrawCylinderMesh();

	// ========== WALL BACKDROP ==========
	SetTransformations(
		{ 10.0f, 5.0f, 0.2f },    // width, height, depth
		0.0f,                     // rotation X
		0.0f,                     // rotation Y
		0.0f,                     // rotation Z
		{ 0.0f, 2.5f, -4.0f }     // position X, Y, Z (behind desk)
	);
	SetShaderTexture("brick");
	applyMaterial("brick");
	m_basicMeshes->DrawBoxMesh();


	
	// ========== FLOOR ==========
	SetTransformations(
		{ 20.0f, 0.1f, 20.0f },     // wide floor under everything
		0.0f, 0.0f, 0.0f,
		{ 0.0f, -2.0f, 0.0f }       // much lower below the desk
	);
	SetShaderTexture("wood");
	applyMaterial("wood");
	m_basicMeshes->DrawBoxMesh();



	
	// ========== CEILING ==========
	SetTransformations(
		{ 20.0f, 0.1f, 20.0f },     // same size as floor
		0.0f, 0.0f, 0.0f,
		{ 0.0f, 7.0f, 0.0f }        // high enough above the monitor
	);
	SetShaderTexture("metal");     // or a custom "ceiling" texture
	applyMaterial("metal");
	m_basicMeshes->DrawBoxMesh();


}
