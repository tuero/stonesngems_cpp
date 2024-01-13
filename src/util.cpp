#include <cassert>
#include <exception>
#include <sstream>
#include <string>
#include <vector>

#include "definitions.h"

namespace stonesngems {

auto parse_board_str(const std::string &board_str) -> Board {
    std::stringstream board_ss(board_str);
    std::string segment;
    std::vector<std::string> seglist;
    // string split on |
    while (std::getline(board_ss, segment, '|')) {
        seglist.push_back(segment);
    }

    assert(seglist.size() >= 2);

    // Get general info
    const auto rows = static_cast<std::size_t>(std::stoi(seglist[0]));
    const auto cols = static_cast<std::size_t>(std::stoi(seglist[1]));
    assert((int)seglist.size() == rows * cols + 4);
    const int max_steps = std::stoi(seglist[2]);
    const int max_gems = std::stoi(seglist[3]);
    Board board(rows, cols, static_cast<uint8_t>(max_gems), max_steps);

    // Parse grid
    int agent_counter = 0;
    for (std::size_t i = 4; i < seglist.size(); ++i) {
        const auto el = static_cast<HiddenCellType>(std::stoi(seglist[i]));
        board.item(i - 4) = to_underlying(el);
        // Really shouldn't be creating a state with the agent in the exit
        if (el == HiddenCellType::kAgent || el == HiddenCellType::kAgentInExit) {
            board.agent_pos = i - 4;
            board.agent_idx = i - 4;
            ++agent_counter;
        }
    }

    if (agent_counter == 0) {
        throw std::invalid_argument("Agent element not found");
    } else if (agent_counter > 1) {
        throw std::invalid_argument("Too many agent elements, expected only one");
    }

    return board;
}

constexpr int SIZE_REQUIRING_ZERO = 10;
auto board_to_str(const Board &board) -> std::string {
    std::stringstream board_ss;
    board_ss << board.rows << "|" << board.cols << "|" << board.max_steps << "|"
             << static_cast<int>(board.gems_required);
    for (const auto &el : board.grid) {
        board_ss << "|";
        if (el < SIZE_REQUIRING_ZERO) {
            board_ss << "0";
        }
        board_ss << static_cast<int>(el);
    }
    return board_ss.str();
}

}    // namespace stonesngems
