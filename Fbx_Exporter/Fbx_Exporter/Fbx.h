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

private:
	FbxScene* m_pFbxScene;
	FbxNode* m_pRoot;

	map<unsigned int, vector<unsigned int>> m_VertexByIndex;
	std::map<std::string, unsigned int> m_IndexByName;

	FbxMatrix *m_pBaseBoneMatrix;
	FbxMatrix **m_pAnimationMatrix;
	FbxMatrix **m_ppResultMatrix;
	
	int m_iAnimationMaxTime;		// 애니메이션 최대 길이
	unsigned int m_uiAnimationNodeIndexCount;	//애니메이션 노드 갯수
	float m_fAnimationPlayTime;				//애니메이션 재생 시간

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