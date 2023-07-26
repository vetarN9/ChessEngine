#include "movegen.h"
#include "position.h"
#include <iostream>

namespace ChessEngine {

namespace {  // anonymous namespace

// Helper functions for generating all legal moves in the current position

void generateKingMoves (const Position& pos, MoveList& moveList, GenType genType);
void generatePawnMoves (const Position& pos, MoveList& moveList, GenType genType);
void generatePieceMoves(const Position& pos, MoveList& moveList, PieceType pt, GenType genType);

inline void addMove(MoveList& moveList, Move move);
inline void addPromotionMove(MoveList& moveList, Square from, Square to);
inline Bitboard legalSquares(const Position& pos);

} // anonymous namespace

void generateMoves(const Position& pos, MoveList& moveList, GenType genType /*= ALL*/)
{
    generateKingMoves(pos, moveList, genType);

    // Only king moves are legal if in double check
    if (moreThanOne(pos.Checkers()))
        return;

    generatePawnMoves(pos, moveList, genType);

    for (PieceType pt : {KNIGHT, BISHOP, ROOK, QUEEN})
        generatePieceMoves(pos, moveList, pt, genType);
}

namespace {  // anonymous namespace

void generateKingMoves(const Position& pos, MoveList& moveList, GenType genType)
{
    Color us = pos.SideToMove();
    Color them = ~us;

    Square kingSq = pos.KingSquare(us);
    uint8_t cr = pos.CastlingRights();
    
    Bitboard kingMoves = kingAttackMask(getSquareMask(kingSq)) & ~pos.Pieces(us);

    if (genType == CAPTURES)
        kingMoves &= pos.Pieces(them);

    while (kingMoves)
    {
        Square to = popSquare(kingMoves);

        // Verify that the "to" square is not attacked by the other side
        if (!(pos.AttackersTo(to, pos.Pieces() ^ kingSq) & pos.Pieces(them)))
            addMove(moveList, createMove(kingSq, to));
    }

    // Verify castling availability
    if (pos.Checkers() || !canCastle(us, cr) || genType == CAPTURES)
        return;

    Bitboard b1 = getSquareMask(relativeSquare(B1, us));
    Bitboard c1 = getSquareMask(relativeSquare(C1, us));
    Bitboard d1 = getSquareMask(relativeSquare(D1, us));
    Bitboard f1 = getSquareMask(relativeSquare(F1, us));
    Bitboard g1 = getSquareMask(relativeSquare(G1, us));

    Bitboard shortMask = f1 | g1;
    Bitboard longMask  = b1 | c1 | d1;

    uint8_t shortCastle = (us == WHITE ? WHITE_SHORT : BLACK_SHORT);
    uint8_t longCastle  = (us == WHITE ? WHITE_LONG  : BLACK_LONG);

    // Verify that short castle is legal
    if ((shortCastle & cr) && !(pos.Pieces() & shortMask) && pos.SquaresNotAttacked(shortMask, them))
        addMove(moveList, createMoveWithFlags(kingSq, relativeSquare(G1, us), CASTLING));
    
    // Verify that long castle is legal                                Remove B1 from is attacked check
    if ((longCastle  & cr) && !(pos.Pieces() & longMask) && pos.SquaresNotAttacked(longMask ^ b1, them))
        addMove(moveList, createMoveWithFlags(kingSq, relativeSquare(C1, us), CASTLING));
}

void generatePawnMoves(const Position& pos, MoveList& moveList, GenType genType)
{
    Color us = pos.SideToMove();
    Color them = ~us;

    Square kingSq = pos.KingSquare(us);

    Bitboard doublePushRank = getRankMask(relativeRank(RANK_3, us));
    Bitboard promotionRank  = getRankMask(relativeRank(RANK_7, us));
    Bitboard emptySquares   = ~pos.Pieces();
    Bitboard pawns          = pos.Pieces(PAWN, us) & ~promotionRank;
    Bitboard promoters      = pos.Pieces(PAWN, us) & promotionRank;
    Bitboard enemies        = pos.Pieces(them);
    Bitboard targets        = legalSquares(pos);
    Bitboard emptyTargets   = targets & emptySquares;
    Bitboard captureTargets = targets & enemies;
    Bitboard pinned         = pos.Pinned(us);

    Direction up = getPawnDir(us);
    Direction upLeft  = (us == WHITE ? NORTH_WEST : SOUTH_EAST);
    Direction upRight = (us == WHITE ? NORTH_EAST : SOUTH_WEST);

    // Single and double pawn pushes
    if (genType != CAPTURES)
    {
        Bitboard singlePush = shift(pawns, up) & emptySquares;
        Bitboard doublePush = shift(singlePush & doublePushRank, up) & emptyTargets;

        singlePush &= targets;
        
        while (singlePush)
        {
            Square to = popSquare(singlePush);
            Square from = to - up;

            if (!(pinned & from) || isAligned(from, to, kingSq))
                addMove(moveList, createMove(from, to));
        }

        while (doublePush)
        {
            Square to = popSquare(doublePush);
            Square from = to - up - up;

            if (!(pinned & from) || isAligned(from, to, kingSq))
                addMove(moveList, createMove(from, to));
        }
    }

    // Promotion moves (Includes captures on rank 8)
    if (promoters)
    {
        Bitboard upPromoters   = shift(promoters, up)      & emptyTargets;
        Bitboard capturesLeft  = shift(promoters, upLeft)  & captureTargets;
        Bitboard capturesRight = shift(promoters, upRight) & captureTargets;

        if (genType == CAPTURES)
            upPromoters = 0;

        while (upPromoters)
        {
            Square to = popSquare(upPromoters);
            Square from = to - up;

            if (!(pinned & from) || isAligned(from, to, kingSq))
                addPromotionMove(moveList, from, to);
        }

        while (capturesLeft)
        {
            Square to = popSquare(capturesLeft);
            Square from = to - upLeft;

            if (!(pinned & from) || isAligned(from, to, kingSq))
                addPromotionMove(moveList, from, to);
        }

        while (capturesRight)
        {
            Square to = popSquare(capturesRight);
            Square from = to - upRight;

            if (!(pinned & from) || isAligned(from, to, kingSq))
                addPromotionMove(moveList, from, to);
        }
    }

    // Capture and en passant moves
    if (!pawns)
        return;

    Bitboard capturesLeft  = shift(pawns, upLeft)  & captureTargets;
    Bitboard capturesRight = shift(pawns, upRight) & captureTargets;

    // normal captures
    while (capturesLeft)
    {
        Square to = popSquare(capturesLeft);
        Square from = to - upLeft;

        if (!(pinned & from) || isAligned(from, to, kingSq))
            addMove(moveList, createMove(from, to));
    }
    
    while (capturesRight)
    {
        Square to = popSquare(capturesRight);
        Square from = to - upRight;

        if (!(pinned & from) || isAligned(from, to, kingSq))
            addMove(moveList, createMove(from, to));
    }

    // en passant captures
    Square epSq = pos.EnpassantSquare();

    // Cannot en passant if in check and epSq doesn't resolve the check
    if (epSq == NO_SQUARE)
        return;

    Bitboard epAttackers = pawnAttackMask(them, epSq) & pawns;

    while (epAttackers)
    {
        Square from = popSquare(epAttackers);
        
        // It's tricky to check if ep move is legal. 
        // Verify that the king is not in check after the move has been made

        // Blockers represent the position after the en passant move is made
        Bitboard blockers = (pos.Pieces() ^ from ^ (epSq - up)) | epSq;

        bool rookAttacked   = attackMask(ROOK,   kingSq, blockers) & pos.Pieces(ROOK,   QUEEN) & pos.Pieces(them);
        bool bishopAttacked = attackMask(BISHOP, kingSq, blockers) & pos.Pieces(BISHOP, QUEEN) & pos.Pieces(them);

        if (!rookAttacked && !bishopAttacked)
            addMove(moveList, createMoveWithFlags(from, epSq, EN_PASSANT));
    }
}

void generatePieceMoves(const Position& pos, MoveList& moveList, PieceType pt, GenType genType)
{
    Color us = pos.SideToMove();

    Bitboard pieces = pos.Pieces(pt, us);

    // If in check, only consider squares that resolves the check
    Bitboard possibleMoves = legalSquares(pos);

    if (genType == CAPTURES)
        possibleMoves &= pos.Pieces(~us);

    while (pieces)
    {
        Square from      = popSquare(pieces);
        Bitboard attacks = attackMask(pt, from, pos.Pieces()) & possibleMoves;
        bool pinned      = pos.Pinned(us) & from;

        // A pinned knight can never move
        if (pt == KNIGHT && pinned)
            continue;

        while (attacks)
        {
            Square to = popSquare(attacks);

            if (!pinned || isAligned(from, to, pos.KingSquare(us)))
                addMove(moveList, createMove(from, to));
        }
    }
}

inline void addMove(MoveList& moveList, Move move)
{
    moveList.moves[moveList.count].move = move;
    moveList.count++;
}

inline void addPromotionMove(MoveList& moveList, Square from, Square to)
{
    addMove(moveList, createMoveWithFlags(from, to, PROMOTION, KNIGHT));
    addMove(moveList, createMoveWithFlags(from, to, PROMOTION, BISHOP));
    addMove(moveList, createMoveWithFlags(from, to, PROMOTION, ROOK));
    addMove(moveList, createMoveWithFlags(from, to, PROMOTION, QUEEN));
}

// If in check, returns the squares that resolves the check,
// Otherwise, return all squares not occupied by the side to move
inline Bitboard legalSquares(const Position& pos)
{
    Color    us = pos.SideToMove();
    Bitboard ch = pos.Checkers();
    Square  kSq = pos.KingSquare(us);

    return (ch ? getBetweenMask(kSq, getSquare(ch)) : ~pos.Pieces(us));
}

} // anonymous namespace

} // namesapce ChessEngine