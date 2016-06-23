#include "stdafx.h"
#include "Fbx.h"

#define AnimationNodeCount 40	// 애니메이션에 영향을 받는 animation Node 개수
// m_uiAnimationNodeIndexCount = m_IndexByName.size();
// _matrix파일 맨 윗줄 두번째 인자

CFbx::CFbx()
{
	m_pFbxScene = nullptr;
	m_pRoot = nullptr;

	m_VertexByIndex.clear();
	m_IndexByName.clear();

	m_pBaseBoneMatrix = new FbxMatrix[AnimationNodeCount];
	m_ppAnimationMatrix = nullptr;
	m_ppResultMatrix = nullptr;

	m_iAnimationMaxTime = 0;
	m_uiAnimationNodeIndexCount = 0;
	m_fAnimationPlayTime = 0.0f;

	m_iVertexSize = 0;

	m_pTxtName = nullptr;
	m_pTxtNameAfterMatrix = nullptr;

	m_MaxVer = FbxVector4(0, 0, 0, 0);
	m_MinVer = FbxVector4(0, 0, 0, 0);

	m_pVertices = nullptr;
}
CFbx::~CFbx()
{
	Destroy();
}
void CFbx::Destroy()
{
	m_pFbxScene->Destroy();

	for (unsigned int i = 0; i < m_VertexByIndex.size(); ++i)
	{
		m_VertexByIndex[i].clear();
	}
	m_VertexByIndex.clear();
	m_IndexByName.clear();

	if (m_pBaseBoneMatrix)
		delete[] m_pBaseBoneMatrix;
	if (m_ppAnimationMatrix)
	{
		for (unsigned int i = 0; i < m_iAnimationMaxTime / 10; ++i)
			delete[] m_ppAnimationMatrix[i];
	}
	if (m_ppResultMatrix)
	{
		for (unsigned int i = 0; i < m_iAnimationMaxTime / 10; ++i)
			delete[] m_ppResultMatrix[i];
	}

	m_pTxtName = nullptr;
	m_pTxtNameAfterMatrix = nullptr;

	if (m_pVertices)
		delete[] m_pVertices;
}

void CFbx::Import(char* pFileName, char* pTxtName, char* pTxtNameAfterMatrix)
{
	m_pTxtName = pTxtName;
	m_pTxtNameAfterMatrix = pTxtNameAfterMatrix;

	_Initialize(pFileName);

	if (!m_pTxtNameAfterMatrix)
	{	// 애니메이션이 없음
		_WriteVertex();
	}
	else
	{ // 애니메이션이 있음
		_WriteVertex();	// _info
		_WriteMinMaxPos();	// _info에 min/max 덮어쓰기(추가)

		_SetBoneMatrix(m_pRoot);
		_SetAnimationData(m_pRoot);

		_WriteWeight();	// _weight
		_WriteAnimationMatrix();	// _matrix
	}
}



FbxAMatrix CFbx::_GetGeometryTransformation(FbxNode* inNode)
{
	if (!inNode)
	{
		Warning("Null for mesh geometry");
	}

	const FbxVector4 lT = inNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 lR = inNode->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 lS = inNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(lT, lR, lS);
}

void CFbx::_Initialize(char* pFileName)
{
	FbxManager* m_pFbxManager = FbxManager::Create();
	FbxIOSettings* pFbxIOsetting = FbxIOSettings::Create(m_pFbxManager, IOSROOT);
	m_pFbxManager->SetIOSettings(pFbxIOsetting);
	m_pFbxScene = FbxScene::Create(m_pFbxManager, "");
	FbxImporter* pImporter = FbxImporter::Create(m_pFbxManager, "");

	if (!pImporter->Initialize(pFileName, -1, m_pFbxManager->GetIOSettings()))
	{
		Warning("FBX importer Initialize fail\n파일이 지정한 경로나 위치에 없을 수도 있습니다.");
	}

	if (!pImporter->Import(m_pFbxScene))
	{
		Warning("FBX importer Import Scene fail");
	}

	FbxAxisSystem CurrAxisSystem = m_pFbxScene->GetGlobalSettings().GetAxisSystem();	//현재 축 형태
	FbxAxisSystem DestAxisSystem = FbxAxisSystem::eMayaYUp;								//원하는 축 형태
	if (CurrAxisSystem != DestAxisSystem) DestAxisSystem.ConvertScene(m_pFbxScene);

	FbxGeometryConverter lGeomConverter(m_pFbxManager);
	if (!lGeomConverter.Triangulate(m_pFbxScene, true))
	{
		Warning("Mesh transform fail");
	}

	pImporter->Destroy();

	m_pRoot = m_pFbxScene->GetRootNode();
}

