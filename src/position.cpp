#include <iostream>
#include <cstring>
#include <sstream>
#include <string_view>
#include <map>

#include "position.h"

namespace ChessEngine {

namespace {  // anonymous namespace

constexpr std::string_view PieceToAscii(" PNBRQK  pnbrqk");

} // anonymous namespace

void Position::Init()
{

}


/*
    A FEN record defines a particular game position, all in one text line and using only the ASCII character set. 
    A text file with only FEN data records should use the filename extension .fen.

    A FEN record contains six fields, each separated by a space. The fields are as follows:

    1. Piece placement data: Each rank is described, starting with rank 8 and ending with rank 1, with a "/" 
       between each one; within each rank, the contents of the squares are described in order from the a-file 
       to the h-file. Each piece is identified by a single letter taken from the standard English names in algebraic
       notation (pawn = "P", knight = "N", bishop = "B", rook = "R", queen = "Q" and king = "K"). White Occupancies are
       designated using uppercase letters ("PNBRQK"), while black Occupancies use lowercase letters ("pnbrqk"). 
       A set of one or more consecutive empty squares within a rank is denoted by a digit from "1" to "8",
       corresponding to the number of squares.
    
    2. Active color: "w" means that White is to move; "b" means that Black is to move.
    
    3. Castling availability: If neither side has the ability to castle, this field uses the character "-".
       Otherwise, this field contains one or more letters: "K" if White can castle kingside, "Q" if White can 
       castle queenside, "k" if Black can castle kingside, and "q" if Black can castle queenside. A situation that 
       temporarily prevents castling does not prevent the use of this notation.
    
    4. En passant target square: This is a square over which a pawn has just passed while moving two squares; 
       it is given in algebraic notation. If there is no en passant target square, this field uses the character "-". 
       This is recorded regardless of whether there is a pawn in position to capture en passant.[6] An updated 
       version of the spec has since made it so the target square is only recorded if a legal en passant move is 
       possible but the old version of the standard is the one most commonly used.
    
    5. Halfmove clock: The number of halfmoves since the last capture or pawn advance, used for the fifty-move rule.
    
    6. Fullmove number: The number of the full moves. It starts at 1 and is incremented after Black's move.

    Example:
        FEN starting posiion: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
*/

Position& Position::Set(const std::string& fen, PosInfo* posInfo)
{
    *this = Position();
    *posInfo = PosInfo();
    this->posInfo = posInfo;

    std::istringstream ss(fen);
    ss >> std::noskipws;
    
    ParsePiecePlacement(ss);  // 1. Piece placement data
    ParseActiveColor(ss);     // 2. Active color
    ParseCastling(ss);        // 3. Castling availability
    ParseEnpassantSquare(ss); // 4. En passant target square
    ParseMoveCounters(ss);    // 5-6. Halfmove clock and Fullmove number

    SetCheckingData();

    return *this;
}

void Position::ParsePiecePlacement(std::istringstream& ss)
{
    uint8_t token;
    int square = A8; // FEN reads from left to right starting from the top rank

    while ((ss >> token) && !isspace(token))
    {
        if (isdigit(token))
            square += (token - '0'); 

        else if (token == '/')
            square += 2*SOUTH;
        
        else
        {
            PlacePiece(Piece(PieceToAscii.find(token)), Square(square));
            square++;
        }
    }
}

void Position::ParseActiveColor(std::istringstream& ss)
{
    uint8_t color;

    ss >> color;
    sideToMove = (color == 'w') ? WHITE : BLACK;
    ss >> color;
}

void Position::ParseCastling(std::istringstream& ss)
{
    uint8_t castleSide;

    while ((ss >> castleSide) && !isspace(castleSide))
    {
        CastlingRight cr = NO_CASTLING;

        switch (castleSide)
        {
            // TODO: Check if valid castling right
            case 'K': cr = WHITE_SHORT; break;
            case 'Q': cr = WHITE_LONG;  break;
            case 'k': cr = BLACK_SHORT; break;
            case 'q': cr = BLACK_LONG;  break;
        }

        SetCastlingRights(cr);
    }
}

void Position::ParseEnpassantSquare(std::istringstream& ss)
{
    posInfo->enpassantSquare = NO_SQUARE;

    uint8_t file, rank;
    ss >> file;

    if ((file != '-') && (ss >> rank))
    {
        Square epSq = createSquare(File(file - 'a'), Rank(rank - '1'));

        // TODO: Check if valid en passant square
        if (true)
            posInfo->enpassantSquare = epSq;
    }
}

void Position::ParseMoveCounters(std::istringstream& ss)
{
    ss >> std::skipws >> posInfo->fiftyMoveCounter;

    int fullmove;
    ss >> fullmove;
    ply = 2 * (fullmove - 1) + (sideToMove == BLACK);
}

void Position::SetCheckingData()
{
    Color us = sideToMove;
    Color them = ~us;

    posInfo->checkersBoard = AttackersTo(KingSquare(us)) & Pieces(them);

    Bitboard ourKingBlockers   = SliderBlockers(us,   KingSquare(us),   posInfo->pinners[them]);
    Bitboard theirKingBlockers = SliderBlockers(them, KingSquare(them), posInfo->pinners[us]);

    posInfo->pinned[us]      = ourKingBlockers   & Pieces(us);
    posInfo->pinned[them]    = theirKingBlockers & Pieces(them);
    posInfo->discovery[us]   = theirKingBlockers & Pieces(us);
    posInfo->discovery[them] = ourKingBlockers   & Pieces(them);

    Square target = KingSquare(them);

    posInfo->checkSquares[PAWN]   = pawnAttackMask(them, target);
    posInfo->checkSquares[KNIGHT] = attackMask(KNIGHT, target);
    posInfo->checkSquares[BISHOP] = attackMask(BISHOP, target, Pieces());
    posInfo->checkSquares[ROOK]   = attackMask(ROOK,   target, Pieces());
    posInfo->checkSquares[QUEEN]  = posInfo->checkSquares[BISHOP] | posInfo->checkSquares[ROOK];
}

Bitboard Position::SliderBlockers(Color blocker, Square target, Bitboard& pinners) const
{
    pinners = 0;
    Bitboard attackers = Pieces(~blocker);

    Bitboard rookAttackersToTarget   = Pieces(ROOK,   QUEEN) & attackMask(ROOK,   target);
    Bitboard bishopAttackersToTarget = Pieces(BISHOP, QUEEN) & attackMask(BISHOP, target);
    Bitboard sliders   = (rookAttackersToTarget | bishopAttackersToTarget) & attackers;
    Bitboard occupancy = Pieces() ^ sliders;
    Bitboard blockers  = 0;

    while (sliders)
    {
        Square slider = popSquare(sliders);
        Bitboard between = getBetweenMask(target, slider) & occupancy;

        if (between && !moreThanOne(between))
        {
            if (between & Pieces(blocker))
                pinners |= slider;

            blockers |= between;
        }
    }
    return blockers;
}

std::string Position::FEN() const
{
    std::ostringstream oss;

    // 1. Piece placement data
    int numEmptySpaces ;

    for (Rank rank = RANK_8; rank >= RANK_1; rank--)
    {
        for (File file = FILE_A; file < NUM_FILES; file++)
        {
            for (numEmptySpaces = 0; file < NUM_FILES && (PieceOn(createSquare(file, rank)) == EMPTY); file++)
                numEmptySpaces++;

            if (numEmptySpaces)
                oss << numEmptySpaces;
            
            if (file < NUM_FILES)
                oss << PieceToAscii[PieceOn(createSquare(File(file), Rank(rank)))];
        }

        if (rank > RANK_1)
            oss << '/';
    }

    // 2. Active color
    oss << (sideToMove == WHITE ? " w " : " b ");

    // 3. Castling availability
    if (posInfo->castlingRights == NO_CASTLING)
        oss << "-";
    
    else
    {
        static constexpr char CastlingToFEN[] = {'-', 'K', 'Q', '-', 'k', '-', '-', '-', 'q'};
        
        for (int i = WHITE_SHORT; i <= BLACK_LONG; i <<= 1)
        {
            if (posInfo->castlingRights & i)
                oss << CastlingToFEN[i];
        }
    }

    // 4. En passant target square
    oss << (posInfo->enpassantSquare == NO_SQUARE ? " - " : ' ' + algebraicNotation(posInfo->enpassantSquare) + ' ');

    // 5. Halfmove clock
    oss << posInfo->fiftyMoveCounter;

    // 6. Fullmove number
    int fullmove = 1 + ((ply - (sideToMove == BLACK)) / 2);
    oss << ' ' << fullmove;

    return oss.str();
}

// Returns the squares that contain a piece that attacks the given square
Bitboard Position::AttackersTo(Square square, Bitboard occupancy) const 
{
    return (pawnAttackMask(WHITE, square)         & Pieces(PAWN, BLACK))
         | (pawnAttackMask(BLACK, square)         & Pieces(PAWN, WHITE))
         | (attackMask(KNIGHT, square)            & Pieces(KNIGHT))
         | (attackMask(ROOK, square, occupancy)   & Pieces(ROOK, QUEEN))
         | (attackMask(BISHOP, square, occupancy) & Pieces(BISHOP, QUEEN))
         | (attackMask(KING, square)              & Pieces(KING));
}

bool Position::SquaresNotAttacked(Bitboard bitboard, Color attacker) const
{
    while (bitboard)
    {
        if (SquareIsAttacked(popSquare(bitboard), attacker))
            return false;
    }

    return true;
}

void Position::MakeMove(Move move, PosInfo& newPosInfo)
{
    std::memcpy(&newPosInfo, posInfo, sizeof(PosInfo));
    newPosInfo.prev = posInfo;
    posInfo = &newPosInfo;

    Color us = sideToMove;
    Color them = ~us;

    Square from         = getFromSquare(move);
    Square to           = getToSquare(move);
    MoveType moveType   = getMoveType(move);
    Direction pawnDir   = getPawnDir(us);
    Piece movedPiece    = PieceOn(from);
    Piece capturedPiece = (moveType != EN_PASSANT ? PieceOn(to) : getPiece(PAWN, them));
    PieceType pt = getType(movedPiece);

    // Update move counters
    ply++;
    posInfo->fiftyMoveCounter++;
    posInfo->movesFromNull++;

    if (moveType == CASTLING)
    {
        MakeCastling(move);
        capturedPiece = EMPTY;
    }
    
    // If there was a capture, remove the captured piece and
    // reset the fifty move counter
    if (capturedPiece)
    {
        Square capturedSq = to;

        if (moveType == EN_PASSANT)
            capturedSq -= pawnDir;
        
        RemovePiece(capturedSq);
        posInfo->fiftyMoveCounter = 0;
    }

    // Update castling rights if it has changed
    if (posInfo->castlingRights &&  (castlingRightsMask[from] | castlingRightsMask[to]))
        posInfo->castlingRights &= ~(castlingRightsMask[from] | castlingRightsMask[to]);

    // Move the piece
    if (moveType != CASTLING)
        MovePiece(from, to);
    
    // Reset the en passant square
    posInfo->enpassantSquare = NO_SQUARE;

    if (pt == PAWN)
    {
        // Set en passant square if double pawn push that is attacked on the square behind the pawn
        if ((int(to) ^ int(from)) == 16 && (pawnAttackMask(us, to - pawnDir) & Pieces(PAWN, them)))
            posInfo->enpassantSquare = to - pawnDir;

        if (moveType == PROMOTION)
        {
            Piece promomotionPiece = getPiece(getPromotionType(move), us);
            RemovePiece(to);
            PlacePiece(promomotionPiece, to);
        }
        
        posInfo->fiftyMoveCounter = 0;
    }
      
    posInfo->capturedPiece = capturedPiece;
    sideToMove = ~sideToMove;

    SetCheckingData();
}

void Position::UndoMove(Move move)
{
    sideToMove = ~sideToMove;

    Color us          = sideToMove;
    Square from       = getFromSquare(move);
    Square to         = getToSquare(move);
    MoveType moveType = getMoveType(move);
    Direction pawnDir = getPawnDir(us);

    // Replace promoted piece with a pawn;
    if (moveType == PROMOTION)
    {
        RemovePiece(to);
        PlacePiece(getPiece(PAWN, us), to);
    }
    
    if (moveType == CASTLING)
        UndoCastling(move);
    
    else 
    {
        MovePiece(to, from);

        // Restore captured piece
        if (posInfo->capturedPiece)
        {
            Square capturedSq = to;

            if (moveType == EN_PASSANT)
                capturedSq -= pawnDir;

            PlacePiece(posInfo->capturedPiece, capturedSq);
        }
    }

    posInfo = posInfo->prev;
    ply--;
}

void Position::MakeCastling(Move move)
{
    Color us = sideToMove;

    Square kingFrom = getFromSquare(move);
    Square kingTo   = getToSquare(move);

    bool kingSide = kingFrom < kingTo;

    Square rookFrom = relativeSquare(kingSide ? H1 : A1, us);
    Square rookTo   = relativeSquare(kingSide ? F1 : D1, us);

    MovePiece(kingFrom, kingTo);
    MovePiece(rookFrom, rookTo);
}

void Position::UndoCastling(Move move)
{
    Color us = sideToMove;

    Square kingFrom = getFromSquare(move);
    Square kingTo   = getToSquare(move);

    bool kingSide = kingFrom < kingTo;

    Square rookFrom = relativeSquare(kingSide ? H1 : A1, us);
    Square rookTo   = relativeSquare(kingSide ? F1 : D1, us);

    MovePiece(kingTo, kingFrom);
    MovePiece(rookTo, rookFrom);
}

void Position::PlacePiece(Piece piece, Square square)
{
    Bitboard squareMask = getSquareMask(square);
    pieceOnSquare[square] = piece;

    typeBoard[ALL_PIECES]       |= squareMask;
    typeBoard[getType(piece)]   |= squareMask;
    colorBoard[getColor(piece)] |= squareMask;

    numPieces[piece]++;
    numPieces[getPiece(ALL_PIECES, getColor(piece))]++;
}

void Position::MovePiece(Square from, Square to)
{
    Bitboard moveMask = getSquareMask(from) | getSquareMask(to);
    Piece piece = pieceOnSquare[from];

    pieceOnSquare[from] = EMPTY;
    pieceOnSquare[to] = piece;

    typeBoard[ALL_PIECES]       ^= moveMask;
    typeBoard[getType(piece)]   ^= moveMask;
    colorBoard[getColor(piece)] ^= moveMask;
}

void Position::RemovePiece(Square square)
{
    assert(PieceOn(square) != EMPTY);
    Bitboard squareMask = getSquareMask(square);
    Piece piece = pieceOnSquare[square];
    pieceOnSquare[square] = EMPTY;

    typeBoard[ALL_PIECES]       ^= squareMask;
    typeBoard[getType(piece)]   ^= squareMask;
    colorBoard[getColor(piece)] ^= squareMask;

    numPieces[piece]--;
    numPieces[getPiece(ALL_PIECES, getColor(piece))]--;
}

void Position::SetCastlingRights(CastlingRight cr)
{
    Color color   = (cr & WHITE_CASTLING ? WHITE : BLACK);
    Square kingSq = KingSquare(color);
    Square rookSq = (cr & QUEEN_SIDE ? relativeSquare(A1, color)
                                     : relativeSquare(H1, color));

    posInfo->castlingRights    |= cr;
    castlingRightsMask[kingSq] |= cr;
    castlingRightsMask[rookSq] |= cr;
}

void Position::Print()
{
    #ifdef _WIN64

    constexpr std::string_view pieceToChar = PieceToAscii;

    #else

    constexpr const char* pieceToChar[] = {" ", u8"♟", u8"♞", u8"♝", u8"♜", u8"♛", u8"♚", " ", " ", u8"♙", u8"♘", u8"♗", u8"♖", u8"♕", u8"♔"};
    
    #endif

    std::cout << "  +---+---+---+---+---+---+---+---+\n";

    for (Rank rank = RANK_8; rank >= RANK_1; rank--)
    {
        std::cout << rank+1 << " ";
        for (File file = FILE_A; file < NUM_FILES; file++)
        {
            std::cout << "| " << pieceToChar[pieceOnSquare[createSquare(file, rank)]] << " ";
        }

        std::cout << "|\n  +---+---+---+---+---+---+---+---+\n";
    }
    
    std::cout << "    a   b   c   d   e   f   g   h\n\n";
    
    std::cout << "        Side:         "   << (sideToMove ? "black" : "white") << "\n";
    std::cout << "        enpassant:    "   << ((posInfo->enpassantSquare != NO_SQUARE)  ? algebraicNotation(posInfo->enpassantSquare) : "no") << "\n";
    std::cout << "        Castling:     "   << ((posInfo->castlingRights & WHITE_SHORT)  ? 'K' : '-')
                                            << ((posInfo->castlingRights & WHITE_LONG)   ? 'Q' : '-')
                                            << ((posInfo->castlingRights & BLACK_SHORT)  ? 'k' : '-')
                                            << ((posInfo->castlingRights & BLACK_LONG)   ? 'q' : '-')
                                            << "\n" << std::endl;
}

} // namespace ChessEngine