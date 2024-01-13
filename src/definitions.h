#ifndef STONESNGEMS_DEFS_H_
#define STONESNGEMS_DEFS_H_

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <vector>

namespace stonesngems {

template <class E>
constexpr inline auto to_underlying(E e) noexcept -> std::underlying_type_t<E> {
    return static_cast<std::underlying_type_t<E>>(e);
}

enum class HiddenCellType : int8_t {
    kNull = -1,
    kAgent = 0,
    kEmpty = 1,
    kDirt = 2,
    kStone = 3,
    kStoneFalling = 4,
    kDiamond = 5,
    kDiamondFalling = 6,
    kExitClosed = 7,
    kExitOpen = 8,
    kAgentInExit = 9,
    kFireflyUp = 10,
    kFireflyLeft = 11,
    kFireflyDown = 12,
    kFireflyRight = 13,
    kButterflyUp = 14,
    kButterflyLeft = 15,
    kButterflyDown = 16,
    kButterflyRight = 17,
    kWallBrick = 18,
    kWallSteel = 19,
    kWallMagicDormant = 20,
    kWallMagicOn = 21,
    kWallMagicExpired = 22,
    kBlob = 23,
    kExplosionDiamond = 24,
    kExplosionBoulder = 25,
    kExplosionEmpty = 26,
    kGateRedClosed = 27,
    kGateRedOpen = 28,
    kKeyRed = 29,
    kGateBlueClosed = 30,
    kGateBlueOpen = 31,
    kKeyBlue = 32,
    kGateGreenClosed = 33,
    kGateGreenOpen = 34,
    kKeyGreen = 35,
    kGateYellowClosed = 36,
    kGateYellowOpen = 37,
    kKeyYellow = 38,
    kNut = 39,
    kNutFalling = 40,
    kBomb = 41,
    kBombFalling = 42,
    kOrangeUp = 43,
    kOrangeLeft = 44,
    kOrangeDown = 45,
    kOrangeRight = 46,
    kPebbleInDirt = 47,
    kStoneInDirt = 48,
    kVoidInDirt = 49,
};
constexpr int kNumHiddenCellType = 50;

// Cell types which are observable
enum class VisibleCellType : int8_t {
    kNull = -1,
    kAgent = 0,
    kEmpty = 1,
    kDirt = 2,
    kStone = 3,
    kDiamond = 4,
    kExitClosed = 5,
    kExitOpen = 6,
    kAgentInExit = 7,
    kFirefly = 8,
    kButterfly = 9,
    kWallBrick = 10,
    kWallSteel = 11,
    kWallMagicOff = 12,
    kWallMagicOn = 13,
    kBlob = 14,
    kExplosion = 15,
    kGateRedClosed = 16,
    kGateRedOpen = 17,
    kKeyRed = 18,
    kGateBlueClosed = 19,
    kGateBlueOpen = 20,
    kKeyBlue = 21,
    kGateGreenClosed = 22,
    kGateGreenOpen = 23,
    kKeyGreen = 24,
    kGateYellowClosed = 25,
    kGateYellowOpen = 26,
    kKeyYellow = 27,
    kNut = 28,
    kBomb = 29,
    kOrange = 30,
    kPebbleInDirt = 31,
    kStoneInDirt = 32,
    kVoidInDirt = 33,
};
constexpr int kNumVisibleCellType = 34;

// Actions the agent can take
enum class Action : std::size_t {
    kNoop = 0,
    kUp = 1,
    kRight = 2,
    kDown = 3,
    kLeft = 4,
};
constexpr int kNumActions = 5;
const std::array<Action, kNumActions> ALL_ACTIONS{
    Action::kNoop, Action::kUp, Action::kRight, Action::kDown, Action::kLeft,
};

// Directions the interactions take place
enum class Direction : decltype(to_underlying(Action::kNoop)) {
    kNoop = to_underlying(Action::kNoop),
    kUp = to_underlying(Action::kUp),
    kRight = to_underlying(Action::kRight),
    kDown = to_underlying(Action::kDown),
    kLeft = to_underlying(Action::kLeft),
    kUpRight = to_underlying(Action::kLeft) + 1,
    kDownRight = to_underlying(Action::kLeft) + 2,
    kDownLeft = to_underlying(Action::kLeft) + 3,
    kUpLeft = to_underlying(Action::kLeft) + 4,
};
// Agent can only take a subset of all directions
constexpr int kNumDirections = 9;

[[nodiscard]] constexpr inline auto action_to_direction(Action action) noexcept -> Direction {
    return static_cast<Direction>(to_underlying(action));
}

enum RewardCodes {
    kRewardAgentDies = 1 << 0,
    kRewardCollectDiamond = 1 << 1,
    kRewardWalkThroughExit = 1 << 2,
    kRewardNutToDiamond = 1 << 3,
    kRewardCollectKey = 1 << 4,
    kRewardCollectKeyRed = 1 << 5,
    kRewardCollectKeyBlue = 1 << 6,
    kRewardCollectKeyGreen = 1 << 7,
    kRewardCollectKeyYellow = 1 << 8,
    kRewardWalkThroughGate = 1 << 9,
    kRewardWalkThroughGateRed = 1 << 10,
    kRewardWalkThroughGateBlue = 1 << 11,
    kRewardWalkThroughGateGreen = 1 << 12,
    kRewardWalkThroughGateYellow = 1 << 13,
};

constexpr std::size_t kAgentPosExit = std::numeric_limits<std::size_t>::max();
constexpr std::size_t kAgentPosDie = kAgentPosExit - 1;

// Element entities, along with properties
struct Element {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    HiddenCellType cell_type;
    VisibleCellType visible_type;
    int properties;
    char id;
    bool has_updated;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    Element()
        : cell_type(HiddenCellType::kNull),
          visible_type(VisibleCellType::kNull),
          properties(0),
          id(0),
          has_updated(false) {}

