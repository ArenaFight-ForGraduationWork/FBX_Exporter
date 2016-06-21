#include "stdafx.h"
#include "Fbx.h"



void main()
{
	char fbxPath[100];
	char pathFront[50];
	char pathBack[50];
	int choice = 1;
	CFbx *pFbx = new CFbx();

	while (1 == choice)
	{
		system("cls");

		cout << "======================================================" << endl;
		cout << "                     fbx Exporter                     " << endl;
		cout << "======================================================" << endl;
		cout << endl;
		cout << " * ��ȯ�Ϸ��� fbx������ ��θ� �Է��� �ּ���: " << endl;
		cin >> fbxPath;
		cout << endl;

		cout << " * �ش� ������ �̸� �տ� ���� �ܾ �Է��� �ּ���: ";
		cin >> pathFront;
		cout << endl;

		cout << " * �ش� ���Ͽ� �ִϸ��̼��� ������ 0��, ������ 1�� �Է��� �ּ���: ";
		cin >> choice;
		cout << endl;

		switch (choice)
		{
		case 0:
		default:
			pFbx->Import(fbxPath, pathFront);
			break;
		case 1:
			cout << " * matrix������ �� �ڿ� ���� �ܾ �Է��� �ּ���: ";
			cin >> pathBack;
			pFbx->Import(fbxPath, pathFront, pathBack);
			break;
		}
		cout << endl;

		cout << " * �ٸ� ������ �� ��ȯ�Ͻ÷��� 1��, �����Ϸ��� 0�� �����ּ���: ";
		cin >> choice;
	}
}
