#include "MyOctree.h"

using namespace Simplex;

//set some variables for my MyOctree class

uint Simplex::MyOctree::m_uIdealEntityCount = 5;// ideal number of objects in a given subdivision
uint Simplex::MyOctree::m_uOctantCount = 0; //starting number of subdivisions
uint Simplex::MyOctree::m_uMaxLevel = 4; //Max number of subdivisions


Simplex::MyOctree::MyOctree(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	Init();

	m_uOctantCount = 0; //at the time of creation there are no octants
	m_uID = m_uOctantCount;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uMaxLevel = a_nMaxLevel;
	


	m_pRoot = this; //the first node is the root
	m_lChild.clear();

	std::vector<vector3> locMinMax;//holds the local min and max values of the objects' bounding boxes

	//get the objects and add their mins and maxes to the locMinMax array to determine if the objects are colliding
	uint nObjects = m_pEntityMngr->GetEntityCount();
	for (uint i = 0; i < nObjects; i++) {
		MyEntity* pEntity = m_pEntityMngr->GetEntity(i);
		MyRigidBody* pRigidbody = pEntity->GetRigidBody();
		locMinMax.push_back(pRigidbody->GetMinGlobal());
		locMinMax.push_back(pRigidbody->GetMaxGlobal());
	}
	MyRigidBody* pRigidBody = new MyRigidBody(locMinMax);

	vector3 v3HalfWidth = pRigidBody->GetHalfWidth();
	float fMax = v3HalfWidth.x;

	//set the fMax
	for (uint i = 0; i < 3; i++) {
		if (fMax < v3HalfWidth[i]) {
			fMax = v3HalfWidth[i];
		}
	}
	vector3 v3Center = pRigidBody->GetCenterLocal();
	locMinMax.clear();
	SafeDelete(pRigidBody);

	m_fSize = 2.0f* fMax;
	m_v3Center = v3Center;
	m_v3Max = m_v3Center - vector3(fMax);
	m_v3Min = m_v3Center + vector3(fMax);

	m_uOctantCount++;

	ConstructTree(m_uMaxLevel);
}

Simplex::MyOctree::MyOctree(vector3 a_v3Center, float a_fSize)
{
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);
	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);

	m_uOctantCount++;
}

Simplex::MyOctree::MyOctree(MyOctree const& other)
{
	
	m_v3Center = other.m_v3Center;
	m_v3Max = other.m_v3Max;
	m_v3Min = other.m_v3Min;

	m_uChildren = other.m_uChildren;
	m_pParent = other.m_pParent;
	m_pRoot = other.m_pRoot;
	m_lChild = other.m_lChild;

	m_fSize = other.m_fSize;
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	for (uint i = 0; i < 8; i++) {
		m_pChild[i] = other.m_pChild[i];
	}
}

Simplex::MyOctree& Simplex::MyOctree::operator=(MyOctree const& other)
{
	//if the object is not the current Octree
	if (this != &other) {
		Release();
		Init();
		MyOctree temp(other);
		Swap(temp);
	}
	return *this;
}

Simplex::MyOctree::~MyOctree(void)
{
	Release();
}

void Simplex::MyOctree::Swap(MyOctree& other)
{
	//copy over MyOctree info from other into new
	std::swap(m_lChild, other.m_lChild);
	std::swap(m_uChildren, other.m_uChildren);
	//swap children
	for (uint i = 0; i < 8; i++) {
		std::swap(m_pChild[i], other.m_pChild[i]);
	}

	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_pParent, other.m_pParent);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	std::swap(m_uID, other.m_uID);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_fSize, other.m_fSize);

	//create new managers
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();
}

float Simplex::MyOctree::GetSize(void)
{
	return m_fSize;
}

vector3 Simplex::MyOctree::GetCenterGlobal(void)
{
	return m_v3Center;
}

vector3 Simplex::MyOctree::GetMinGlobal(void)
{
	return m_v3Min;
}

