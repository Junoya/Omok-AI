
#include "stdafx.h"
#include "ai.h"


namespace ai
{
	pair<Pos,int> RecursiveSearch( STable &table, const PIECE pieceType, const PIECE originalPieceType, const int count );

	enum GETLINETYPE_OPTION { SEARCH_CANDIDATE, SEARCH_DEFENSE };
	bool GetCandidateLocation( STable &table, const PIECE piece, const GETLINETYPE_OPTION option,
		OUT vector<SCandidate> &candidates, OUT map<Pos,SSearchInfo> &info );

	linetype GetLineType( STable &table, const CHECK_TYPE chkType, const PIECE piece, const Pos &pos, const GETLINETYPE_OPTION option,
		INOUT map<Pos,SSearchInfo> &info, OUT vector<Pos> &candidates );

	void SearchCombination( INOUT vector<SCandidate> &candidates );
	void MergeCandidate(const vector<SCandidate> &curCandidates, const vector<SCandidate> &oppositeCandidates, OUT vector<SCandidate> &out);
	int CalculateCandidateScore(const vector<SCandidate> &curCandidates, const vector<SCandidate> &oppositeCandidates);
	Pos RandLocation(STable &table, const PIECE piece);


	struct PosInfo
	{ 
		PosInfo(Pos pos0, int piece0):pos(pos0),piece(piece0) {}
		Pos pos;
		int piece;
	};
	bool CompareLines(const vector<PosInfo> &line, const string &pieces);

	struct SResultInfo 
	{
		SResultInfo(	int result0, Pos pos0, linetype ltype0,PIECE pieceType0) : result(result0), pos(pos0), ltype(ltype0), pieceType(pieceType0) {}
		bool operator<(const SResultInfo &rhs) {
			return (result == rhs.result)? CompareLineType(ltype, rhs.ltype) : result < rhs.result;
		}
		int result;
		Pos pos;
		linetype ltype;
		PIECE pieceType;
	};

	struct SMergeInfo 
	{
		SMergeInfo(Pos pos0, linetype ltype0, int score0) : pos(pos0), ltype(ltype0), score(score0) {}
		bool operator<(const SMergeInfo &rhs) {
			return score < rhs.score;
		}
		Pos pos;
		linetype ltype;
		int score;
	};

	int g_totalCnt=0;
}

using namespace ai;


/**
 @brief ���� �ΰ����� ����.
			pieceType�� ���� ���� ������ ��ġ�� �����Ѵ�.
 @date 2014-03-18
*/
GAME_STATE ai::SearchBestLocation( STable &table, const PIECE pieceType, OUT Pos &out )
{
	g_totalCnt = 0;
	const pair<Pos,int> result = RecursiveSearch(table, pieceType, pieceType, 3);

	out = result.first;
	return GAME;
}


/**
 @brief ��������� Ž���ϸ� ���� ���� ���� ã�´�.
			 count �� n ���� �� ������ Ʈ���� �����, �� ���¿��� peiceType�� ��Ȳ��
			 ������ ǥ���� �����Ѵ�.

			 ������ 0 ���ΰ�� �������� ��ġ�� ���� ��
			 ����� ��� pieceType�� ���� �� ��Ȳ
			 ������ ��� pieceType�� ��밡 ������ ��Ȳ 

			 ���ϰ� pair<��ġ, ����>
 @date 2014-03-18
*/
pair<Pos,int> ai::RecursiveSearch( STable &table, const PIECE pieceType, const PIECE originalPieceType, const int count )
{
	stringstream ss;
	ss << "RecursiveSearch " << ++g_totalCnt << " curCnt " << count << std::endl;
	OutputDebugStringA( ss.str().c_str() );

	map<Pos,SSearchInfo> searchInfo;
	vector<SCandidate> curCandidate, oppositeCandidate;
	const bool isOverOpposite = GetCandidateLocation(table, OppositePiece(pieceType), SEARCH_DEFENSE, oppositeCandidate, searchInfo);
	const bool isOverCurrent = GetCandidateLocation(table, pieceType, SEARCH_CANDIDATE, curCandidate, searchInfo);

	if (isOverOpposite || isOverCurrent)
	{
		return (isOverCurrent)? pair<Pos,int>(Pos(-1,-1),-100) : pair<Pos,int>(Pos(-1,-1),100);
	}

	if ((count <= 0) && (pieceType == originalPieceType))// max search depth
	{
		const int score = CalculateCandidateScore(curCandidate, oppositeCandidate);
		return pair<Pos,int>(Pos(-1,-1), score);
	}

	vector<SCandidate> candidates;
	MergeCandidate(curCandidate, oppositeCandidate, candidates);

	if (candidates.empty())
	{
		Pos pos = RandLocation(table, pieceType);
		candidates.push_back(SCandidate(pos, 0));
	}
	
	int procCnt = 0;
	int maxCnt = 3;
	vector<SResultInfo> results;
	results.reserve(10);
	BOOST_REVERSE_FOREACH (auto &cand, candidates)
	{
		if (++procCnt > maxCnt)
			break;

		STable newTable = table;
		SetPiece(newTable, cand.pos, pieceType);
		const pair<Pos,int> r = RecursiveSearch(newTable, OppositePiece(pieceType), originalPieceType, count-1);
		results.push_back( SResultInfo(r.second, cand.pos, cand.ltype, pieceType) );
	}

	if (results.empty())
		return pair<Pos,int>(Pos(-1,-1), -100);

	// ���� ���� ������ ã�� �����Ѵ�.
	std::sort(results.begin(), results.end());

	return pair<Pos,int>(results.back().pos, -results.back().result); // ����� ��ƾ���� �Ѿ�鼭 result���� ��ȣ�� �ٲ��.
}


