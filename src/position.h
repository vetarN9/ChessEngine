#ifndef POSITION_INCLUDED
#define POSITION_INCLUDED

#include <string>
#include <cassert>

#include "bitboard.h"

namespace ChessEngine {

class Position {
public:
    static void Init();

    Position() = default;
    Position(const Position&) = delete;

    // Get/set FEN string
    Position& Set(const std::string& fen);
    std::string FEN() const;

    // Position pieces 
    inline Bitboard Pieces(PieceType pt, Color color)   const { return Pieces(pt) & Pieces(color); }
    inline Bitboard Pieces(PieceType pt, PieceType pt2) const { return Pieces(pt) | Pieces(pt2); }
    inline Bitboard Pieces(PieceType pt = ALL_PIECES)   const { return typeBoard[pt]; }
    inline Bitboard Pieces(Color color)                 const { return colorBoard[color]; }
    inline Piece PieceOn(Square square)                 const { return pieceOnSquare[square]; }
    inline int NumPieces(PieceType pt, Color color)     const { return numPieces[getPiece(pt, color)]; }
    inline int NumPieces(PieceType pt)                  const { return NumPieces(pt, WHITE) + NumPieces(pt, BLACK); }
    Square KingSquare(Color color)                      const;

    // Checking
    inline Bitboard Checkers() const { return checkersBoard; }
    inline Bitboard CheckSquares(PieceType pt) const { return checkSquares[pt]; }
    inline Bitboard KingBlockers(Color kingColor) const { return kingBlockers[kingColor]; }
    inline Bitboard Pinned(Color color) const { return pinned[color]; }
    inline Bitboard Discovery(Color color) const { return discovery[color]; }
    inline Bitboard Pinners(Color color) const { return pinners[color]; }

    //
    Bitboard AttacksTo(Square square, Bitboard occupancy) const;
    inline Bitboard AttacksTo(Square square) const { return AttacksTo(square, Pieces()); }
    inline bool SquareIsAttacked(Square square) const { return AttacksTo(square) & Pieces(); }
    inline bool SquareIsAttacked(Square square, Color attacker) const { return AttacksTo(square) & Pieces(attacker); }

    // Getters of member variables
    inline Color SideToMove() const       { return sideToMove; }
    inline uint8_t CastlingRights() const { return castlingRights; }
    inline Piece CapturedPiece() const    { return capturedPiece; }
    inline Square EnpassantSquare() const { return enpassantSquare; }

    // Doing and undoing moves
    
    void Print();

private:
    // Piece manipulation
    void PlacePiece(Piece piece, Square square);
    void MovePiece(Square from, Square to);
    void RemovePiece(Square square);

    Bitboard SliderBlockers(Color attacker, Square target, Bitboard& pinners) const;

    // Helpers for initialization
    void ParsePiecePlacement(std::istringstream& ss);
    void ParseActiveColor(std::istringstream& ss);
    void ParseCastling(std::istringstream& ss);
    void ParseEnpassantSquare(std::istringstream& ss);
    void ParseMoveCounters(std::istringstream& ss);
    void SetCheckingData();

    Piece pieceOnSquare[NUM_SQUARES];
    Bitboard typeBoard[NUM_PIECE_TYPES];
    Bitboard colorBoard[NUM_COLORS];
    int numPieces[NUM_PIECES];

    int ply;
    Color sideToMove;
    Square enpassantSquare;
    int fiftyMoveCounter;
    uint8_t castlingRights;

    Bitboard checkersBoard;
    Bitboard kingBlockers[NUM_COLORS];
    Bitboard pinned[NUM_COLORS];
    Bitboard pinners[NUM_COLORS];
    Bitboard discovery[NUM_COLORS];
    Bitboard checkSquares[NUM_PIECE_TYPES];
    Piece    capturedPiece;
    int      repetition;
};

inline Square Position::KingSquare(Color color) const
{
    assert(Pieces(KING, color) && "Position must include one king of both sides");
    return getSquare(Pieces(KING, color));
}

} // namespace ChessEngine

#endif // POSITION_INCLUDED