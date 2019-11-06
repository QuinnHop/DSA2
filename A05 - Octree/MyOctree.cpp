#include "MyOctree.h"

using namespace Simplex;
//set varous MyOctree variables

uint MyOctree::m_uIdealEntityCount = 5;
uint MyOctree::m_uOctantCount = 0;

Simplex::MyOctree::MyOctree(uint a_nMaxLevel, uint a_nIdealEntityCount)
{

}

MyOctree::MyOctree(vector3 a_v3Center, float a_fSize)
{
}

MyOctree::MyOctree(MyOctree const& other)
{
}

MyOctree& MyOctree::operator=(MyOctree const& other)
{
	// TODO: insert return statement here
}

MyOctree::~MyOctree(void)
{
}

void MyOctree::Swap(MyOctree& other)
{
}

Simplex::MyOctree::MyOctree(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
}

float MyOctree::GetSize(void)
{
	return 0.0f;
}

vector3 MyOctree::GetCenterGlobal(void)
{
	return vector3();
}

vector3 MyOctree::GetMinGlobal(void)
{
	return vector3();
}

vector3 MyOctree::GetMaxGlobal(void)
{
	return vector3();
}

bool MyOctree::IsColliding(uint a_uRBIndex)
{
	return false;
}

void MyOctree::Display(uint a_nIndex, vector3 a_v3Color)
{
}

void MyOctree::Display(vector3 a_v3Color)
{
}

void MyOctree::DisplayLeafs(vector3 a_v3Color)
{
}

void MyOctree::ClearEntityList(void)
{
}

void MyOctree::Subdivide(void)
{
}

MyOctree* MyOctree::GetChild(uint a_nChild)
{
	return nullptr;
}

MyOctree* MyOctree::GetParent(void)
{
	return nullptr;
}

bool MyOctree::IsLeaf(void)
{
	return false;
}

bool MyOctree::ContainsMoreThan(uint a_nEntities)
{
	return false;
}

void MyOctree::KillBranches(void)
{
}

void MyOctree::ConstructTree(uint a_nMaxLevel)
{
}

void MyOctree::AssignIDtoEntity(void)
{
}

uint MyOctree::GetMyOctreeCount(void)
{
	return uint();
}

void MyOctree::Release(void)
{
}

void MyOctree::Init(void)
{
}

void MyOctree::ConstructList(void)
{
}
