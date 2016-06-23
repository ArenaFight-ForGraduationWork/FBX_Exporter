#ifndef FBX_H_
#define FBX_H_

#include "stdafx.h"
#include "Vertex.h"



class CFbx
{
public:
	CFbx();
	~CFbx();

	void Import(char* pFileName, char* pTxtName, char* pTxtNameAfterMatrix = nullptr);
	void Destroy();

private:
	FbxScene* m_pFbxScene;
	FbxNode* m_pRoot;

	map<unsigned int, vector<unsigned int>> m_VertexByIndex;
	map<string, unsigned int> m_IndexByName;

	// 클라이언트에서 쓸 XMMATRIX가 row-major행렬이고, FbxMatrix&FbxAMatrix는 column-major행렬이라 넣어줄 때 전치시켜야 한다
	FbxMatrix *m_pBaseBoneMatrix;
	FbxMatrix **m_ppAnimationMatrix;
	FbxMatrix **m_ppResultMatrix;

	unsigned int m_iAnimationMaxTime;			// 애니메이션 최대 길이
	unsigned int m_uiAnimationNodeIndexCount;	//애니메이션 노드 갯수
	float m_fAnimationPlayTime;					//애니메이션 재생 시간

	int m_iVertexSize;

	char *m_pTxtName;
	char *m_pTxtNameAfterMatrix;

	FbxVector4 m_MaxVer;
	FbxVector4 m_MinVer;

	CAnimationVertex *m_pVertices;

	FbxAMatrix _GetGeometryTransformation(FbxNode* inNode);

	void _Initialize(char* pFileName);
	void _WriteVertex();			// _info
	void _WriteMinMaxPos();			// _info 맨 아래에 Min/Max 추가
	void _SetBoneMatrix(FbxNode*);
	void _SetAnimationData(FbxNode*);
	void _WriteWeight();			// _weight
	void _SetAnimationMatrix(FbxNode* pNode, FbxAnimStack *pFbxAS);
	void _WriteAnimationMatrix();	// _matrix
};



void Warning(char* pWarningText);


#endif