void CFbx::_WriteVertex()
{
	FILE *fp;
	char pPath[50] = "";
	strcat_s(pPath, 50, m_pTxtName);
	strcat_s(pPath, 50, "_info.txt");
	fopen_s(&fp, pPath, "wt");

	for (int i = 0; i < m_pRoot->GetChildCount(); ++i)
	{
		FbxNode* pFbxChildNode = m_pRoot->GetChild(i);
		if (pFbxChildNode->GetNodeAttribute() == NULL)
			continue;
		FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();
		if (AttributeType != FbxNodeAttribute::eMesh)
			continue;

		FbxMesh* pMesh = (FbxMesh*)pFbxChildNode->GetNodeAttribute();

		const unsigned int ctrlPointCnt = pMesh->GetControlPointsCount();
		const unsigned int polygonCnt = pMesh->GetPolygonCount();
		m_iVertexSize = pMesh->GetPolygonVertexCount();
		m_pVertices = new CAnimationVertex[m_iVertexSize];
		fprintf(fp, "%d\n", m_iVertexSize);

		FbxVector4 outPos;
		FbxVector4 outNormal;
		FbxVector2 outUV;

		FbxVector4* mControlPoint = pMesh->GetControlPoints();
		FbxGeometryElementUV* vertexUV = pMesh->GetElementUV(0);
		FbxGeometryElementNormal* vertexNormal = pMesh->GetElementNormal(0);

		//정점 데이터 얻기
		for (int j = 0; j < pMesh->GetPolygonCount(); j++)
		{
			int iNumVertieces = pMesh->GetPolygonSize(j);
			if (iNumVertieces != 3)
			{
				Warning("폴리곤이 삼각형이 아니다");
			}
			else //3개로 잘 나뉘어있으면
			{
				for (int k = 0; k < iNumVertieces; k++)
				{
					int iPolygonVertexNum = pMesh->GetPolygonVertex(j, k);
					int iNormalIndex = pMesh->GetPolygonVertexNormal(j, k, outNormal);
					int iTextureUVIndex = pMesh->GetTextureUVIndex(j, k);

					// position
					outPos.mData[0] = mControlPoint[iPolygonVertexNum].mData[0];
					outPos.mData[1] = mControlPoint[iPolygonVertexNum].mData[1];
					outPos.mData[2] = mControlPoint[iPolygonVertexNum].mData[2];
					fprintf(fp, "%f %f %f\n", outPos.mData[0], outPos.mData[1], outPos.mData[2]);

					// normal
					fprintf(fp, "%f %f %f\n", outNormal.mData[0], outNormal.mData[1], outNormal.mData[2]);

					// uv
					switch (vertexUV->GetMappingMode())
					{
					case FbxGeometryElement::eByControlPoint:
						switch (vertexUV->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
						{
							outUV.mData[0] = vertexUV->GetDirectArray().GetAt(iPolygonVertexNum).mData[0];
							outUV.mData[1] = 1 - vertexUV->GetDirectArray().GetAt(iPolygonVertexNum).mData[1];
						}break;
						case FbxGeometryElement::eIndexToDirect:
						{
							int index = vertexUV->GetIndexArray().GetAt(iPolygonVertexNum);
							outUV.mData[0] = vertexUV->GetDirectArray().GetAt(index).mData[0];
							outUV.mData[1] = 1 - vertexUV->GetDirectArray().GetAt(index).mData[1];
						}break;
						default:
							Warning("UV Invalid Reference");
						}break;
					case FbxGeometryElement::eByPolygonVertex:
						switch (vertexUV->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
						case FbxGeometryElement::eIndexToDirect:
						{
							outUV.mData[0] = vertexUV->GetDirectArray().GetAt(iTextureUVIndex).mData[0];
							outUV.mData[1] = 1 - vertexUV->GetDirectArray().GetAt(iTextureUVIndex).mData[1];
						}break;
						default:
							Warning("UV Invalid Reference");
						}break;
					default:
						Warning("UV Invalid Reference");
					}
					fprintf(fp, "%f %f\n", outUV.mData[0], outUV.mData[1]);
				}
			}
		}
	}

	fclose(fp);
}

