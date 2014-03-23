/**
@filename table.h

*/
#pragma once


class CTable
{
public:
	CTable();
	virtual ~CTable();

	void Init();
	void Render(HDC hdc);
	int NextAIStep(const PIECE pieceType=WHITE);
	GAME_STATE GetGameState();
	bool SetPiece(const Pos &pos, PIECE piece);
	void BackPiece();//�Ѽ� �ڷ�
	Pos ScreenPosToTablePos(const int x, const int y);
	void DisplayPieceNumber(bool isDisplay);


private:
	enum {
			MAX_BRUSH = 5,
	};

	GAME_STATE m_state;
	STable m_table;
	HBRUSH m_brushes[ MAX_BRUSH];
	float m_procTime; //milli second
	bool m_isDisplayNumber;
};


inline GAME_STATE CTable::GetGameState() { return m_state; }
inline void CTable::DisplayPieceNumber(bool isDisplay) {m_isDisplayNumber = isDisplay; }
