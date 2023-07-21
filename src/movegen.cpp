#include "movegen.h"
#include "position.h"

namespace ChessEngine {

namespace {

void generateQuietPawnMoves(const Position& pos, MoveList& moveList);
void generatePawnPromotionMoves(const Position& pos, MoveList& moveList);
void generatePawnCaptures(const Position& pos, MoveList& moveList);

inline void addMove(MoveList& moveList, Move move);
inline void addPromotionMove(MoveList& moveList, Square from, Square to);

} // anonymous namespace


void generateMoves(const Position& pos, MoveList& moveList)
{
    generateQuietPawnMoves(pos, moveList);
    generatePawnPromotionMoves(pos, moveList);
    generatePawnCaptures(pos, moveList);
}


namespace {

void generateQuietPawnMoves(const Position& pos, MoveList& moveList)
{
    Color us = pos.SideToMove();

    Bitboard promotionRank  = getRankMask(relativeRank(RANK_7, us));
    Bitboard doublePushRank = getRankMask(relativeRank(RANK_3, us));
    Bitboard emptySquares   = ~pos.Pieces();
    Bitboard pushers        = pos.Pieces(PAWN, us) & ~promotionRank;
    Direction up            = getPawnDir(us);

    Bitboard singlePush = shift(pushers, up)  & emptySquares;
    Bitboard doublePush = shift(singlePush & doublePushRank, up) & emptySquares;

    while (singlePush)
    {
        Square to = popFirstSquare(singlePush);
        Square from = to - up;
        addMove(moveList, makeQuietMove(from, to));
    }

    while (doublePush)
    {
        Square to = popFirstSquare(doublePush);
        Square from = to - up - up;
        addMove(moveList, makeQuietMove(from, to));
    }
}

void generatePawnPromotionMoves(const Position& pos, MoveList& moveList)
{
    Color us = pos.SideToMove();
    Color them = ~us;

    Bitboard promotionRank  = getRankMask(relativeRank(RANK_7, us));
    Bitboard promoters      =  pos.Pieces(PAWN, us) & promotionRank;

    if (!promoters)
        return;

    Bitboard captures = pos.Pieces(them);
    Bitboard emptySquares  = ~pos.Pieces();

    Direction up = getPawnDir(us);
    Direction upLeft  = (us == WHITE ? NORTH_WEST : SOUTH_EAST);
    Direction upRight = (us == WHITE ? NORTH_EAST : SOUTH_WEST);

    Bitboard pushers       = shift(promoters, up)  & emptySquares;
    Bitboard capturesLeft  = shift(promoters, upLeft)  & captures;
    Bitboard capturesRight = shift(promoters, upRight) & captures;

    while (pushers)
    {
        Square to = popFirstSquare(pushers);
        Square from = to - up;
        addPromotionMove(moveList, from, to);
    }

    while (capturesLeft)
    {
        Square to = popFirstSquare(capturesLeft);
        Square from = to - upLeft;
        addPromotionMove(moveList, from, to);
    }

    while (capturesRight)
    {
        Square to = popFirstSquare(capturesRight);
        Square from = to - upRight;
        addPromotionMove(moveList, from, to);
    }
}

void generatePawnCaptures(const Position& pos, MoveList& moveList)
{
    Color us = pos.SideToMove();
    Color them = ~us;

    Bitboard notOnPromotionRank = ~getRankMask(relativeRank(RANK_7, us));
    Bitboard pawns = pos.Pieces(PAWN, us) & notOnPromotionRank;

    if (!pawns)
        return;
    
    Direction upLeft  = (us == WHITE ? NORTH_WEST : SOUTH_EAST);
    Direction upRight = (us == WHITE ? NORTH_EAST : SOUTH_WEST);

    Bitboard captures = pos.Pieces(them);
    Bitboard capturesLeft  = shift(pawns, upLeft)  & captures;
    Bitboard capturesRight = shift(pawns, upRight) & captures;

    // normal captures
    while (capturesLeft)
    {
        Square target = popFirstSquare(capturesLeft);
        Square from = target - upLeft;
        addMove(moveList, makeQuietMove(from, target));
    }
    
    while (capturesRight)
    {
        Square target = popFirstSquare(capturesRight);
        Square from = target - upRight;
        addMove(moveList, makeQuietMove(from, target));
    }

    // en passant captures
    Square epSq = pos.EnpassantSquare();

    if (epSq == NO_SQUARE)
        return;

    Bitboard epAttackers = pawnAttackMask(them, epSq) & pawns;

    while (epAttackers)
    {
        Square from = popFirstSquare(epAttackers);
        addMove(moveList, makeMove(from, epSq, EN_PASSANT));
    }
}

inline void addMove(MoveList& moveList, Move move)
{
    moveList.moves[moveList.count].move = move;
    moveList.count++;
}

inline void addPromotionMove(MoveList& moveList, Square from, Square to)
{
    addMove(moveList, makeMove(from, to, PROMOTION, KNIGHT));
    addMove(moveList, makeMove(from, to, PROMOTION, BISHOP));
    addMove(moveList, makeMove(from, to, PROMOTION, ROOK));
    addMove(moveList, makeMove(from, to, PROMOTION, QUEEN));
}

} // anonymous namespace

} // namesapce ChessEngine