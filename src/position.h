#ifndef POSITION_INCLUDED
#define POSITION_INCLUDED

#include <string>
#include <cassert>

#include "bitboard.h"

namespace ChessEngine {

struct PosInfo {
    Square enpassantSquare;
    uint8_t castlingRights;
    int fiftyMoveCounter;
    int movesFromNull;
    
    PosInfo* prev;
    Bitboard checkersBoard;
    Bitboard pinners[NUM_COLORS];
    Bitboard pinned[NUM_COLORS];
    Bitboard discovery[NUM_COLORS];
    Bitboard checkSquares[NUM_PIECE_TYPES];
    Piece capturedPiece;
    int repetition;
};

class Position {
public:
    static void Init();

    Position() = default;
    Position(const Position&) = delete;

    // Get/set FEN string
    Position& Set(const std::string& fen, PosInfo* posInfo);
    std::string FEN() const;

    // Position pieces 
    Square KingSquare(Color color) const;
    inline Bitboard Pieces(PieceType pt, Color color)   const { return Pieces(pt) & Pieces(color); }
    inline Bitboard Pieces(PieceType pt, PieceType pt2) const { return Pieces(pt) | Pieces(pt2); }
    inline Bitboard Pieces(PieceType pt = ALL_PIECES)   const { return typeBoard[pt]; }
    inline Bitboard Pieces(Color color)                 const { return colorBoard[color]; }
    inline Piece PieceOn(Square square)                 const { return pieceOnSquare[square]; }
    inline int NumPieces(PieceType pt, Color color)     const { return numPieces[getPiece(pt, color)]; }
    inline int NumPieces(PieceType pt)                  const { return NumPieces(pt, WHITE) + NumPieces(pt, BLACK); }

    // Checking
    inline Bitboard Checkers() const { return posInfo->checkersBoard; }
    inline Bitboard CheckSquares(PieceType pt) const { return posInfo->checkSquares[pt]; }
    inline Bitboard Pinned(Color color) const { return posInfo->pinned[color]; }
    inline Bitboard Discovery(Color color) const { return posInfo->discovery[color]; }
    inline Bitboard Pinners(Color color) const { return posInfo->pinners[color]; }

    // Attack info
    Bitboard AttackersTo(Square square, Bitboard occupancy) const;
    inline Bitboard AttackersTo(Square square) const { return AttackersTo(square, Pieces()); }
    inline bool SquareIsAttacked(Square square) const { return AttackersTo(square) & Pieces(); }
    inline bool SquareIsAttacked(Square square, Color attacker) const { return AttackersTo(square) & Pieces(attacker); }
    bool SquaresNotAttacked(Bitboard bitboard, Color attacker) const;

    // Getters of member variables
    inline Color SideToMove() const       { return sideToMove; }
    inline uint8_t CastlingRights() const { return posInfo->castlingRights; }
    inline Piece CapturedPiece() const    { return posInfo->capturedPiece; }
    inline Square EnpassantSquare() const { return posInfo->enpassantSquare; }

    // Making and undoing moves
    void MakeMove(Move move, PosInfo& newPosInfo);
    void UndoMove(Move move);

    void Print();

private:
    // Piece manipulation
    void PlacePiece(Piece piece, Square square);
    void MovePiece(Square from, Square to);
    void RemovePiece(Square square);

    void SetCastlingRights(CastlingRight cr);

    void MakeCastling(Move move);
    void UndoCastling(Move move);

    Bitboard SliderBlockers(Color attacker, Square target, Bitboard& pinners) const;

    // Helpers for initialization
    void ParsePiecePlacement(std::istringstream& ss);
    void ParseActiveColor(std::istringstream& ss);
    void ParseCastling(std::istringstream& ss);
    void ParseEnpassantSquare(std::istringstream& ss);
    void ParseMoveCounters(std::istringstream& ss);
    void SetCheckingData();

    PosInfo* posInfo;
    Piece pieceOnSquare[NUM_SQUARES];
    Bitboard typeBoard[NUM_PIECE_TYPES];
    Bitboard colorBoard[NUM_COLORS];
    uint8_t castlingRightsMask[NUM_SQUARES];
    int numPieces[NUM_PIECES];
    int ply;
    Color sideToMove;
};

inline Square Position::KingSquare(Color color) const
{
    assert(Pieces(KING, color) && "Position must include one king of both sides");
    return firstSquare(Pieces(KING, color));
}

} // namespace ChessEngine

#endif // POSITION_INCLUDED