vector3 Simplex::MyOctree::GetMaxGlobal(void)
{
	return m_v3Max;
}

bool Simplex::MyOctree::IsColliding(uint a_uRBIndex)
{
	uint numObjs = m_pEntityMngr->GetEntityCount();

	if (a_uRBIndex >= numObjs)
		return false;//there aren't enough objects for there to be a collision

	MyEntity* pEntity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* pRigidBody = pEntity->GetRigidBody();
	vector3 v3EntityMin = pRigidBody -> GetMinGlobal();
	vector3 v3EntityMax = pRigidBody -> GetMaxGlobal();

	//check for x collisions
	if (m_v3Min.x > v3EntityMin.x) {
		return false;
	}
	if (m_v3Max.x < v3EntityMax.x) {
		return false;
	}

	//check for y collisions
	if (m_v3Min.y > v3EntityMin.y) {
		return false;
	}
	if (m_v3Max.y < v3EntityMax.y) {
		return false;
	}

	//check for z collisions
	if (m_v3Min.z > v3EntityMin.z) {
		return false;
	}
	if (m_v3Max.z < v3EntityMax.z) {
		return false;
	}

	//all negative checks failed, so there is a collision
	return true;
}

void Simplex::MyOctree::Display(uint a_nIndex, vector3 a_v3Color)
{
	if (m_uID == a_nIndex) {
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center)* glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
		return;
	}
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->Display(a_nIndex);
	}
}

void Simplex::MyOctree::Display(vector3 a_v3Color)
{
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->Display(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void Simplex::MyOctree::DisplayLeafs(vector3 a_v3Color)
{
	//loops through each child in lChild and renders it
	uint nLeaves = m_lChild.size();
	for (uint i = 0; i < nLeaves; i++) {
		m_lChild[i]->DisplayLeafs(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);

}

void Simplex::MyOctree::ClearEntityList(void)
{
	//calls clearEntityList for each child in the list of children
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->ClearEntityList();
	}
}

void Simplex::MyOctree::Subdivide(void)
{
	if (m_uLevel >= m_uMaxLevel) {
		return;//subdivision has already reached bottom of tree
	}
	if (m_uChildren != 0) {
		return;//subdivision doesn't need to be divided
	}

	m_uChildren = 8; //since this is an octree it will always have 8 children

	float fNewSize = m_fSize / 4.0f;
	float fDoubleNewSize = fNewSize * 2.0f;
	vector3 v3NewCenter;

	//the 8 subdivisions that are created
	//subdivision 1 Bottom Left Back
	v3NewCenter = m_v3Center;
	v3NewCenter.x -= fNewSize;
	v3NewCenter.y -= fNewSize;
	v3NewCenter.z -= fNewSize;
	m_pChild[0] = new MyOctree(v3NewCenter, fDoubleNewSize);

	//subdivision 2 Bottom Right Back
	v3NewCenter.x += fDoubleNewSize;
	m_pChild[1] = new MyOctree(v3NewCenter, fDoubleNewSize);

	//subdivision 3 Bottom Right Front
	v3NewCenter.z += fDoubleNewSize;
	m_pChild[2] = new MyOctree(v3NewCenter, fDoubleNewSize);

	//subdivision 4 Bottom Left Front
	v3NewCenter.x -= fDoubleNewSize;
	m_pChild[3] = new MyOctree(v3NewCenter, fDoubleNewSize);

	//subdivision 5 Top Left Front
	v3NewCenter.y += fDoubleNewSize;
	m_pChild[4] = new MyOctree(v3NewCenter, fDoubleNewSize);

	//subdivision 6 Top Left Back
	v3NewCenter.z -= fDoubleNewSize;
	m_pChild[5] = new MyOctree(v3NewCenter, fDoubleNewSize);

	//subdivision 7 Top Right Back
	v3NewCenter.x += fDoubleNewSize;
	m_pChild[6] = new MyOctree(v3NewCenter, fDoubleNewSize);

	//subdivision 7 Top Right Front
	v3NewCenter.z += fDoubleNewSize;
	m_pChild[7] = new MyOctree(v3NewCenter, fDoubleNewSize);

	//pass info to the children and further subdivide if necessary
	for (uint i = 0; i < 8; i++) {
		m_pChild[i]->m_pRoot = m_pRoot;
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel + 1;
		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount)) {
			m_pChild[i]->Subdivide();
		}
	}
}