    Element(HiddenCellType cell_type, VisibleCellType visible_type, int properties, char id)
        : cell_type(cell_type), visible_type(visible_type), properties(properties), id(id), has_updated(false) {}

    auto operator==(const Element &rhs) const -> bool {
        return this->cell_type == rhs.cell_type;
    }

    auto operator!=(const Element &rhs) const -> bool {
        return this->cell_type != rhs.cell_type;
    }
};

struct Board {
    Board() = delete;
    Board(std::size_t rows, std::size_t cols, uint8_t gems_required, int max_steps)
        : rows(rows),
          cols(cols),
          gems_required(gems_required),
          max_steps(max_steps),
          grid(rows * cols, 0),
          has_updated(rows * cols, false) {}

    auto operator==(const Board &other) const -> bool {
        return grid == other.grid;
    }

    [[nodiscard]] auto item(std::size_t index) noexcept -> int8_t & {
        return grid[index];
    }

    [[nodiscard]] auto item(std::size_t index) const noexcept -> int8_t {
        return grid[index];
    }

    [[nodiscard]] auto find_all(int8_t element) const noexcept -> std::vector<std::size_t> {
        std::vector<std::size_t> indices;
        for (std::size_t i = 0; i < rows * cols; ++i) {
            if (grid[i] == element) {
                indices.push_back(i);
            }
        }
        return indices;
    }

    void reset_updated() noexcept {
        for (std::size_t i = 0; i < rows * cols; ++i) {
            has_updated[i] = false;
        }
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    uint64_t zorb_hash = 0;
    std::size_t rows;
    std::size_t cols;
    uint8_t gems_required;
    std::size_t agent_pos = kAgentPosDie;
    std::size_t agent_idx = kAgentPosDie;
    int max_steps;
    std::vector<int8_t> grid;
    std::vector<bool> has_updated;
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

}    // namespace stonesngems

#endif    // STONESNGEMS_DEFS_H_
