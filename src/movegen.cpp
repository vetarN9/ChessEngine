#include "movegen.h"
#include "position.h"

namespace ChessEngine {

namespace {

void generateQuietPawnMoves(const Position& pos, MoveList& moveList, Bitboard targets);
void generatePawnPromotionMoves(const Position& pos, MoveList& moveList, Bitboard targets);
void generatePawnCaptures(const Position& pos, MoveList& moveList, Bitboard targets);

void generatePieceMoves(PieceType pt, const Position& pos, MoveList& moveList, Bitboard targets);

inline void addMove(MoveList& moveList, Move move);
inline void addPromotionMove(MoveList& moveList, Square from, Square to);

} // anonymous namespace


void generateMoves(const Position& pos, MoveList& moveList)
{
    Color us = pos.SideToMove();
    Color them = ~us;

    Square kingSq = pos.KingSquare(us);
    
    Bitboard checkers = pos.Checkers();


    // Only king moves are legal if in double check
    if (moreThanOne(checkers))
    {
        Bitboard kingMoves = kingAttackMask(getSquareMask(kingSq)) & ~pos.Pieces(us);

        while (kingMoves)
            addMove(moveList, makeMove(kingSq, popSquare(kingMoves)));
        
        return;
    }

    Bitboard targets = (checkers ? getBetweenMask(kingSq, getSquare(checkers)) : ~pos.Pieces(us));

    generateQuietPawnMoves(pos, moveList, targets);
    generatePawnPromotionMoves(pos, moveList,targets);
    generatePawnCaptures(pos, moveList, targets);

    for (PieceType pt : {KNIGHT, BISHOP, ROOK, QUEEN})
        generatePieceMoves(pt, pos, moveList, targets);
    
    Bitboard kingMoves = kingAttackMask(getSquareMask(kingSq)) & ~pos.Pieces(us);

    while (kingMoves)
        addMove(moveList, makeMove(kingSq, popSquare(kingMoves)));
}


namespace {

void generateQuietPawnMoves(const Position& pos, MoveList& moveList, Bitboard targets)
{
    Color us = pos.SideToMove();

    Bitboard promotionRank  = getRankMask(relativeRank(RANK_7, us));
    Bitboard doublePushRank = getRankMask(relativeRank(RANK_3, us));
    Bitboard emptySquares   = ~pos.Pieces();
    Bitboard pushers        = pos.Pieces(PAWN, us) & ~promotionRank;
    Direction up            = getPawnDir(us);

    targets &= emptySquares;

    Bitboard singlePush = shift(pushers, up)  & emptySquares;
    Bitboard doublePush = shift(singlePush & doublePushRank, up) & targets;
    singlePush &= targets;

    while (singlePush)
    {
        Square to = popSquare(singlePush);
        Square from = to - up;
        addMove(moveList, makeMove(from, to));
    }

    while (doublePush)
    {
        Square to = popSquare(doublePush);
        Square from = to - up - up;
        addMove(moveList, makeMove(from, to));
    }
}

void generatePawnPromotionMoves(const Position& pos, MoveList& moveList, Bitboard targets)
{
    Color us = pos.SideToMove();
    Color them = ~us;

    Bitboard promotionRank  = getRankMask(relativeRank(RANK_7, us));
    Bitboard promoters      =  pos.Pieces(PAWN, us) & promotionRank;

    if (!promoters)
        return;

    Bitboard captures = pos.Pieces(them) & targets;
    Bitboard emptySquares  = ~pos.Pieces();

    Direction up = getPawnDir(us);
    Direction upLeft  = (us == WHITE ? NORTH_WEST : SOUTH_EAST);
    Direction upRight = (us == WHITE ? NORTH_EAST : SOUTH_WEST);

    Bitboard pushers       = shift(promoters, up)  & emptySquares & targets;
    Bitboard capturesLeft  = shift(promoters, upLeft)  & captures;
    Bitboard capturesRight = shift(promoters, upRight) & captures;

    while (pushers)
    {
        Square to = popSquare(pushers);
        Square from = to - up;
        addPromotionMove(moveList, from, to);
    }

    while (capturesLeft)
    {
        Square to = popSquare(capturesLeft);
        Square from = to - upLeft;
        addPromotionMove(moveList, from, to);
    }

    while (capturesRight)
    {
        Square to = popSquare(capturesRight);
        Square from = to - upRight;
        addPromotionMove(moveList, from, to);
    }
}

void generatePawnCaptures(const Position& pos, MoveList& moveList, Bitboard targets)
{
    Color us = pos.SideToMove();
    Color them = ~us;

    Bitboard notOnPromotionRank = ~getRankMask(relativeRank(RANK_7, us));
    Bitboard pawns = pos.Pieces(PAWN, us) & notOnPromotionRank;

    if (!pawns)
        return;
    
    Direction upLeft  = (us == WHITE ? NORTH_WEST : SOUTH_EAST);
    Direction upRight = (us == WHITE ? NORTH_EAST : SOUTH_WEST);

    Bitboard captures = pos.Pieces(them) & targets;
    Bitboard capturesLeft  = shift(pawns, upLeft)  & captures;
    Bitboard capturesRight = shift(pawns, upRight) & captures;

    // normal captures
    while (capturesLeft)
    {
        Square target = popSquare(capturesLeft);
        Square from = target - upLeft;
        addMove(moveList, makeMove(from, target));
    }
    
    while (capturesRight)
    {
        Square target = popSquare(capturesRight);
        Square from = target - upRight;
        addMove(moveList, makeMove(from, target));
    }

    // en passant captures
    Square epSq = pos.EnpassantSquare();

    if (epSq == NO_SQUARE)
        return;

    // Cannot en passant if in check and epSq doesn't resolve the check
    if (!(getSquareMask(epSq) & targets))
        return;
    
    Bitboard epAttackers = pawnAttackMask(them, epSq) & pawns;

    while (epAttackers)
        addMove(moveList, makeMoveWithFlags(popSquare(epAttackers), epSq, EN_PASSANT));
}

void generatePieceMoves(PieceType pt, const Position& pos, MoveList& moveList, Bitboard targets)
{
    Color us = pos.SideToMove();

    Bitboard pieces = pos.Pieces(pt, us);
    Bitboard blockers = pos.Pieces();

    while (pieces)
    {
        Square from = popSquare(pieces);
        Bitboard attacks = attackMask(pt, from, blockers) & targets;

        while (attacks)
            addMove(moveList, makeMove(from, popSquare(attacks)));
    }
}

inline void addMove(MoveList& moveList, Move move)
{
    moveList.moves[moveList.count].move = move;
    moveList.count++;
}

inline void addPromotionMove(MoveList& moveList, Square from, Square to)
{
    addMove(moveList, makeMoveWithFlags(from, to, PROMOTION, KNIGHT));
    addMove(moveList, makeMoveWithFlags(from, to, PROMOTION, BISHOP));
    addMove(moveList, makeMoveWithFlags(from, to, PROMOTION, ROOK));
    addMove(moveList, makeMoveWithFlags(from, to, PROMOTION, QUEEN));
}

} // anonymous namespace

} // namesapce ChessEngine