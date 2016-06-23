#include "stdafx.h"
#include "Fbx.h"



void main()
{
	char fbxPath[100];
	char pathFront[50];
	char pathBack[50];
	int choice = 1;

	while (1 == choice)
	{
		system("cls");

		cout << "======================================================" << endl;
		cout << "                     fbx Exporter                     " << endl;
		cout << "======================================================" << endl;
		cout << endl;
		cout << " * 변환하려는 fbx파일의 경로를 입력해 주세요: " << endl;
		cin >> fbxPath;
		cout << endl;

		cout << " * 해당 파일의 이름 앞에 붙일 단어를 입력해 주세요: ";
		cin >> pathFront;
		cout << endl;

		cout << " * 해당 파일에 애니메이션이 없으면 0을, 있으면 1을 입력해 주세요: ";
		cin >> choice;
		cout << endl;

		CFbx *pFbx = new CFbx();
		switch (choice)
		{
		case 0:
		default:
			pFbx->Import(fbxPath, pathFront);
			break;
		case 1:
			cout << " * matrix파일의 맨 뒤에 붙일 단어를 입력해 주세요: ";
			cin >> pathBack;
			pFbx->Import(fbxPath, pathFront, pathBack);
			break;
		}
		cout << endl;
		delete pFbx;

		cout << " * 다른 파일을 더 변환하시려면 1을, 종료하려면 0을 눌러주세요: ";
		cin >> choice;
	}
}
