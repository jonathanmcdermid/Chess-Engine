#include <string>
#include <sstream>
#include "interface.h"

namespace Chess{

	void interface::drawBoard() {//prints board in cmd
		char letter;
		std::cout << "\n  a   b   c   d   e   f   g   h";
		for (uint8_t i = 0; i < WIDTH; ++i) {
			std::cout << "\n---------------------------------\n|";
			for (uint8_t j = 0; j < WIDTH; ++j) {
				switch (game.getGrid(i * WIDTH + j)) {
				case PAWN:		{ letter = 'P'; break; }
				case ROOK:		{ letter = 'R'; break; }
				case KNIGHT:	{ letter = 'N'; break; }
				case BISHOP:	{ letter = 'B'; break; }
				case QUEEN:		{ letter = 'Q'; break; }
				case KING:		{ letter = 'K'; break; }
				case -PAWN:		{ letter = 'p'; break; }
				case -ROOK:		{ letter = 'r'; break; }
				case -KNIGHT:	{ letter = 'n'; break; }
				case -BISHOP:	{ letter = 'b'; break; }
				case -QUEEN:	{ letter = 'q'; break; }
				case -KING:		{ letter = 'k'; break; }
				default:		{ letter = ' '; }
				}
				std::cout << " " << letter << " |";
			}
			std::cout << " " << WIDTH - i;
		}
		std::cout << "\n---------------------------------\n";
	}

	void interface::go(std::istringstream& is) {//most options are not implemented yet
		std::string word;
		while (is >> word) {
			if (word == "searchmoves")		{ ; }
			else if (word == "wtime")		{ is >> ai.lim.time[WHITE]; }
			else if (word == "btime")		{ is >> ai.lim.time[BLACK]; }
			else if (word == "winc")		{ is >> ai.lim.inc[WHITE]; }
			else if (word == "binc")		{ is >> ai.lim.inc[BLACK]; }
			else if (word == "movestogo")	{ is >> ai.lim.movesleft; }
			else if (word == "depth")		{ is >> ai.lim.depth; }
			else if (word == "nodes")		{ is >> ai.lim.nodes; }
			else if (word == "movetime")	{ is >> ai.lim.movetime; }
			else if (word == "mate")		{ is >> ai.lim.mate; }
			else if (word == "perft")		{ is >> ai.lim.perft; }
			else if (word == "infinite")	{ ai.lim.infinite = true; }
			else if (word == "ponder")		{ ; }
		}
		interface::botMove();
	}

	void interface::position(std::istringstream& is) {
		std::string word, fen;
		is >> word;
		if (word == "startpos") {
			fen = STARTFEN;
			is >> word;
		}
		else if (word == "fen") { while (is >> word && word != "moves") { fen += word + " "; } }
		else { return; }
		game.fenSet(fen);
		while (is >> word) { if (!playerMove(word)) { break; } }
	}

	void interface::uci(int argc, char* argv[]) {//uci communication loop, some options non functioning
		std::string word, cmd;
		game = board();
		ai = bot();
		std::cout << "id name chessbrainlet 1.0\nid author Jonathan M\nuciok\n";
		for (int i = 1; i < argc; ++i) { cmd += std::string(argv[i]) + " "; }
		do {
			if (argc == 1 && !getline(std::cin, cmd)) { cmd = "quit"; }
			std::istringstream is(cmd);
			word.clear();
			is >> std::skipws >> word;
			if (word == "quit" || word == "stop")	{ ; }
			else if (word == "ponderhit")			{ ; }
			else if (word == "uci")					{ std::cout << "id name chessbrainlet 1.0\nid author Jonathan M\nuciok\n"; }
			else if (word == "setoption")			{ ; }
			else if (word == "go")					{ go(is); }
			else if (word == "position")			{ position(is); }
			else if (word == "ucinewgame")			{ game = board(); }
			else if (word == "isready")				{ std::cout << "readyok\n"; }
			else if (word == "print")				{ drawBoard(); }
		} while (word != "quit" && argc == 1);
	}

	interface::interface(int argc, char* argv[]){//awaits input from user or uci
		std::string input;
		while (1) {
			std::getline(std::cin, input);
			if (input == "uci")		{ uci(argc, argv); }
			if (input == "local")	{ local(); }
			if (input == "quit")	{ exit(1); }
		}
	}

	void interface::local() {//for play without uci
		game = board();
		ai = bot();
		std::string input;
		interface::drawBoard();
		while (1) {
			std::getline(std::cin, input);
			if (input.length()) {
				while (!playerMove(input)) { std::getline(std::cin, input); }
				interface::drawBoard();
				if (game.checkMate()) { break; }
				interface::botMove();
				interface::drawBoard();
				if (game.checkMate()) { break; }
			}
		}
		std::cout << "good game\n";
	}

	bool interface::playerMove(std::string input) {//makes external moves
		if (input.length() == 4) {
			uint8_t from = (WIDTH - (input.c_str()[1] - '0')) * WIDTH + input.c_str()[0] - 'a';
			uint8_t to	 = (WIDTH - (input.c_str()[3] - '0')) * WIDTH + input.c_str()[2] - 'a';
			if (from >= 0 && from < SPACES && to >= 0 && to < SPACES) {
				m = game.createMove(from, to);
				if (m.getFlags() != FAIL && m.getFlags() < PROMOTE) {
					if (game.movePiece(m)) { return true; }
				}
			}
		}
		else if (input.length() == 5) {
			uint8_t from = (WIDTH - (input.c_str()[1] - '0')) * WIDTH + input.c_str()[0] - 'a';
			uint8_t to = (WIDTH - (input.c_str()[3] - '0')) * WIDTH + input.c_str()[2] - 'a';
			char flags = input.c_str()[4];
			if (from >= 0 && from < SPACES && to >= 0 && to < SPACES) {
				m = game.createMove(from, to);
				if (m.getFlags() >= PROMOTE) {
					switch (flags) {
					case 'n': { m = (m.getFlags() == QPROMOTEC) ? move(m.getFrom(), m.getTo(), NPROMOTEC) : move(m.getFrom(), m.getTo(), NPROMOTE); break; }
					case 'b': { m = (m.getFlags() == QPROMOTEC) ? move(m.getFrom(), m.getTo(), BPROMOTEC) : move(m.getFrom(), m.getTo(), BPROMOTE); break; }
					case 'r': { m = (m.getFlags() == QPROMOTEC) ? move(m.getFrom(), m.getTo(), RPROMOTEC) : move(m.getFrom(), m.getTo(), RPROMOTE); break; }
					case 'q': { m = (m.getFlags() == QPROMOTEC) ? move(m.getFrom(), m.getTo(), QPROMOTEC) : move(m.getFrom(), m.getTo(), QPROMOTE); break; }
					}
					if (game.movePiece(m)) { return true; }
				}
			}
		}
		else if (input == "fenget") { std::cout << game.fenGet() << "\n"; }
		else if (input == "fenset") {
			std::getline(std::cin, input);
			game.fenSet(input);
		}
		return false;
	}

	void interface::botMove() {//generates internal moves
		m = ai.getMove(&game);
		game.movePiece(m);
		char message[] = { (int) WIDTH - m.getFrom() / WIDTH + '0', (int) m.getFrom() % WIDTH + 'a', (int) WIDTH - m.getTo() / WIDTH + '0', (int) m.getTo() % WIDTH + 'a' };
		std::cout << "bestmove " << message[1] << message[0] << message[3] << message[2];
		if (game.getmHist(0).getFlags() >= PROMOTE) { std::cout << "q"; }
		std::cout << "\n";
	}
}