/**
 @brief check range
 @date 2014-03-15
*/
bool ai::CheckEmpty(STable &table, const Pos &pos)
{
	if (!CheckRange(pos))
		return false;
	if (table.pieces[ pos.x][ pos.y] != EMPTY)
		return false;
	return true;
}


/**
 @brief ���̺� ������ ����� false�� �����Ѵ�.
 @date 2014-03-17
*/
bool ai::CheckRange(const Pos &pos)
{
	if (MAX_TABLE_X <= pos.x)
		return false;
	if (0 > pos.x)
		return false;
	if (MAX_TABLE_Y <= pos.y)
		return false;
	if (0 > pos.y)
		return false;
	return true;
}


/**
 @brief 
 @date 2014-03-15
*/
bool ai::SetPiece(STable &table, const Pos &pos, PIECE piece)
{
	//if (GAME != m_state)
	//	return false;
	if (piece == EMPTY)
	{
		if (!CheckRange(pos))
			return false;
	}
	else
	{
		if (!CheckEmpty(table, pos))
			return false;
	}

	table.pieces[ pos.x][ pos.y] = piece;

	switch (piece)
	{
	case BLACK: table.blacks.push_back(pos); break;
	case WHITE: table.whites.push_back(pos); break;
	}
	return true;
}


/**
 @brief 
 @date 2014-03-15
*/
PIECE ai::GetPiece(STable &table, const Pos &pos)
{
	if (CheckRange(pos))
		return table.pieces[ pos.x][ pos.y];
	return WALL;
}


/**
 @brief Get Candidate Location
			���� ���� ���� candidates �� back() �� �ִ�.
			�������� ������� ������������ ���ĵȴ�.
 @date 2014-03-15
*/
bool ai::GetCandidateLocation( STable &table, const PIECE piece,  const GETLINETYPE_OPTION option, 
	OUT vector<SCandidate> &candidates, OUT map<Pos,SSearchInfo> &info )
{
	if ((piece != WHITE) && (piece != BLACK)) return false;

	CHECK_TYPE chktypes[4] = {CHECK_ROW, CHECK_COL, CHECK_SLASH_LEFT, CHECK_SLASH_RIGHT};

	const vector<Pos> &pieces = (piece == WHITE)? table.whites : table.blacks;

	vector<SCandidate> cand;
	cand.reserve(pieces.size());

	BOOST_FOREACH (auto &pos, pieces)
 	{
		for (int c=0; c < 4; ++c)
		{
			vector<Pos> out;
			const linetype ltype = GetLineType(table, chktypes[ c], piece, pos, option, info, out);
			if ((GetPieceCntFromlinetype(ltype) == 5) && (GetEmptyCntFromlinetype(ltype) == 0))
				return true; // ����, ���ӿ���.

			if (!out.empty())
			{
				BOOST_FOREACH (auto &p0, out)
					cand.push_back(SCandidate(p0, ltype));
			}
		}
	}

	SearchCombination(cand);

	//----------------------------------------------------------------------------------
	// candidate ����.
	vector<int> scores(cand.size());
	for (unsigned int i=0; i < cand.size(); ++i)
		scores[ i] = GetLinetypeScore(cand[ i].ltype);

	const int MAX_CANDIDATE_COUNT = 5;
	// ���� ���� �켱������ Location �� MAX_CANDIDATE_COUNT ���� ã�� �����Ѵ�.
	int count = 0;
	while ((count < MAX_CANDIDATE_COUNT) && (count < (int)scores.size()))
	{
		int maxScore = -10000;
		int maxIdx = -1;
		for (unsigned int i=0; i < scores.size(); ++i)
		{
			if (maxScore < scores[ i])
			{
				maxScore = scores[ i];
				maxIdx = (int)i;
			}
		}
		// find!!
		if (maxIdx >= 0)
		{
			++count;
			scores[ maxIdx] = -10000;// initialize
			candidates.push_back( SCandidate(cand[ maxIdx].pos, cand[ maxIdx].ltype) );
		}
	}

	//std::sort(candidates.begin(), candidates.end());
	std::reverse(candidates.begin(), candidates.end());
	//----------------------------------------------------------------------------------


	return false;
}


