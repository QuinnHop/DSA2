#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	//calculate internal degree for each triangle
	float fDegree = (2 * PI) / a_nSubdivisions;

	//offset is used to make the centerpoint in the middle of the shape
	float offset = -a_fHeight / 2;
	vector3 vCenter = vector3(0.0f, offset, 0.0f);

	//save previous third point to use later as second point
	vector3 vPreviousThirdPoint = vCenter;
	vector3 vTop = vector3(0.0f, a_fHeight+offset, 0.0f);//the top of the cone
	for (int i = 0; i < a_nSubdivisions; i++) {

		float fXValue;
		float fZValue;
		vector3 vSecondPoint;
		vector3 vThirdPoint;
		
		//if the previous second point is equal to the center point then this is the first itteration,
		//and the previous second point must be set
		if (vPreviousThirdPoint == vCenter) {
			//center is always point 1 so we must calculate points 2 and 3
			//xy for point 2
			fXValue = a_fRadius * cos(fDegree * i);
			fZValue = a_fRadius * sin(fDegree * i);

			vSecondPoint = vector3(fXValue, offset, fZValue);
		}
		else {//previous second point has already been set
			vSecondPoint = vPreviousThirdPoint;
		}

		//xy for point 3
		fXValue = a_fRadius * cos(fDegree * (i + 1));
		fZValue = a_fRadius * sin(fDegree * (i + 1));

		vThirdPoint = vector3(fXValue, offset, fZValue);
		vPreviousThirdPoint = vThirdPoint;//sets this to save repeating it during the next itteration

		//draw the triangle
		AddTri(vCenter, vThirdPoint, vSecondPoint);//draws the triangle for base with correct direction
		AddTri(vTop, vSecondPoint, vThirdPoint);//draws triangle for height
	}





	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	//for generateCylinder we will create two circles and use the GenerateQuad method to attach the two circles
	//calculate internal degree for each triangle
	float fDegree = (2 * PI) / a_nSubdivisions;
	float offset = -a_fHeight / 2;

	vector3 vCenter = vector3(0.0f, offset, 0.0f);
	vector3 vPreviousThirdPoint = vCenter;
	
	for (int i = 0; i < a_nSubdivisions; i++) {

		float fXValue;
		float fZValue;
		vector3 vSecondPoint;
		vector3 vThirdPoint;

		//if the previous second point is equal to the center point then this is the first itteration,
		//and the previous second point must be set
		if (vPreviousThirdPoint == vCenter) {
			//center is always point 1 so we must calculate points 2 and 3
			//xy for point 2
			fXValue = a_fRadius * cos(fDegree * i);
			fZValue = a_fRadius * sin(fDegree * i);

			vSecondPoint = vector3(fXValue, offset, fZValue);
		}
		else {//previous second point has already been set
			vSecondPoint = vPreviousThirdPoint;
		}
		
		//xy for point 3
		fXValue = a_fRadius * cos(fDegree * (i + 1));
		fZValue = a_fRadius * sin(fDegree * (i + 1));

		vThirdPoint = vector3(fXValue, offset, fZValue);
		vPreviousThirdPoint = vThirdPoint;//sets this to save repeating it during the next itteration

		//draw the triangles for first circle
		
		//TODO: Fix this
		AddTri(vCenter, vSecondPoint, vThirdPoint);//draws the triangle for base with correct direction for bottom circle
		//draws circle for top of cylinder
		AddTri(vector3(vCenter.x, offset + a_fHeight, vCenter.z),   vector3(vThirdPoint.x, offset + a_fHeight, vThirdPoint.z), vector3(vSecondPoint.x, offset + a_fHeight, vSecondPoint.z));
		
		//Draws quads to connect top and bottom of cylinder
		AddQuad(  vThirdPoint, vSecondPoint, vector3(vThirdPoint.x, offset + a_fHeight, vThirdPoint.z), vector3(vSecondPoint.x, offset + a_fHeight, vSecondPoint.z));
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	//this function has the z and y switched correctly
	float fDegree = (2 * PI) / a_nSubdivisions;
	float offset = -a_fHeight / 2;

	
	vector3 vCenter = vector3(0.0f, offset, 0.0f);
	vector3 vInnerPreviousThirdPoint = vCenter;
	vector3 vOuterPreviousThirdPoint = vCenter;
	
	for (int i = 0; i < a_nSubdivisions; i++) {
		float fInnerValueX;
		float fInnerValueZ;
		float fOuterValueX;
		float fOuterValueZ;
		vector3 vInnerSecondPoint;
		vector3 vOuterSecondPoint;
		vector3 vInnerThirdPoint;
		vector3 vOuterThirdPoint;
		

		//if the previous second point is equal to the center point then this is the first itteration,
		//and the previous second point must be set
		if (vInnerPreviousThirdPoint == vCenter && vOuterPreviousThirdPoint == vCenter) {

			//center is always point 1 so we must calculate points 2 and 3
				//xz for point 2
			fInnerValueX = a_fInnerRadius * cos(fDegree * i);
			fInnerValueZ = a_fInnerRadius * sin(fDegree * i);

			fOuterValueX = a_fOuterRadius * cos(fDegree * i);
			fOuterValueZ = a_fOuterRadius * sin(fDegree * i);

			vInnerSecondPoint = vector3(fInnerValueX, offset, fInnerValueZ);
			vOuterSecondPoint = vector3(fOuterValueX, offset, fOuterValueZ);
		}
		else {//previous second point has already been set
			vInnerSecondPoint = vInnerPreviousThirdPoint;
			vOuterSecondPoint = vOuterPreviousThirdPoint;
		}
		
		//settting coordinates for inner and outer points
		fInnerValueX = a_fInnerRadius * cos(fDegree * (i + 1));
		fInnerValueZ = a_fInnerRadius * sin(fDegree * (i + 1));
		fOuterValueX = a_fOuterRadius * cos(fDegree * (i + 1));
		fOuterValueZ = a_fOuterRadius * sin(fDegree * (i + 1));

		vInnerThirdPoint = vector3(fInnerValueX, offset, fInnerValueZ);
		vOuterThirdPoint = vector3(fOuterValueX, offset, fOuterValueZ);
		vInnerPreviousThirdPoint = vInnerThirdPoint;//sets this to save repeating it during the next itteration
		vOuterPreviousThirdPoint = vOuterThirdPoint;

		


		//Make new vectors lower to represent the bottom of the shape
		vector3 vOuterSecondPointBottom = vector3(vOuterSecondPoint.x, offset + a_fHeight, vOuterSecondPoint.z);
		vector3 vOuterThirdPointBottom = vector3(vOuterThirdPoint.x, offset + a_fHeight, vOuterThirdPoint.z);
		vector3 vInnerSecondPointBottom = vector3(vInnerSecondPoint.x, offset + a_fHeight, vInnerSecondPoint.z);
		vector3 vInnerThirdPointBottom = vector3(vInnerThirdPoint.x, offset + a_fHeight, vInnerThirdPoint.z);


		//draw quads connecting points

		//add bottom of tube
		AddQuad(vOuterThirdPointBottom, vOuterSecondPointBottom, vInnerThirdPointBottom, vInnerSecondPointBottom);
		//across top of tube
		AddQuad(vInnerThirdPoint, vInnerSecondPoint, vOuterThirdPoint, vOuterSecondPoint);

		//add sides of cube
		AddQuad( vOuterSecondPointBottom, vOuterThirdPointBottom, vOuterSecondPoint, vOuterThirdPoint);
		AddQuad(vInnerSecondPoint, vInnerThirdPoint, vInnerSecondPointBottom, vInnerThirdPointBottom);
	}
	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	



	Release();
	Init();

	//The sphere is constructed by forming a cube then normalizing all the points and scaling them by the radius


	//variables
	std::vector<std::vector<vector3>> rings;//stores vector of ring vectors, eacho of which holds the coordinates for the points in its ring
	vector3 center(0.0f, -a_fRadius/2, 0.0f);



	for (int i = 0; i < a_nSubdivisions; i++) {
	
		
		float fVerticalDegree = PI / 2 - (i*(PI / a_nSubdivisions));//vertical angle of a point on the ring
		std::vector<vector3> ring;//defined in the for loop, this holds the information of a single ring
		float radiusOfRing = a_fRadius * cosf(fVerticalDegree);//actual radius scaled to correct position
		//defines the points in each ring and adds them to the rings vector
		for (int j = 0; j < a_nSubdivisions+1; j++) {
			float fHorizontalDegree = (j * ((2 * PI) / a_nSubdivisions));//horizontal angle of the point on the ring
			vector3 point = vector3(cos(fHorizontalDegree)*radiusOfRing, a_fRadius*sinf(fVerticalDegree), sin(fHorizontalDegree)*radiusOfRing);
			ring.push_back(point);//adds point to the ring
			
		}
		rings.push_back(ring);

	}

	//convert points in rings to quads and tris 
	for (int i = 0; i < a_nSubdivisions-1; i++) {

		for (int j = 0; j < a_nSubdivisions; j++) {
			if (i == 0) {//the first subsection has triangles instead of quads

				vector3 bottom = center;
				bottom.y = a_fRadius;

				if(j < a_nSubdivisions)
				AddTri(bottom, rings[i + 1][j], rings[i + 1][j + 1]);
			}

			else if(j < a_nSubdivisions){
				//the quads are part of the body, and therefore are rendered using quads
				AddQuad(rings[i][j], rings[i + 1][j], rings[i][j + 1], rings[i + 1][j + 1]);
				
			}
		}
	}
	for (int j = 0; j < a_nSubdivisions; j++)
	 {
		//top of the shape is rendered using tris
		vector3 top = center;
		top.y = -a_fRadius;

		if (j < a_nSubdivisions)
			AddTri(top,   rings[a_nSubdivisions - 1][j+1], rings[a_nSubdivisions - 1][j]);
	}
	
	
	
	
	
	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}