Simplex::MyOctree* Simplex::MyOctree::GetChild(uint a_nChild)
{
	if (a_nChild > 7) {
		return nullptr;
	}
	return m_pChild[a_nChild];
}

Simplex::MyOctree* Simplex::MyOctree::GetParent(void)
{
	return m_pParent;
}

bool Simplex::MyOctree::IsLeaf(void)
{
	if (m_uChildren == 0) return true;
	else return false;
}

bool Simplex::MyOctree::ContainsMoreThan(uint a_nEntities)
{
	uint currCount = 0;//count of objects in current octant
	uint objectCount = m_pEntityMngr->GetEntityCount();
	for (uint i = 0; i < objectCount; i++) {
		if (IsColliding(i))//if the number of objects in the octant is greater than the number passed in, return true
			currCount++;
		if (currCount > a_nEntities)
			return true;
	}
	return false;
}

void Simplex::MyOctree::KillBranches(void)
{
	//recursivley loops through the children up the tree as each set of children is set to nullptr
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->KillBranches();
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}
	m_uChildren = 0;
}

void Simplex::MyOctree::ConstructTree(uint a_nMaxLevel)
{
	if (m_uLevel != 0)
		return;

	//set basic info
	m_uMaxLevel = a_nMaxLevel;
	m_uOctantCount = 1;

	//clear tree incase there is previous data
	m_EntityList.clear();
	KillBranches();
	m_lChild.clear();

	//if it needs to subdivide, do so
	if (ContainsMoreThan(m_uIdealEntityCount)) {
		Subdivide();
	}

	//assign container ID and create list
	AssignIDtoEntity();
	ConstructList();
}

void Simplex::MyOctree::AssignIDtoEntity(void)
{
	for (uint uChild = 0; uChild < m_uChildren; uChild++) {
		m_pChild[uChild]->AssignIDtoEntity();
	}
	//if you reach a leaf
	if (m_uChildren == 0) {
		uint numEntities = m_pEntityMngr->GetEntityCount();
		for (uint i = 0; i < numEntities; i++) {
			if (IsColliding(i)) {
				m_EntityList.push_back(i);
				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}
}

uint Simplex::MyOctree::GetOctantCount(void)
{
	return m_uOctantCount;
}

void Simplex::MyOctree::Release(void)
{
	//root release
	if (m_uLevel == 0) {
		KillBranches();
	}
	m_uChildren = 0;
	m_fSize = 0.0f;
	m_EntityList.clear();
	m_lChild.clear();
}

//sets initial values of MyOctree
void Simplex::MyOctree::Init(void)
{
	m_uChildren = 0; //no children
	m_fSize = 0.0f; //initial size  = 0

	m_uID = m_uOctantCount; //the id of the first octant is 0
	m_uLevel = 0; //no subdivisions, so level is 0

	//no max.min/center values have been added
	m_v3Center = vector3(0.0f);
	m_v3Max = vector3(0.0f);
	m_v3Min = vector3(0.0f);

	m_pEntityMngr = MyEntityManager::GetInstance();
	m_pMeshMngr = MeshManager::GetInstance();
	

	m_pRoot = nullptr;
	m_pParent = nullptr;

	//parent node has 8 subdivisions, create the children
	for (uint i = 0; i < 8; i++) {
		m_pChild[i] = nullptr;
	}

}

void Simplex::MyOctree::ConstructList(void)
{
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->ConstructList();
	}
	if(m_EntityList.size() > 0) {
		m_pRoot->m_lChild.push_back(this);
	}
}