/**
 @brief ���ӵ� ���� ������ �����Ѵ�. 2, 3, 4 
             ��ĭ�� ������ ���յ� Ž���Ѵ�.
 @date 2014-03-15
*/
linetype ai::GetLineType( STable &table, const CHECK_TYPE chkType, const PIECE piece, const Pos &pos, 
	const GETLINETYPE_OPTION option, OUT map<Pos,SSearchInfo> &info, OUT vector<Pos> &candidates )
{
	Pos offset;
	switch (chkType)
	{
	case CHECK_ROW: offset = Pos(1,0); break;
	case CHECK_COL: offset = Pos(0,1); break;
	case CHECK_SLASH_LEFT: offset = Pos(1,1); break;
	case CHECK_SLASH_RIGHT: offset = Pos(1,-1); break;
	}

	vector<PosInfo> cand; // 0=empty pos, 1=piece, 
	Pos startPos(-1,-1);

	// line down/up
	int pieceCnt = 0;
	int wallCnt = 0;
	int loopCnt = 0;
	while (loopCnt < 2)
	{
		int emptyContinue = 0;
		Pos p0 = (startPos==Pos(-1,-1))? pos : startPos;

		while (1)
		{
			const PIECE curPiece = GetPiece(table, p0);
			if ((curPiece == WALL) || (curPiece == OppositePiece(piece)))
			{
				++wallCnt;
				if (emptyContinue > 0)
					--wallCnt;

				if (0 == loopCnt)
				{
					startPos = p0 - offset;
					if (emptyContinue > 0)
					{
						startPos -= offset;
						cand.push_back(PosInfo(p0-offset,0));
					}
				}
				break;
			}

			auto it = info.find(p0);
			if (info.end() != it)
			{
				if (chkType & it->second.check)
				{
					return 0; // already check
				}
			}

			if (curPiece == EMPTY)
			{
				++emptyContinue;
				if (emptyContinue >= 2)
				{
					if (0 == loopCnt)
					{
						startPos = p0 - offset - offset;
						cand.push_back(PosInfo(p0,0));
						cand.push_back(PosInfo(p0-offset,0));
					}
					else
					{
						cand.push_back(PosInfo(p0,0));
					}

					break;
				}
				else
				{
					if (1 == loopCnt)
						cand.push_back(PosInfo(p0,0));
				}
			}
			else
			{
				emptyContinue = 0;

				if (1 == loopCnt)
				{
					cand.push_back(PosInfo(p0,1));
					++pieceCnt;
					info[ p0].check |= chkType;
				}
			}

			p0 += ((loopCnt==0)? offset : -offset);
		} // while

		++loopCnt;
	} // while


	// calculate empty piece count
	bool startPiece = false;
	int emptyCnt = 0;
	BOOST_FOREACH (auto &c, cand)
	{
		if (!startPiece && (1 == c.piece))
			startPiece = true;
		if (startPiece && (0 == c.piece))
			++emptyCnt;
	}
	startPiece = false;
	BOOST_REVERSE_FOREACH(auto &c, cand)
	{
		if (!startPiece && (1 == c.piece))
			break;
		--emptyCnt;
	}
	//

	// 11011 -> 11X11
	// 10111 -> 1X111 ... etc
	// 010110 -> 01X110 ... etc
	if (((4 == pieceCnt) && (1 == emptyCnt)) ||
		((3 == pieceCnt) && (1 == emptyCnt) && (0 == wallCnt)))
	{
		// ������ emtpy ������ �����Ѵ�.
		while (!cand.empty() && (cand.front().piece == 0))
		{ // pop_front
			std::rotate(cand.begin(), cand.begin()+1, cand.end());
			cand.pop_back();
		}
		while (!cand.empty() && (cand.back().piece == 0))
			cand.pop_back();
	}

	// 0111101X -> 01111
	// X1111010 -> 10
	if ((5 <= pieceCnt) && (1 == emptyCnt))
	{
		// 0111101X -> 01111
		// 00111101X -> 01111
		if (CompareLines(cand, "0111101") || CompareLines(cand, "00111101"))
		{
			cand.pop_back();
			cand.pop_back();
			// -> 01111, 001111
			pieceCnt = 4;
			emptyCnt = 0;
			wallCnt = 1;
		}
		else
		{
			// ���ӵ� �� ������ pieceCnt�� �ȴ�.
			// 1110111 -> 111X111
			int maxPieces = 0;
			int cnt = 0;
			BOOST_FOREACH(auto &c, cand)
			{
				if (c.piece == 1)
				{
					++cnt;
				}
				else
				{
					c.piece = -1; // X �ĺ������� ����.

					if (maxPieces < cnt)
						maxPieces = cnt;
					cnt = 0;
				}
			}

			pieceCnt = maxPieces;
			wallCnt = max(wallCnt,1);
			emptyCnt = 0;
		}

	}

	bool removeSideEmpty = false;
	if ((4 <= pieceCnt) && (2 <= emptyCnt))
	{
		// 110101, 1010101
		if ((4 == pieceCnt)  && (2 == emptyCnt))
		{
			// nothing
		}
		else
		{
			//pieceCnt = 3;
			//wallCnt = 0;
			//removeSideEmpty = true;
		}
	}

	// X1101011X -> X
	if ((5 <= pieceCnt) && (2 <= emptyCnt))
	{
		// X1101011X -> X
		if (wallCnt == 2)
		{
			if (CompareLines(cand, "1101011"))
			{
				pieceCnt = 0;
				emptyCnt = 0;
				wallCnt = 0;
				cand.clear();
			}
		}

		// 1110101
		// 1011101
	}

	// 0111, 1110 -> 111
	// wallCnt=2 �� ����
	if ((3 == pieceCnt) && (0 == emptyCnt) && (1==wallCnt))
	{
		if (CompareLines(cand, "0111") || CompareLines(cand, "1110"))
		{
			wallCnt = 2;
			removeSideEmpty = true;
		}
	}

	// 3���̻� ���ӵ� ��� ���� �� empty�� 1���θ� �����Ѵ�.
	if (3 <= pieceCnt)
	{
		removeSideEmpty = true;
	}

	// ����ɼ��� ���� ������ empty������ 1�� �����Ѵ�.
	// ���ӵ� 4���϶��� ��������.
	if (!cand.empty() &&
		(removeSideEmpty ||
		(((4 == pieceCnt) && (0 == emptyCnt)) ||
		 (SEARCH_DEFENSE == option))))
	{ // 00111100 -> 011110
		if ((cand[ 0].piece == 0) && (cand[ 1].piece == 0)) 
		{ // pop_front
			std::rotate(cand.begin(), cand.begin()+1, cand.end());
			cand.pop_back();
		}

		const int s = cand.size();
		if ((cand[ s-1].piece == 0) && (cand[ s-2].piece == 0))
			cand.pop_back();
	}

	// ���ӵ� ���� 5�� ��� pieceCnt�� 5�� �����ϰ� �����Ѵ�.
	if (pieceCnt >= 6)
	{
		int cnt = 0;
		BOOST_FOREACH (auto &c, cand)
		{
			if (1 == c.piece)
			{
				++cnt;
			}
			else
			{
				if (5 == cnt)
					break;
				cnt = 0;
			}
		}

		if (5 == cnt)
		{
			pieceCnt = 5;
			emptyCnt = 0;
			wallCnt = 0;
		}
	}


	BOOST_FOREACH (auto &c, cand)
	{
		if (0 == c.piece)
		{
			candidates.push_back(c.pos);
		}
	}

	return ::GetLineType(pieceCnt, emptyCnt, wallCnt);
}