void CFbx::_WriteMinMaxPos()
{
	FILE *fp;
	char pPath[50] = "";
	strcat_s(pPath, 50, m_pTxtName);
	strcat_s(pPath, 50, "_info.txt");
	fopen_s(&fp, pPath, "a+");

	for (int i = 0; i < m_pRoot->GetChildCount(); ++i)
	{
		FbxNode* pFbxChildNode = m_pRoot->GetChild(i);
		if (pFbxChildNode->GetNodeAttribute() == NULL)
			continue;
		FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();
		if (AttributeType != FbxNodeAttribute::eMesh)
			continue;
		FbxMesh* pMesh = (FbxMesh*)pFbxChildNode->GetNodeAttribute();
		FbxVector4* mControlPoints = pMesh->GetControlPoints();

		//max
		for (int j = 0; j < pMesh->GetControlPointsCount(); ++j)
		{
			if (m_MaxVer.mData[0] < mControlPoints[j].mData[0])
				m_MaxVer.mData[0] = mControlPoints[j].mData[0];

			if (m_MaxVer.mData[1] < mControlPoints[j].mData[2])
				m_MaxVer.mData[1] = mControlPoints[j].mData[2];

			if (m_MaxVer.mData[2] < mControlPoints[j].mData[1])
				m_MaxVer.mData[2] = mControlPoints[j].mData[1];
		}
		fprintf(fp, "%f %f %f\n", m_MaxVer.mData[0], m_MaxVer.mData[1], m_MaxVer.mData[2]);

		// min
		for (int j = 0; j < pMesh->GetControlPointsCount(); ++j)
		{
			if (m_MinVer.mData[0] > mControlPoints[j].mData[0])
				m_MinVer.mData[0] = mControlPoints[j].mData[0];

			if (m_MinVer.mData[1] > mControlPoints[j].mData[2])
				m_MinVer.mData[1] = mControlPoints[j].mData[2];

			if (m_MinVer.mData[2] > mControlPoints[j].mData[1])
				m_MinVer.mData[2] = mControlPoints[j].mData[1];
		}
		fprintf(fp, "%f %f %f\n", m_MinVer.mData[0], m_MinVer.mData[1], m_MinVer.mData[2]);
	}

	fclose(fp);
}

void CFbx::_WriteWeight()
{
	FILE *fp;
	char pPath[50] = "";
	strcat_s(pPath, 50, m_pTxtName);
	strcat_s(pPath, 50, "_weight.txt");
	fopen_s(&fp, pPath, "wt");

	for (int i = 0; i < m_iVertexSize; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			fprintf(fp, "%d %d %f\n", i, m_pVertices[i].GetBoneIndexArray(j), m_pVertices[i].GetBoneWeightArray(j));
		}
	}

	fclose(fp);
}

void CFbx::_WriteAnimationMatrix()
{
	if (!m_pTxtNameAfterMatrix)
	{
		Warning("_matrix 파일 뒤에 적을 이름을 Initialize함수에 안 적었다. 적어라");
	}

	FILE *fp;
	char pPath[50] = "";
	strcat_s(pPath, 50, m_pTxtName);
	strcat_s(pPath, 50, "_matrix_");
	strcat_s(pPath, 50, m_pTxtNameAfterMatrix);
	strcat_s(pPath, 50, ".txt");
	fopen_s(&fp, pPath, "wt");

	FbxAnimStack* AnimStack = m_pFbxScene->GetSrcObject<FbxAnimStack>();
	if (AnimStack)
	{
		//애니메이션 최대길이
		m_iAnimationMaxTime = static_cast<int>(AnimStack->GetLocalTimeSpan().GetDuration().GetMilliSeconds());

		//애니메이션에 영향을 받는 animation Node 개수
		m_uiAnimationNodeIndexCount = m_IndexByName.size();

		//애니메이션 2차원 배열 생성
		m_ppAnimationMatrix = new FbxMatrix*[m_iAnimationMaxTime / 10];	//최대시간/10만큼 애니메이션행렬 배열 할당
		m_ppResultMatrix = new FbxMatrix*[m_iAnimationMaxTime / 10];	//최대시간/10만큼 애니메이션 최종 변환행렬 배열 할당

		for (unsigned int i = 0; i < m_iAnimationMaxTime / 10; ++i)
		{
			m_ppAnimationMatrix[i] = new FbxMatrix[m_uiAnimationNodeIndexCount];	//i번째 시간대 : 배열에 애니메이션 노드 개수만큼 배열 할당
			m_ppResultMatrix[i] = new FbxMatrix[m_uiAnimationNodeIndexCount];	//i번째 시간대 : 최종변환행렬에 애니메이션 노드 개수만큼 배열 할당
		}

		//Animation Matrix채우기
		_SetAnimationMatrix(m_pRoot, AnimStack);

		fprintf(fp, "%d %d\n", m_iAnimationMaxTime, m_uiAnimationNodeIndexCount);

		for (unsigned int i = 0; i < m_iAnimationMaxTime / 10; ++i)
		{
			for (unsigned int j = 0; j < m_uiAnimationNodeIndexCount; ++j)
			{
				m_ppResultMatrix[i][j] = m_pBaseBoneMatrix[j] * m_ppAnimationMatrix[i][j];
				for (int k = 0; k < 4; ++k)
				{
					for (int l = 0; l < 4; ++l)
					{
						fprintf(fp, "%f\n", m_ppResultMatrix[i][j].Transpose().mData[k][l]);
					}
				}
			}
		}
	}

	fclose(fp);
}

