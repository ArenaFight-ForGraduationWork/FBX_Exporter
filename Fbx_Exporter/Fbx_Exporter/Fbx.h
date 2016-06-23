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

	// Ŭ���̾�Ʈ���� �� XMMATRIX�� row-major����̰�, FbxMatrix&FbxAMatrix�� column-major����̶� �־��� �� ��ġ���Ѿ� �Ѵ�
	FbxMatrix *m_pBaseBoneMatrix;
	FbxMatrix **m_ppAnimationMatrix;
	FbxMatrix **m_ppResultMatrix;

	unsigned int m_iAnimationMaxTime;			// �ִϸ��̼� �ִ� ����
	unsigned int m_uiAnimationNodeIndexCount;	//�ִϸ��̼� ��� ����
	float m_fAnimationPlayTime;					//�ִϸ��̼� ��� �ð�

	int m_iVertexSize;

	char *m_pTxtName;
	char *m_pTxtNameAfterMatrix;

	FbxVector4 m_MaxVer;
	FbxVector4 m_MinVer;

	CAnimationVertex *m_pVertices;

	FbxAMatrix _GetGeometryTransformation(FbxNode* inNode);

	void _Initialize(char* pFileName);
	void _WriteVertex();			// _info
	void _WriteMinMaxPos();			// _info �� �Ʒ��� Min/Max �߰�
	void _SetBoneMatrix(FbxNode*);
	void _SetAnimationData(FbxNode*);
	void _WriteWeight();			// _weight
	void _SetAnimationMatrix(FbxNode* pNode, FbxAnimStack *pFbxAS);
	void _WriteAnimationMatrix();	// _matrix
};



void Warning(char* pWarningText);


#endif