/**
 @brief 3-3, 3-4, 4-4 ������ ã�Ƽ� ��ģ��.
 @date 2014-03-16
*/
void ai::SearchCombination( INOUT vector<SCandidate> &candidates )
{
	map<Pos, vector<SCandidate> > combs;

	BOOST_FOREACH (auto cand, candidates)
	{
		combs[ cand.pos].push_back(cand);
	}

	// sorting
	BOOST_FOREACH (auto &kv, combs)
		std::sort(kv.second.begin(), kv.second.end());

	BOOST_FOREACH (auto &kv, combs)
	{
		if (2 > GetPieceCntFromlinetype(kv.second.back().ltype))
		{
			// ���Ӱ����� 2���̻� �˻翡 ���Խ�Ų��.
			kv.second.clear();
			continue;
		}

		if (kv.second.size() >= 2)
		{
			const linetype ltype = MergeLineType(kv.second[ kv.second.size()-1].ltype, kv.second[ kv.second.size()-2].ltype);
			kv.second.clear();
			kv.second.push_back(SCandidate(kv.first, ltype));
		}
	}

	candidates.clear();
	candidates.reserve(combs.size());

	BOOST_FOREACH(auto kv, combs)
	{
		if (!kv.second.empty())
			candidates.push_back( kv.second.back() );
	}
}