void CFbx::_SetBoneMatrix(FbxNode* pNode)
{
	m_IndexByName[pNode->GetName()] = m_IndexByName.size();
	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		_SetBoneMatrix(pNode->GetChild(i));
	}
}

void CFbx::_SetAnimationData(FbxNode* pNode)
{
	FbxNodeAttribute *pFbxNodeAttribute = pNode->GetNodeAttribute();
	if (pFbxNodeAttribute && pFbxNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		FbxMesh* pMesh = pNode->GetMesh();

		int *IndexArr = new int[m_iVertexSize];
		for (int i = 0; i < m_iVertexSize; ++i)
		{
			IndexArr[i] = pMesh->GetPolygonVertices()[i];
			m_VertexByIndex[IndexArr[i]].push_back(i);
		}

		FbxAMatrix geometryTransform = _GetGeometryTransformation(pNode);
		FbxGeometry *pGeo = pNode->GetGeometry();
		int SkinCount = pGeo->GetDeformerCount(FbxDeformer::eSkin);
		for (int i = 0; i < SkinCount; ++i)
		{
			FbxSkin* pSkin = (FbxSkin*)pGeo->GetDeformer(i, FbxDeformer::eSkin);
			int ClusterCount = pSkin->GetClusterCount();

			for (int j = 0; j < ClusterCount; ++j)
			{
				FbxCluster *pCluster = pSkin->GetCluster(j);
				int ClusterIndexCount = pCluster->GetControlPointIndicesCount();
				int *ClusterIndices = pCluster->GetControlPointIndices();
				double *ClusterWeights = pCluster->GetControlPointWeights();

				for (int k = 0; k < ClusterIndexCount; ++k)
				{
					string BoneName = string(pCluster->GetLink()->GetName());
					int INDEX = m_IndexByName[BoneName];

					FbxAMatrix transformMatrix;
					FbxAMatrix transformLinkMatrix;
					FbxAMatrix ResultMtx;

					pCluster->GetTransformMatrix(transformMatrix); // The transformation of the mesh at binding time
					pCluster->GetTransformLinkMatrix(transformLinkMatrix); // The transformation of the cluster(joint) at binding time from joint space to world space
					ResultMtx = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

					for (int m = 0; m < 4; ++m)
					{
						for (int n = 0; n < 4; ++n)
						{
							m_pBaseBoneMatrix[INDEX].mData[m][n] = ResultMtx.Transpose().Get(m, n);
						}
					}

					float BoneWeight = static_cast<float>(ClusterWeights[k]);
					int BoneIndex = ClusterIndices[k];

					for (auto iter : m_VertexByIndex[BoneIndex])
					{
						if (INDEX != 0 && INDEX != -1)
						{
							m_pVertices[iter].AddBone(INDEX, BoneWeight);
						}
					}
				}

			}
		}
		delete[] IndexArr;
	}

	int nNodeChild = pNode->GetChildCount();
	for (int i = 0; i < nNodeChild; ++i)
	{
		FbxNode* pChildNode = pNode->GetChild(i);
		_SetAnimationData(pChildNode);
	}
}

void CFbx::_SetAnimationMatrix(FbxNode *pNode, FbxAnimStack *FbxAS)
{
	if (pNode)
	{
		unsigned int BoneIndex = m_IndexByName[pNode->GetName()];	//m_IndexByName.size();

		FbxTime maxTime = FbxAS->GetLocalTimeSpan().GetDuration();
		for (long long i = 0; i < maxTime.GetMilliSeconds() / 10; ++i)
		{
			FbxTime n_time;
			n_time.SetMilliSeconds(i * 10);

			for (int m = 0; m < 4; ++m)
			{
				for (int n = 0; n < 4; ++n)
				{
					m_ppAnimationMatrix[i][BoneIndex].mData[m][n] = pNode->EvaluateGlobalTransform(n_time).Transpose().Get(m, n);
				}
			}
		}
	}

	for (int i = 0; i < pNode->GetChildCount(); ++i)
	{
		_SetAnimationMatrix(pNode->GetChild(i), FbxAS);
	}
}











void Warning(char* pWarningText)
{
	cout << pWarningText << endl;
	cout << "아무 키나 누르면 종료합니다" << endl;
	char a;
	cin >> a;
	exit(0);
}



