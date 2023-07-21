#include <iostream>
#include <cstring>
#include <sstream>
#include <string_view>
#include <map>

#include "position.h"

namespace ChessEngine {

namespace {

constexpr std::string_view PieceToChar(" PNBRQK  pnbrqk");

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

Position& Position::Set(const std::string& fen)
{
    *this = Position();

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
            PlacePiece(Piece(PieceToChar.find(token)), Square(square));
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
        switch (castleSide)
        {
            // TODO: Check if valid castling right
            case 'K': castlingRights |= WHITE_SHORT; break;
            case 'Q': castlingRights |= WHITE_LONG;  break;
            case 'k': castlingRights |= BLACK_SHORT; break;
            case 'q': castlingRights |= BLACK_LONG;  break;
        }
    }
}

void Position::ParseEnpassantSquare(std::istringstream& ss)
{
    enpassantSquare = NO_SQUARE;

    uint8_t file, rank;
    ss >> file;

    if ((file != '-') && (ss >> rank))
    {
        Square epSq = createSquare(File(file - 'a'), Rank(rank - '1'));

        // TODO: Check if valid en passant square
        if (true)
            enpassantSquare = epSq;
    }
}

void Position::ParseMoveCounters(std::istringstream& ss)
{
    ss >> std::skipws >> fiftyMoveCounter;

    int fullmove;
    ss >> fullmove;
    ply = 2 * (fullmove - 1) + (sideToMove == BLACK);
}

void Position::SetCheckingData()
{
    Color us = sideToMove;
    Color them = ~us;

    checkersBoard = AttacksTo(KingSquare(us)) & Pieces(them);
    
    kingBlockers[us]   = SliderBlockers(us, KingSquare(us), pinners[them]);
    kingBlockers[them] = SliderBlockers(them, KingSquare(them), pinners[us]);

    pinned[us]      = kingBlockers[us]   & Pieces(us);
    pinned[them]    = kingBlockers[them] & Pieces(them);
    discovery[us]   = kingBlockers[them] & Pieces(us);
    discovery[them] = kingBlockers[us]   & Pieces(them);

    Square target = KingSquare(them);

    checkSquares[PAWN]   = pawnAttackMask(them, target);
    checkSquares[KNIGHT] = attackMask(KNIGHT, target);
    checkSquares[BISHOP] = attackMask(BISHOP, target, Pieces());
    checkSquares[ROOK]   = attackMask(ROOK,   target, Pieces());
    checkSquares[QUEEN]  = checkSquares[BISHOP] | checkSquares[ROOK];
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
        Square slider = popFirstSquare(sliders);
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
                oss << PieceToChar[PieceOn(createSquare(File(file), Rank(rank)))];
        }

        if (rank > RANK_1)
            oss << '/';
    }

    // 2. Active color
    oss << (sideToMove == WHITE ? " w " : " b ");

    // 3. Castling availability
    if (castlingRights == NO_CASTLING)
        oss << "-";
    
    else
    {
        static constexpr char CastlingToFEN[] = {'-', 'K', 'Q', '-', 'k', '-', '-', '-', 'q'};
        
        for (int i = WHITE_SHORT; i <= BLACK_LONG; i <<= 1)
        {
            if (castlingRights & i)
                oss << CastlingToFEN[i];
        }
    }

    // 4. En passant target square
    oss << (enpassantSquare == NO_SQUARE ? " - " : ' ' + algebraicNotation(enpassantSquare) + ' ');

    // 5. Halfmove clock
    oss << fiftyMoveCounter;

    // 6. Fullmove number
    int fullmove = 1 + ((ply - (sideToMove == BLACK)) / 2);
    oss << ' ' << fullmove;

    return oss.str();
}

// Returns the squares that contain a piece that attacks the given square
Bitboard Position::AttacksTo(Square square, Bitboard occupancy) const 
{
    return (pawnAttackMask(WHITE, square)           & Pieces(PAWN, BLACK))
         | (pawnAttackMask(BLACK, square)           & Pieces(PAWN, WHITE))
         | (attackMask(KNIGHT, square)              & Pieces(KNIGHT))
         | (attackMask(ROOK, square, occupancy)     & Pieces(ROOK, QUEEN))
         | (attackMask(BISHOP, square, occupancy)   & Pieces(BISHOP, QUEEN))
         | (attackMask(KING, square)                & Pieces(KING));
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
    Bitboard squareMask = getSquareMask(square);
    Piece piece = pieceOnSquare[square];
    pieceOnSquare[square] = EMPTY;

    typeBoard[ALL_PIECES]       ^= squareMask;
    typeBoard[getType(piece)]   ^= squareMask;
    colorBoard[getColor(piece)] ^= squareMask;

    numPieces[piece]--;
    numPieces[getPiece(ALL_PIECES, getColor(piece))]--;
}

void Position::Print()
{
    std::map<Piece, std::string> pieceToChar = {
        {WHITE_PAWN,     "♟"}, {BLACK_PAWN,     "♙"},
        {WHITE_KNIGHT,   "♞"}, {BLACK_KNIGHT,   "♘"},
        {WHITE_BISHOP,   "♝"}, {BLACK_BISHOP,   "♗"},
        {WHITE_ROOK,     "♜"}, {BLACK_ROOK,     "♖"},
        {WHITE_QUEEN,    "♛"}, {BLACK_QUEEN,    "♕"},
        {WHITE_KING,     "♚"}, {BLACK_KING,     "♔"},
        {EMPTY,          " "}
    };

    std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;

    for (Rank rank = RANK_8; rank >= RANK_1; rank--)
    {
        std::cout << rank+1 << " ";
        for (File file = FILE_A; file < NUM_FILES; file++)
        {
            std::cout << "| " << pieceToChar[pieceOnSquare[createSquare(file, rank)]] << " ";
        }

        std::cout << "|\n  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    
    std::cout << "    a   b   c   d   e   f   g   h\n" << std::endl;
    
    std::cout << "        Side:         "   << (sideToMove ? "black" : "white") << std::endl;
    std::cout << "        enpassant:    "   << ((enpassantSquare != NO_SQUARE)  ? algebraicNotation(enpassantSquare) : "no") << std::endl;
    std::cout << "        Castling:     "   << ((castlingRights & WHITE_SHORT)  ? 'K' : '-')
                                            << ((castlingRights & WHITE_LONG)   ? 'Q' : '-')
                                            << ((castlingRights & BLACK_SHORT)  ? 'k' : '-')
                                            << ((castlingRights & BLACK_LONG)   ? 'q' : '-')
                                            << std::endl << std::endl;
}

} // namespace ChessEngine