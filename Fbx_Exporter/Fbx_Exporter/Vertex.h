#ifndef VERTEX_H_
#define VERTEX_H_



class CAnimationVertex
{
public:
	CAnimationVertex()
	{
		for (int i = 0; i < 8; ++i)
		{
			BoneIndexArr[i] = 0;
			BoneWeightArr[i] = -1.0f;
		}
	}
	~CAnimationVertex() {}

	void AddBone(int index, float Weight)
	{
		for (int i = 0; i < 8; ++i)
		{
			if (BoneWeightArr[i] <= 0.0f)
			{
				BoneIndexArr[i] = index;
				BoneWeightArr[i] = Weight;
				return;
			}
		}
		//만약 배열을 벗어나면 첫번째 인덱스에 다 더한다.
		BoneWeightArr[0] += Weight;
	}

	int GetBoneIndexArray(int idx)
	{
		if ((idx < 0) || (idx > 7))
			throw std::exception("idx 범위를 넘어감");
		return BoneIndexArr[idx];
	}
	float GetBoneWeightArray(int idx)
	{
		if ((idx < 0) || (idx > 7))
			throw std::exception("idx 범위를 넘어감");
		return BoneWeightArr[idx];
	}

private:
	int BoneIndexArr[8];
	float BoneWeightArr[8];
};



#endif