#ifndef POSITION_INCLUDED
#define POSITION_INCLUDED

#include <string>
#include <cassert>

#include "bitboard.h"

namespace ChessEngine {

class Position 
{
public:
    static void init();

    Position() { for (int i = 0; i < NUM_SQUARES; i++) pieceBoard[i] = EMPTY; };
    Position(const Position&) = delete;

    Position& setPosFromFEN(const std::string& fen);

    uint64_t getPieceMask(PieceType pieceType = ALL_PIECES) const;
    uint64_t getPieceMask(PieceType pieceType, Color color) const;
    uint64_t getColorMask(Color color) const;

    Piece pieceOn(Square square) const;

    void placePiece(Piece piece, Square square);
    void movePiece(Square from, Square to);
    void removePiece(Square square);
    
    void print();

private:
    Piece pieceBoard[NUM_SQUARES];
    uint64_t typeBoard[NUM_PIECE_TYPES];
    uint64_t colorBoard[NUM_COLORS];
    int numPieces[NUM_PIECE_TYPES * NUM_COLORS];
    int ply;
    Color sideToMove;
};

inline uint64_t Position::getPieceMask(PieceType pieceType /*=ALL_PIECES */) const
{
    return typeBoard[pieceType];
}

inline uint64_t Position::getPieceMask(PieceType pieceType, Color color) const
{
    return getPieceMask(pieceType) & getColorMask(color);
}

inline uint64_t Position::getColorMask(Color color) const
{
    return colorBoard[color];
}

inline Piece Position::pieceOn(Square square) const 
{
    return pieceBoard[square];
}

inline void Position::placePiece(Piece piece, Square square)
{
    uint64_t squareMask = getSquareMask(square);
    pieceBoard[square] = piece;

    typeBoard[ALL_PIECES]       |= squareMask;
    typeBoard[getType(piece)]   |= squareMask;
    colorBoard[getColor(piece)] |= squareMask;

    numPieces[piece]++;
    numPieces[getPiece(ALL_PIECES, getColor(piece))]++;
}

inline void Position::movePiece(Square from, Square to)
{
    uint64_t moveMask = getSquareMask(from) | getSquareMask(to);
    Piece piece = pieceBoard[from];
    pieceBoard[from] = EMPTY;
    pieceBoard[to] = piece;

    typeBoard[ALL_PIECES]       ^= moveMask;
    typeBoard[getType(piece)]   ^= moveMask;
    colorBoard[getColor(piece)] ^= moveMask;
}

inline void Position::removePiece(Square square)
{
    uint64_t squareMask = getSquareMask(square);
    Piece piece = pieceBoard[square];
    pieceBoard[square] = EMPTY;

    typeBoard[ALL_PIECES]       ^= squareMask;
    typeBoard[getType(piece)]   ^= squareMask;
    colorBoard[getColor(piece)] ^= squareMask;

    numPieces[piece]--;
    numPieces[getPiece(ALL_PIECES, getColor(piece))]--;
}

} // namespace ChessEngine

#endif // POSITION_INCLUDED