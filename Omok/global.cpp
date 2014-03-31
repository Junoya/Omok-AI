
#include "stdafx.h"
#include "global.h"


/**
 @brief operator
 @date 2014-03-17
*/
bool SCandidate::operator<(const SCandidate &rhs)
{
	bool r = CompareLineType(ltype, rhs.ltype);
	return r;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// STable
STable::STable()
{
}

STable::STable(const STable &t)
{
	operator=(t);
}

const STable& STable::operator=(const STable &rhs)
{
	if (this != &rhs)
	{
		memcpy_s(pieces, sizeof(pieces), rhs.pieces, sizeof(rhs.pieces));
		blacks = rhs.blacks;
		whites = rhs.whites;
	}
	return *this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////




/**
 @brief ���ӵ� ���� Ÿ������ � ���� �� ����  �켱������ ���������� �����Ѵ�.
			ltype0 < ltype1
 @date 2014-03-17
*/
bool CompareLineType(const linetype ltype0, const linetype ltype1, const int alpha0, const int alpha1)
{
	const bool isComb0 = IsCombinationLineType(ltype0);
	const bool isComb1 = IsCombinationLineType(ltype1);
	if (isComb0 && isComb1)
	{
		return CompareCombinationLineType(ltype0, ltype1, alpha0, alpha1);
	}

	if (isComb0 || isComb1)
	{
		return CompareOneSideCombinationLineType(ltype0, ltype1, alpha0, alpha1);
	}

	const int score0 = GetLinetypeScore(ltype0);
	const int score1 = GetLinetypeScore(ltype1);
	return score0+alpha0 < score1+alpha1;
}


/**
 @brief �ΰ� �̻� ���յ� ����Ÿ���̶�� true�� �����Ѵ�.
 @date 2014-03-17
*/
bool IsCombinationLineType(const linetype ltype)
{
	return (ltype / 10000) > 0;
}


/**
 @brief ���յ� ����Ÿ�Գ��� �� operator< 
 @date 2014-03-17
*/
bool CompareCombinationLineType(const linetype ltype0, const linetype ltype1, const int alpha0, const int alpha1)
{
	const linetype ltype0Max = GetMaxCombinationLineType(ltype0);
	const linetype ltype1Max = GetMaxCombinationLineType(ltype1);
	const linetype ltype0Min = GetMinCombinationLineType(ltype0);
	const linetype ltype1Min = GetMinCombinationLineType(ltype1);
	
	const int score0Max = GetLinetypeScore(ltype0Max);
	const int score1Max = GetLinetypeScore(ltype1Max);

	if (score0Max == score1Max)
	{
		return CompareLineType(ltype0Min, ltype1Min, alpha0, alpha1);
	}
	return score0Max+alpha0 < score1Max+alpha1;
}


/**
 @brief ���ʸ� ���յ� ����Ÿ���� �� ��
 @date 2014-03-17
*/
bool CompareOneSideCombinationLineType(const linetype ltype0, const linetype ltype1, const int alpha0, const int alpha1)
{
	linetype cltype0 = ltype0;
	if (IsCombinationLineType(ltype0))
	{
		cltype0 = GetMaxCombinationLineType(ltype0);
	}

	linetype cltype1 = ltype1;
	if (IsCombinationLineType(ltype1))
	{
		cltype1 = GetMaxCombinationLineType(ltype1);
	}

	const int score0Max = GetLinetypeScore(cltype0);
	const int score1Max = GetLinetypeScore(cltype1);
	if (score0Max+alpha0 == score1Max+alpha1)
	{
		if (IsCombinationLineType(ltype0))
			return false;
		return true;
	}

	return score0Max+alpha0 < score1Max+alpha1;
}


/**
 @brief ���յ� ����Ÿ�Կ��� �� ���� �켱������ ����Ÿ���� �����Ѵ�.
 @date 2014-03-17
*/
linetype GetMaxCombinationLineType(const linetype ltype)
{
	linetype sltype[2];
	SeparateLineType(ltype, sltype[0], sltype[1]);
	const int maxLtype = CompareLineType(sltype[ 0], sltype[ 1]);
	if (1 == maxLtype)
		return sltype[ 1];
	return sltype[ 0];
}
linetype GetMinCombinationLineType(const linetype ltype)
{
	linetype sltype[2];
	SeparateLineType(ltype, sltype[0], sltype[1]);
	const int maxLtype = CompareLineType(sltype[ 0], sltype[ 1]);
	if (1 == maxLtype)
		return sltype[ 0];
	return sltype[ 1];
}

/**
 @brief ����Ÿ���� ������ ȯ���ؼ� �����Ѵ�.
 @date 2014-03-18
*/
int GetLinetypeScore(const linetype ltype)
{
	if (IsCombinationLineType(ltype))
	{
		const linetype maxLinetype =  GetMaxCombinationLineType(ltype);
		return GetLinetypeScore(maxLinetype);
	}

	const int pieceCnt0 = GetPieceCntFromlinetype(ltype);
	const int emptyCnt0 = GetEmptyCntFromlinetype(ltype);
	const int firstCnt0 = GetFirstCntFromlinetype(ltype);
	const int lastCnt0 = GetLastCntFromlinetype(ltype);
	const int length0 = pieceCnt0 + emptyCnt0 + firstCnt0 + lastCnt0;
	
	if (pieceCnt0 >= 6)
	{
		// nothing
	}
	else if (pieceCnt0 == 5)
	{
		return 100;
	}

	if (pieceCnt0 == 4)
	{
		if (length0 > 5) 
		{
			return 49;
		}
		else if (length0 == 5)
		{
			return 40;
		}
		else 
		{
			return 0;
		}
	}
	else if (pieceCnt0 == 3)
	{
		if (length0 <= 4)
			return 0; // X111X, X1101X, X1110X
		else
		{
			if ((emptyCnt0==1) && (firstCnt0 >= 1) && (lastCnt0 >= 1))
				return 39; // 011010, 010110
			else if ((emptyCnt0 <= 0) && (firstCnt0 >= 1) && (lastCnt0 >= 1))
				return 39; // 01110
			else if ((emptyCnt0 <= 1) && ((firstCnt0 >= 1) || (lastCnt0 >= 1)))
				return 30; // 0111X, X11010
			else if (emptyCnt0 >= 2)
				return 30; // 10101, 11001
			else
				return 0;
		}
	}
	else if (pieceCnt0 == 2)
	{
		if (length0 < 4) // maximum -> 001100, minimum -> 1100
			return 0;
		else if ((emptyCnt0==0) && (length0 < 5))
			return 0; // X0110X
		else if ((firstCnt0 >= 1) || (lastCnt0 >= 1))
			return 29; // 0110, 01010
		else 
			return 20; // X11000
	}
	else if (pieceCnt0 == 1)
	{
		if ((firstCnt0 >= 2) || (lastCnt0 >= 2))
			return 10; // 001, 100, 00100
		else
			return 0;
	}

	return 0;
}