/**
 @brief candidate0-1 ������ ���ļ� �켱������ ���� ������ ������������ �����Ѵ�.
 @date 2014-03-18
*/
void ai::MergeCandidate(const vector<SCandidate> &cur, const vector<SCandidate> &oppoisite, 
	OUT vector<SCandidate> &out)
{
	if (cur.empty() && oppoisite.empty())
		return;

	const int MAX_PROC_COUNT = 5;
	vector<SMergeInfo> m;
	m.reserve(cur.size() + oppoisite.size());
	
	for (int i=cur.size()-1; ((cur.size()-i) < MAX_PROC_COUNT) && (i >= 0); --i)
		m.push_back( SMergeInfo(cur[ i].pos, cur[ i].ltype, GetLinetypeScore(cur[ i].ltype)+5) );

	for (int i=oppoisite.size()-1; ((oppoisite.size()-i) < MAX_PROC_COUNT) && (i >= 0); --i)
		m.push_back( SMergeInfo(oppoisite[ i].pos, oppoisite[ i].ltype, GetLinetypeScore(oppoisite[ i].ltype)) );
	
	sort(m.begin(), m.end());

	BOOST_FOREACH (auto &info, m)
		out.push_back( SCandidate(info.pos, info.ltype) );
}


/**
 @brief ���� ���¸� �������� ������ ���Ѵ�. ���ӵ� �� ������ wall ������ �������´�.
            �ڽŰ� ����� ������ ���� �����Ѵ�.
			�ڽ��� �����ϸ� ���, �Ҹ��ϸ� ������ �����ϰ� �ȴ�.
 @date 2014-03-18
*/
int ai::CalculateCandidateScore(const vector<SCandidate> &cur, const vector<SCandidate> &opposite)
{
	int curScore = (cur.empty())? 0 : GetLinetypeScore(cur.back().ltype);
	int oppositeScore = (opposite.empty())? 0 : GetLinetypeScore(opposite.back().ltype);

	// ���� ���� �� ������� +5 ���� �߰��Ѵ�.
	return curScore+5 - oppositeScore;
}


/**
 @brief ������ �ϼ��Ǹ� true�� �����Ѵ�.
 @date 2014-03-17
*/
bool ai::IsGameComplete(STable &table, const PIECE pieceType)
{
	map<Pos,SSearchInfo> info;
	vector<SCandidate> candidates;
	if (GetCandidateLocation(table, pieceType, SEARCH_CANDIDATE, candidates, info))
		return true;

	return false;
}


/**
 @brief search random  location
 @date 2014-03-17
*/
Pos ai::RandLocation(STable &table, const PIECE piece)
{
	vector<Pos> &pieces = (OppositePiece(piece)==BLACK)? table.blacks : table.whites;

	int x = MAX_TABLE_X/2;
	int y = MAX_TABLE_Y/2;
	if (!pieces.empty())
	{
		x = pieces[0].x;
		y = pieces[0].y;
	}

	while (table.pieces[ x][ y] != EMPTY)
	{
		++x;
	}

	return Pos(x,y);
}


/**
 @brief ������ ���� ������ ���ٸ� true�� �����Ѵ�.
		    pieces => 010101 , 11011, 1?1 -> 101, 111 ��� �ش�
 @date 2014-03-19
*/
bool ai::CompareLines(const vector<PosInfo> &line, const string &pieces)
{
	if (line.size() != pieces.length())
		return false;

	for (unsigned int i=0; i < line.size(); ++i)
	{
		if (pieces[ i] == '?')
			continue;
		if ((pieces[ i] == '0') && (line[ i].piece == 0))
			continue;
		if ((pieces[ i] == '1') && (line[ i].piece == 1))
			continue;
		return false;
	}
	return true;
}
 