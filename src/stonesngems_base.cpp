#include "stonesngems_base.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <random>

#include "definitions.h"
#include "util.h"

namespace stonesngems {

RNDGameState::RNDGameState(const GameParameters &params) : shared_state_ptr(std::make_shared<SharedStateInfo>(params)) {
    reset();
}

auto RNDGameState::operator==(const RNDGameState &other) const noexcept -> bool {
    return local_state == other.local_state && board == other.board;
}

auto RNDGameState::operator!=(const RNDGameState &other) const noexcept -> bool {
    return !(*this == other);
}

const std::vector<Action> RNDGameState::ALL_ACTIONS{
    Action::kNoop, Action::kUp, Action::kRight, Action::kDown, Action::kLeft,
};

// ---------------------------------------------------------------------------

namespace {
[[noreturn]] inline void unreachable() {
    // Uses compiler specific extensions if possible.
    // Even if no extension is used, undefined behavior is still raised by
    // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__)    // MSVC
    __assume(false);
#else    // GCC, Clang
    __builtin_unreachable();
#endif
}
}    // namespace

// https://en.wikipedia.org/wiki/Xorshift
// Portable RNG Seed
// NOLINTBEGIN
auto splitmix64(uint64_t seed) noexcept -> uint64_t {
    uint64_t result = seed + 0x9E3779B97f4A7C15;
    result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
    result = (result ^ (result >> 27)) * 0x94D049BB133111EB;
    return result ^ (result >> 31);
}
// NOLINTEND

// Portable RNG
// NOLINTBEGIN
auto xorshift64(uint64_t &s) noexcept -> uint64_t {
    uint64_t x = s;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    s = x;
    return x;
}
// NOLINTEND

void RNDGameState::reset() {
    // Board, local, and shared state info
    board = parse_board_str(shared_state_ptr->game_board_str);
    local_state = LocalState();
    local_state.random_state = splitmix64(static_cast<uint64_t>(shared_state_ptr->rng_seed));
    local_state.steps_remaining = board.max_steps;
    shared_state_ptr->blob_max_size =
        static_cast<int>(static_cast<float>(board.cols * board.rows) * shared_state_ptr->blob_max_percentage);

    // Set the item IDs
    for (std::size_t i = 0; i < board.cols * board.rows; ++i) {
        AddIndexID(i);
    }

    // zorbist hashing
    std::mt19937 gen(static_cast<unsigned long>(shared_state_ptr->rng_seed));
    std::uniform_int_distribution<uint64_t> dist(0);
    for (std::size_t channel = 0; channel < kNumHiddenCellType; ++channel) {
        for (std::size_t i = 0; i < board.cols * board.rows; ++i) {
            shared_state_ptr->zrbht[(channel * board.cols * board.rows) + i] = dist(gen);
        }
    }

    // Set initial hash
    for (std::size_t i = 0; i < board.cols * board.rows; ++i) {
        board.zorb_hash ^=
            shared_state_ptr->zrbht.at((static_cast<std::size_t>(board.item(i)) * board.cols * board.rows) + i);
    }

    // In bounds fast access
    shared_state_ptr->in_bounds_board.clear();
    shared_state_ptr->in_bounds_board.insert(shared_state_ptr->in_bounds_board.end(),
                                             (board.cols + 2) * (board.rows + 2), true);
    // Pad the outer boarder
    for (std::size_t i = 0; i < board.cols + 2; ++i) {
        shared_state_ptr->in_bounds_board[i] = false;
        shared_state_ptr->in_bounds_board[(board.rows + 1) * (board.cols + 2) + i] = false;
    }
    for (std::size_t i = 0; i < board.rows + 2; ++i) {
        shared_state_ptr->in_bounds_board[i * (board.cols + 2)] = false;
        shared_state_ptr->in_bounds_board[i * (board.cols + 2) + board.cols + 1] = false;
    }
    // In bounds idx conversion table
    shared_state_ptr->board_to_inbounds.clear();
    for (std::size_t r = 0; r < board.rows; ++r) {
        for (std::size_t c = 0; c < board.cols; ++c) {
            shared_state_ptr->board_to_inbounds.push_back((board.cols + 2) * (r + 1) + c + 1);
        }
    }
}

void RNDGameState::apply_action(Action action) {
    assert(is_valid_action(action));
    StartScan();

    // Handle agent first
    const Direction action_direction = action_to_direction(action);
    UpdateAgent(board.agent_idx, action_direction);

    // Handle all other items
    for (std::size_t i = 0; i < board.rows * board.cols; ++i) {
        if (board.has_updated[i]) {    // Item already updated
            continue;
        }
        switch (board.item(i)) {
            // Handle non-compound types
            case HiddenCellType::kStone:
                UpdateStone(i);
                break;
            case HiddenCellType::kStoneFalling:
                UpdateStoneFalling(i);
                break;
            case HiddenCellType::kDiamond:
                UpdateDiamond(i);
                break;
            case HiddenCellType::kDiamondFalling:
                UpdateDiamondFalling(i);
                break;
            case HiddenCellType::kNut:
                UpdateNut(i);
                break;
            case HiddenCellType::kNutFalling:
                UpdateNutFalling(i);
                break;
            case HiddenCellType::kBomb:
                UpdateBomb(i);
                break;
            case HiddenCellType::kBombFalling:
                UpdateBombFalling(i);
                break;
            case HiddenCellType::kExitClosed:
                UpdateExit(i);
                break;
            case HiddenCellType::kBlob:
                UpdateBlob(i);
                break;
            default:
                // Handle compound types
                // NOLINTNEXTLINE(*-bounds-constant-array-index)
                const Element &element = kCellTypeToElement[static_cast<std::size_t>(board.item(i)) + 1];
                if (IsButterfly(element)) {
                    UpdateButterfly(i, kButterflyToDirection.at(element));
                } else if (IsFirefly(element)) {
                    UpdateFirefly(i, kFireflyToDirection.at(element));
                } else if (IsOrange(element)) {
                    UpdateOrange(i, kOrangeToDirection.at(element));
                } else if (IsMagicWall(element)) {
                    UpdateMagicWall(i);
                } else if (IsExplosion(element)) {
                    UpdateExplosions(i);
                }
                break;
        }
    }

    EndScan();
}

auto RNDGameState::is_terminal() const noexcept -> bool {
    // timeout or agent is either dead/in exit
    const bool out_of_time = (board.max_steps > 0 && local_state.steps_remaining <= 0);
    return out_of_time || board.agent_pos == kAgentPosExit || board.agent_pos == kAgentPosDie;
}

auto RNDGameState::is_solution() const noexcept -> bool {
    // not timeout and agent is in exit
    const bool out_of_time = (board.max_steps > 0 && local_state.steps_remaining <= 0);
    return !out_of_time && board.agent_pos == kAgentPosExit;
}

auto RNDGameState::legal_actions() const noexcept -> std::vector<Action> {
    return {Action::kNoop, Action::kUp, Action::kRight, Action::kDown, Action::kLeft};
}

void RNDGameState::legal_actions(std::vector<Action> &actions) const noexcept {
    actions.clear();
    for (const auto &a : ALL_ACTIONS) {
        actions.push_back(a);
    }
}

auto RNDGameState::observation_shape() const noexcept -> std::array<std::size_t, 3> {
    return {kNumVisibleCellType, board.rows, board.cols};
}

auto RNDGameState::get_observation() const noexcept -> std::vector<float> {
    const std::size_t channel_length = board.cols * board.rows;
    std::vector<float> obs(kNumVisibleCellType * channel_length, 0);
    for (std::size_t i = 0; i < channel_length; ++i) {
        obs[static_cast<std::size_t>(GetItem(i).visible_type) * channel_length + i] = 1;
    }
    return obs;
}

void RNDGameState::get_observation(std::vector<float> &obs) const noexcept {
    const std::size_t channel_length = board.cols * board.rows;
    const std::size_t obs_size = kNumVisibleCellType * channel_length;
    obs.clear();
    obs.reserve(obs_size);
    std::fill_n(std::back_inserter(obs), obs_size, static_cast<float>(0));
    for (std::size_t i = 0; i < channel_length; ++i) {
        obs[static_cast<std::size_t>(GetItem(i).visible_type) * channel_length + i] = 1;
    }
}

auto RNDGameState::get_observation(const std::vector<VisibleCellType> &filter_elements) const noexcept
    -> std::vector<float> {
    const std::size_t channel_length = board.cols * board.rows;
    std::vector<float> obs(filter_elements.size() * channel_length, 0);
    for (std::size_t i = 0; i < channel_length; ++i) {
        // Slow but this allows us to control the order of the element channels than arbitrary set
        const auto channel = static_cast<std::size_t>(
            std::distance(filter_elements.begin(),
                          std::find(filter_elements.begin(), filter_elements.end(), GetItem(i).visible_type)));
        obs[channel * channel_length + i] =
            channel < filter_elements.size() ? static_cast<float>(1) : static_cast<float>(0);
    }
    return obs;
}

// VisibleCellType to image binary data
#include "assets_all.inc"

auto RNDGameState::image_shape() const noexcept -> std::array<std::size_t, 3> {
    const auto rows = board.rows;
    const auto cols = board.cols;
    return {rows * SPRITE_HEIGHT, cols * SPRITE_WIDTH, SPRITE_CHANNELS};
}

auto RNDGameState::to_image() const noexcept -> std::vector<uint8_t> {
    const std::size_t flat_size = board.cols * board.rows;
    std::vector<uint8_t> img(flat_size * SPRITE_DATA_LEN, 0);
    for (std::size_t h = 0; h < board.rows; ++h) {
        for (std::size_t w = 0; w < board.cols; ++w) {
            const std::size_t img_idx_top_left = h * (SPRITE_DATA_LEN * board.cols) + (w * SPRITE_DATA_LEN_PER_ROW);
            const std::vector<uint8_t> &data = img_asset_map.at(GetItem(h * board.cols + w).visible_type);
            for (std::size_t r = 0; r < SPRITE_HEIGHT; ++r) {
                for (std::size_t c = 0; c < SPRITE_WIDTH; ++c) {
                    const std::size_t data_idx = (r * SPRITE_DATA_LEN_PER_ROW) + (3 * c);
                    const std::size_t img_idx =
                        (r * SPRITE_DATA_LEN_PER_ROW * board.cols) + (SPRITE_CHANNELS * c) + img_idx_top_left;
                    img[img_idx + 0] = data[data_idx + 0];
                    img[img_idx + 1] = data[data_idx + 1];
                    img[img_idx + 2] = data[data_idx + 2];
                }
            }
        }
    }
    return img;
}

auto RNDGameState::board_to_str() const noexcept -> std::string {
    return ::stonesngems::board_to_str(board);
}

auto RNDGameState::get_reward_signal() const noexcept -> uint64_t {
    return local_state.reward_signal;
}

auto RNDGameState::get_hash() const noexcept -> uint64_t {
    return board.zorb_hash;
}

auto RNDGameState::get_positions(HiddenCellType element) const noexcept -> std::vector<Position> {
    assert(is_valid_hidden_element(element));
    std::vector<Position> indices;
    for (const auto &idx : board.find_all(element)) {
        indices.emplace_back(idx / board.cols, idx % board.cols);
    }
    return indices;
}

auto RNDGameState::position_to_index(const Position &position) const noexcept -> std::size_t {
    return position.first * board.cols + position.second;
}

auto RNDGameState::index_to_position(std::size_t index) const noexcept -> Position {
    return {index / board.cols, index % board.cols};
}

auto RNDGameState::get_indices(HiddenCellType element) const noexcept -> std::vector<std::size_t> {
    assert(is_valid_hidden_element(element));
    std::vector<std::size_t> indices;
    for (const auto &idx : board.find_all(element)) {
        indices.push_back(idx);
    }
    return indices;
}

auto RNDGameState::is_pos_in_bounds(const Position &position) const noexcept -> bool {
    return position.first < board.rows && position.second < board.cols;
}

auto RNDGameState::get_index_id(std::size_t index) const noexcept -> int {
    assert(index < board.rows * board.cols);
    for (const auto &p : local_state.index_id_mappings) {
        if (p.index == index) {
            return p.id;
        }
    }
    return -1;
}

auto RNDGameState::get_id_index(int id) const noexcept -> std::size_t {
    for (const auto &p : local_state.index_id_mappings) {
        if (p.id == id) {
            return p.index;
        }
    }
    return std::numeric_limits<std::size_t>::max();
}

auto RNDGameState::get_valid_rewards() const noexcept -> std::unordered_set<RewardCodes> {
    std::unordered_set<RewardCodes> reward_codes;
    for (const auto &el : board.grid) {
        if (kElementToRewardMap.find(el) != kElementToRewardMap.end()) {
            reward_codes.insert(kElementToRewardMap.at(el));
        }
    }
    return reward_codes;
}

auto RNDGameState::get_agent_pos() const noexcept -> std::size_t {
    return board.agent_pos;
}

auto RNDGameState::get_agent_index() const noexcept -> std::size_t {
    return board.agent_idx;
}

auto RNDGameState::get_hidden_item(std::size_t index) const noexcept -> HiddenCellType {
    assert(index < board.rows * board.cols);
    return board.item(index);
}

auto operator<<(std::ostream &os, const RNDGameState &state) -> std::ostream & {
    const auto print_horz_boarder = [&]() {
        for (std::size_t w = 0; w < state.board.cols + 2; ++w) {
            os << "-";
        }
        os << std::endl;
    };
    print_horz_boarder();
    for (std::size_t h = 0; h < state.board.rows; ++h) {
        os << "|";
        for (std::size_t w = 0; w < state.board.cols; ++w) {
            // NOLINTNEXTLINE(*-bounds-constant-array-index)
            os << kCellTypeToElement[static_cast<std::size_t>(state.board.grid[h * state.board.cols + w]) + 1].id;
        }
        os << "|" << std::endl;
    }
    print_horz_boarder();
    return os;
}

// ---------------------------------------------------------------------------

// Not safe, assumes InBounds has been called (or used in conjunction)
auto RNDGameState::IndexFromDirection(std::size_t index, Direction direction) const noexcept -> std::size_t {
    switch (direction) {
        case Direction::kNoop:
            return index;
        case Direction::kUp:
            return index - board.cols;
        case Direction::kRight:
            return index + 1;
        case Direction::kDown:
            return index + board.cols;
        case Direction::kLeft:
            return index - 1;
        case Direction::kUpRight:
            return index - board.cols + 1;
        case Direction::kDownRight:
            return index + board.cols + 1;
        case Direction::kUpLeft:
            return index - board.cols - 1;
        case Direction::kDownLeft:
            return index + board.cols - 1;
        default:
            unreachable();
    }
}
auto RNDGameState::BoundsIndexFromDirection(std::size_t index, Direction direction) const noexcept -> std::size_t {
    switch (direction) {
        case Direction::kNoop:
            return index;
        case Direction::kUp:
            return index - (board.cols + 2);
        case Direction::kRight:
            return index + 1;
        case Direction::kDown:
            return index + (board.cols + 2);
        case Direction::kLeft:
            return index - 1;
        case Direction::kUpRight:
            return index - (board.cols + 2) + 1;
        case Direction::kDownRight:
            return index + (board.cols + 2) + 1;
        case Direction::kUpLeft:
            return index - (board.cols + 2) - 1;
        case Direction::kDownLeft:
            return index + (board.cols + 2) - 1;
        default:
            unreachable();
    }
}

auto RNDGameState::InBounds(std::size_t index, Direction direction) const noexcept -> bool {
    return shared_state_ptr
        ->in_bounds_board[BoundsIndexFromDirection(shared_state_ptr->board_to_inbounds[index], direction)];
}

auto RNDGameState::IsType(std::size_t index, const Element &element, Direction direction) const noexcept -> bool {
    const std::size_t new_index = IndexFromDirection(index, direction);
    return InBounds(index, direction) && GetItem(new_index) == element;
}

auto RNDGameState::HasProperty(std::size_t index, int property, Direction direction) const noexcept -> bool {
    const std::size_t new_index = IndexFromDirection(index, direction);
    return InBounds(index, direction) && ((GetItem(new_index).properties & property) > 0);
}

void RNDGameState::UpdateIDIndex(std::size_t index_old, std::size_t index_new) noexcept {
    for (auto &p : local_state.index_id_mappings) {
        if (p.index == index_old) {
            p.index = index_new;
            return;
        }
    }
}

void RNDGameState::UpdateIndexID(std::size_t index) noexcept {
    for (auto &p : local_state.index_id_mappings) {
        if (p.index == index) {
            p.id = ++local_state.id_state;
            return;
        }
    }
}

void RNDGameState::AddIndexID(std::size_t index) noexcept {
    switch (board.item(index)) {
        case HiddenCellType::kStone:
        case HiddenCellType::kStoneFalling:
        case HiddenCellType::kDiamond:
        case HiddenCellType::kDiamondFalling:
        case HiddenCellType::kNut:
        case HiddenCellType::kNutFalling: {
            local_state.index_id_mappings.emplace_back(index, ++local_state.id_state);
            break;
        }
        default:
            break;
    }
}

void RNDGameState::RemoveIndexID(std::size_t index) noexcept {
    std::erase_if(local_state.index_id_mappings, [&](const auto &p) -> bool { return p.index == index; });
}

void RNDGameState::MoveItem(std::size_t index, Direction direction) noexcept {
    const std::size_t new_index = IndexFromDirection(index, direction);
    board.zorb_hash ^= shared_state_ptr->zrbht.at(
        (static_cast<std::size_t>(board.item(new_index)) * board.cols * board.rows) + new_index);
    board.item(new_index) = board.item(index);
    board.zorb_hash ^= shared_state_ptr->zrbht.at(
        (static_cast<std::size_t>(board.item(new_index)) * board.cols * board.rows) + new_index);
    // grid_.ids[new_index] = grid_.ids[index];

    board.zorb_hash ^=
        shared_state_ptr->zrbht.at((static_cast<std::size_t>(board.item(index)) * board.cols * board.rows) + index);
    board.item(index) = kElEmpty.cell_type;
    board.zorb_hash ^= shared_state_ptr->zrbht.at(
        (static_cast<std::size_t>(ElementToItem(kElEmpty)) * board.cols * board.rows) + index);
    board.has_updated[new_index] = true;
    // grid_.ids[index] = ++id_counter_;

    // Update ID
    UpdateIDIndex(index, new_index);
}

void RNDGameState::SetItem(std::size_t index, const Element &element, int id, Direction direction) noexcept {
    (void)id;
    const std::size_t new_index = IndexFromDirection(index, direction);
    board.zorb_hash ^= shared_state_ptr->zrbht.at(
        (static_cast<std::size_t>(board.item(new_index)) * board.cols * board.rows) + new_index);
    board.item(new_index) = element.cell_type;
    board.zorb_hash ^= shared_state_ptr->zrbht.at(
        (static_cast<std::size_t>(ElementToItem(element)) * board.cols * board.rows) + new_index);
    // grid_.ids[new_index] = id;
    board.has_updated[new_index] = true;
}

auto RNDGameState::GetItem(std::size_t index, Direction direction) const noexcept -> const Element & {
    const std::size_t new_index = IndexFromDirection(index, direction);
    // NOLINTNEXTLINE(*-bounds-constant-array-index)
    return kCellTypeToElement[static_cast<std::size_t>(board.item(new_index)) + 1];
}

auto RNDGameState::IsTypeAdjacent(std::size_t index, const Element &element) const noexcept -> bool {
    return IsType(index, element, Direction::kUp) || IsType(index, element, Direction::kLeft) ||
           IsType(index, element, Direction::kDown) || IsType(index, element, Direction::kRight);
}

// ---------------------------------------------------------------------------

auto RNDGameState::CanRollLeft(std::size_t index) const noexcept -> bool {
    return HasProperty(index, ElementProperties::kRounded, Direction::kDown) &&
           IsType(index, kElEmpty, Direction::kLeft) && IsType(index, kElEmpty, Direction::kDownLeft);
}

auto RNDGameState::CanRollRight(std::size_t index) const noexcept -> bool {
    return HasProperty(index, ElementProperties::kRounded, Direction::kDown) &&
           IsType(index, kElEmpty, Direction::kRight) && IsType(index, kElEmpty, Direction::kDownRight);
}

void RNDGameState::RollLeft(std::size_t index, const Element &element) noexcept {
    SetItem(index, element, -1);
    MoveItem(index, Direction::kLeft);
}

void RNDGameState::RollRight(std::size_t index, const Element &element) noexcept {
    SetItem(index, element, -1);
    MoveItem(index, Direction::kRight);
}

void RNDGameState::Push(std::size_t index, const Element &stationary, const Element &falling,
                        Direction direction) noexcept {
    const std::size_t new_index = IndexFromDirection(index, direction);
    // Check if same direction past element is empty so that theres room to push
    if (IsType(new_index, kElEmpty, direction)) {
        // Check if the element will become stationary or falling
        const std::size_t next_index = IndexFromDirection(new_index, direction);
        const bool is_empty = IsType(next_index, kElEmpty, Direction::kDown);
        // Move item and set as falling or stationary
        MoveItem(new_index, direction);
        SetItem(next_index, is_empty ? falling : stationary, -1);
        // Move the agent
        MoveItem(index, direction);
        board.agent_pos = IndexFromDirection(index, direction);    // Assume only agent is pushing?
        board.agent_idx = IndexFromDirection(index, direction);    // Assume only agent is pushing?
    }
}

void RNDGameState::MoveThroughMagic(std::size_t index, const Element &element) noexcept {
    // Check if magic wall is still active
    if (local_state.magic_wall_steps <= 0) {
        return;
    }
    local_state.magic_active = true;
    const std::size_t index_wall = IndexFromDirection(index, Direction::kDown);
    const std::size_t index_under_wall = IndexFromDirection(index_wall, Direction::kDown);
    // Need to ensure cell below magic wall is empty (so item can pass through)
    if (IsType(index_under_wall, kElEmpty)) {
        SetItem(index, kElEmpty, -1);
        SetItem(index_under_wall, element, -1);
        UpdateIDIndex(index, index_under_wall);
    }
}

// NOLINTNEXTLINE (misc-no-recursion)
void RNDGameState::Explode(std::size_t index, const Element &element, Direction direction) noexcept {
    const std::size_t new_index = IndexFromDirection(index, direction);
    const auto it = kElementToExplosion.find(GetItem(new_index));
    const Element &ex = (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second;
    if (GetItem(new_index) == kElAgent) {
        board.agent_pos = kAgentPosDie;
    }
    SetItem(new_index, element, -1);
    RemoveIndexID(new_index);
    // Recursively check all directions for chain explosions
    for (std::size_t dir_index = 0; dir_index < kNumDirections; ++dir_index) {
        const auto dir = static_cast<Direction>(dir_index);
        if (dir == Direction::kNoop || !InBounds(new_index, dir)) {
            continue;
        }
        if (HasProperty(new_index, ElementProperties::kCanExplode, dir)) {
            Explode(new_index, ex, dir);
        } else if (HasProperty(new_index, ElementProperties::kConsumable, dir)) {
            SetItem(new_index, ex, -1, dir);
            if (GetItem(new_index, dir) == kElAgent) {
                board.agent_pos = kAgentPosDie;
            }
        }
    }
}

// ---------------------------------------------------------------------------

void RNDGameState::UpdateStone(std::size_t index) noexcept {
    // If no gravity, do nothing
    if (!shared_state_ptr->gravity) {
        return;
    }

    // Boulder falls if empty below
    if (IsType(index, kElEmpty, Direction::kDown)) {
        SetItem(index, kElStoneFalling, -1);
        UpdateStoneFalling(index);
    } else if (CanRollLeft(index)) {    // Roll left/right if possible
        RollLeft(index, kElStoneFalling);
    } else if (CanRollRight(index)) {
        RollRight(index, kElStoneFalling);
    }
}

void RNDGameState::UpdateStoneFalling(std::size_t index) noexcept {
    // Continue to fall as normal
    if (IsType(index, kElEmpty, Direction::kDown)) {
        MoveItem(index, Direction::kDown);
    } else if (HasProperty(index, ElementProperties::kCanExplode, Direction::kDown)) {
        // Falling stones can cause elements to explode
        const auto it = kElementToExplosion.find(GetItem(index, Direction::kDown));
        Explode(index, (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second, Direction::kDown);
    } else if (IsType(index, kElWallMagicOn, Direction::kDown) ||
               IsType(index, kElWallMagicDormant, Direction::kDown)) {
        MoveThroughMagic(index, kMagicWallConversion.at(GetItem(index)));
    } else if (IsType(index, kElNut, Direction::kDown)) {
        // Falling on a nut, crack it open to reveal a diamond!
        SetItem(index, kElDiamond, -1, Direction::kDown);
        UpdateIndexID(IndexFromDirection(index, Direction::kDown));
    } else if (IsType(index, kElNut, Direction::kDown)) {
        // Falling on a bomb, explode!
        const auto it = kElementToExplosion.find(GetItem(index));
        Explode(index, (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second);
    } else if (CanRollLeft(index)) {    // Roll left/right
        RollLeft(index, kElStoneFalling);
    } else if (CanRollRight(index)) {
        RollRight(index, kElStoneFalling);
    } else {
        // Default options is for falling stones to become stationary
        SetItem(index, kElStone, -1);
    }
}

void RNDGameState::UpdateDiamond(std::size_t index) noexcept {
    // If no gravity, do nothing
    if (!shared_state_ptr->gravity) {
        return;
    }

    // Diamond falls if empty below
    if (IsType(index, kElEmpty, Direction::kDown)) {
        SetItem(index, kElDiamondFalling, -1);
        UpdateDiamondFalling(index);
    } else if (CanRollLeft(index)) {    // Roll left/right if possible
        RollLeft(index, kElDiamondFalling);
    } else if (CanRollRight(index)) {
        RollRight(index, kElDiamondFalling);
    }
}

void RNDGameState::UpdateDiamondFalling(std::size_t index) noexcept {
    // Continue to fall as normal
    if (IsType(index, kElEmpty, Direction::kDown)) {
        MoveItem(index, Direction::kDown);
    } else if (HasProperty(index, ElementProperties::kCanExplode, Direction::kDown) &&
               !IsType(index, kElBomb, Direction::kDown) && !IsType(index, kElBombFalling, Direction::kDown)) {
        // Falling diamonds can cause elements to explode (but not bombs)
        const auto it = kElementToExplosion.find(GetItem(index, Direction::kDown));
        Explode(index, (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second, Direction::kDown);
    } else if (IsType(index, kElWallMagicOn, Direction::kDown) ||
               IsType(index, kElWallMagicDormant, Direction::kDown)) {
        MoveThroughMagic(index, kMagicWallConversion.at(GetItem(index)));
    } else if (CanRollLeft(index)) {    // Roll left/right
        RollLeft(index, kElDiamondFalling);
    } else if (CanRollRight(index)) {
        RollRight(index, kElDiamondFalling);
    } else {
        // Default options is for falling diamond to become stationary
        SetItem(index, kElDiamond, -1);
    }
}

void RNDGameState::UpdateNut(std::size_t index) noexcept {
    // If no gravity, do nothing
    if (!shared_state_ptr->gravity) {
        return;
    }

    // Nut falls if empty below
    if (IsType(index, kElEmpty, Direction::kDown)) {
        SetItem(index, kElNutFalling, -1);
        UpdateNutFalling(index);
    } else if (CanRollLeft(index)) {    // Roll left/right
        RollLeft(index, kElNutFalling);
    } else if (CanRollRight(index)) {
        RollRight(index, kElNutFalling);
    }
}

void RNDGameState::UpdateNutFalling(std::size_t index) noexcept {
    // Continue to fall as normal
    if (IsType(index, kElEmpty, Direction::kDown)) {
        MoveItem(index, Direction::kDown);
    } else if (CanRollLeft(index)) {    // Roll left/right
        RollLeft(index, kElNutFalling);
    } else if (CanRollRight(index)) {
        RollRight(index, kElNutFalling);
    } else {
        // Default options is for falling nut to become stationary
        SetItem(index, kElNut, -1);
    }
}

void RNDGameState::UpdateBomb(std::size_t index) noexcept {
    // If no gravity, do nothing
    if (!shared_state_ptr->gravity) {
        return;
    }

    // Bomb falls if empty below
    if (IsType(index, kElEmpty, Direction::kDown)) {
        SetItem(index, kElBombFalling, -1);
        UpdateBombFalling(index);
    } else if (CanRollLeft(index)) {    // Roll left/right
        RollLeft(index, kElBomb);
    } else if (CanRollRight(index)) {
        RollRight(index, kElBomb);
    }
}

void RNDGameState::UpdateBombFalling(std::size_t index) noexcept {
    // Continue to fall as normal
    if (IsType(index, kElEmpty, Direction::kDown)) {
        MoveItem(index, Direction::kDown);
    } else if (CanRollLeft(index)) {    // Roll left/right
        RollLeft(index, kElBombFalling);
    } else if (CanRollRight(index)) {
        RollRight(index, kElBombFalling);
    } else {
        // Default options is for bomb to explode if stopped falling
        const auto it = kElementToExplosion.find(GetItem(index));
        Explode(index, (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second);
    }
}

void RNDGameState::UpdateExit(std::size_t index) noexcept {
    // Open exit if enough gems collected
    if (local_state.gems_collected >= board.gems_required) {
        SetItem(index, kElExitOpen, -1);
    }
}

void RNDGameState::UpdateAgent(std::size_t index, Direction direction) noexcept {
    // If action results not in bounds, don't do anything
    if (!InBounds(index, direction)) {
        return;
    }

    if (IsType(index, kElEmpty, direction) || IsType(index, kElDirt, direction)) {    // Move if empty/dirt
        MoveItem(index, direction);
        board.agent_pos = IndexFromDirection(index, direction);
        board.agent_idx = IndexFromDirection(index, direction);
    } else if (IsType(index, kElDiamond, direction) || IsType(index, kElDiamondFalling, direction)) {    // Collect gems
        ++local_state.gems_collected;
        local_state.current_reward += kPointMap.at(GetItem(index, direction).cell_type);
        local_state.reward_signal |= RewardCodes::kRewardCollectDiamond;
        MoveItem(index, direction);
        RemoveIndexID(IndexFromDirection(index, direction));
        board.agent_pos = IndexFromDirection(index, direction);
        board.agent_idx = IndexFromDirection(index, direction);
    } else if (IsDirectionHorz(direction) && HasProperty(index, ElementProperties::kPushable, direction)) {
        // Push stone, nut, or bomb if action is horizontal
        Push(index, GetItem(index, direction), kElToFalling.at(GetItem(index, direction)), direction);
    } else if (IsKey(GetItem(index, direction))) {
        // Collecting key, set gate open
        const Element &key_type = GetItem(index, direction);
        OpenGate(kKeyToGate.at(key_type));
        // OpenGate(shared_state_ptr->key_swap ? kKeyToGateSwap.at(key_type) : kKeyToGate.at(key_type));
        MoveItem(index, direction);
        board.agent_pos = IndexFromDirection(index, direction);
        board.agent_idx = IndexFromDirection(index, direction);
        local_state.reward_signal |= RewardCodes::kRewardCollectKey;
        local_state.reward_signal |= static_cast<uint64_t>(kKeyToSignal.at(key_type));
    } else if (IsOpenGate(GetItem(index, direction))) {
        // Walking through an open gate, with traversable element on other side
        const std::size_t index_gate = IndexFromDirection(index, direction);
        if (HasProperty(index_gate, ElementProperties::kTraversable, direction)) {
            // Correct for landing on traversable elements
            if (IsType(index_gate, kElDiamond, direction) || IsType(index_gate, kElDiamondFalling, direction)) {
                ++local_state.gems_collected;
                local_state.current_reward += kPointMap.at(GetItem(index_gate, direction).cell_type);
                local_state.reward_signal |= RewardCodes::kRewardCollectDiamond;
            } else if (IsKey(GetItem(index_gate, direction))) {
                const Element &key_type = GetItem(index_gate, direction);
                OpenGate(kKeyToGate.at(key_type));
                local_state.reward_signal |= RewardCodes::kRewardCollectKey;
                local_state.reward_signal |= static_cast<uint64_t>(kKeyToSignal.at(key_type));
            }
            // Move agent through gate
            SetItem(index_gate, kElAgent, -1, direction);
            SetItem(index, kElEmpty, -1);
            board.agent_pos = IndexFromDirection(index_gate, direction);
            board.agent_idx = IndexFromDirection(index_gate, direction);
            local_state.reward_signal |= RewardCodes::kRewardWalkThroughGate;
            local_state.reward_signal |= static_cast<uint64_t>(kGateToSignal.at(GetItem(index_gate)));
        }
    } else if (IsType(index, kElExitOpen, direction)) {
        // Walking into exit after collecting enough gems
        MoveItem(index, direction);
        SetItem(index, kElAgentInExit, -1, direction);
        board.agent_pos = kAgentPosExit;
        board.agent_idx = IndexFromDirection(index, direction);
        local_state.reward_signal |= RewardCodes::kRewardWalkThroughExit;
        local_state.current_reward += local_state.steps_remaining;
    }
}

void RNDGameState::UpdateFirefly(std::size_t index, Direction direction) noexcept {
    // NOLINTBEGIN(*-bounds-constant-array-index)
    const Direction new_dir = kRotateLeft[static_cast<std::size_t>(direction)];
    if (IsTypeAdjacent(index, kElAgent) || IsTypeAdjacent(index, kElBlob)) {
        // Explode if touching the agent/blob
        const auto it = kElementToExplosion.find(GetItem(index));
        Explode(index, (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second);
    } else if (IsType(index, kElEmpty, new_dir)) {
        // Fireflies always try to rotate left, otherwise continue forward
        SetItem(index, kDirectionToFirefly[static_cast<std::size_t>(new_dir)], -1);
        MoveItem(index, new_dir);
    } else if (IsType(index, kElEmpty, direction)) {
        SetItem(index, kDirectionToFirefly[static_cast<std::size_t>(direction)], -1);
        MoveItem(index, direction);
    } else {
        // No other options, rotate right
        SetItem(index, kDirectionToFirefly[static_cast<std::size_t>(kRotateRight[static_cast<std::size_t>(direction)])],
                -1);
    }

    // NOLINTEND(*-bounds-constant-array-index)
}

void RNDGameState::UpdateButterfly(std::size_t index, Direction direction) noexcept {
    // NOLINTBEGIN(*-bounds-constant-array-index)
    const Direction new_dir = kRotateRight[static_cast<std::size_t>(direction)];
    if (IsTypeAdjacent(index, kElAgent) || IsTypeAdjacent(index, kElBlob)) {
        // Explode if touching the agent/blob
        const auto it = kElementToExplosion.find(GetItem(index));
        Explode(index, (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second);
    } else if (IsType(index, kElEmpty, new_dir)) {
        // Butterflies always try to rotate right, otherwise continue forward
        SetItem(index, kDirectionToButterfly[static_cast<std::size_t>(new_dir)], -1);
        MoveItem(index, new_dir);
    } else if (IsType(index, kElEmpty, direction)) {
        SetItem(index, kDirectionToButterfly[static_cast<std::size_t>(direction)], -1);
        MoveItem(index, direction);
    } else {
        // No other options, rotate right
        SetItem(index,
                kDirectionToButterfly[static_cast<std::size_t>(kRotateLeft[static_cast<std::size_t>(direction)])], -1);
    }
    // NOLINTEND(*-bounds-constant-array-index)
}

void RNDGameState::UpdateOrange(std::size_t index, Direction direction) noexcept {
    if (IsType(index, kElEmpty, direction)) {
        // Continue moving in direction
        MoveItem(index, direction);
    } else if (IsTypeAdjacent(index, kElAgent)) {
        // Run into the agent, explode!
        const auto it = kElementToExplosion.find(GetItem(index));
        Explode(index, (it == kElementToExplosion.end()) ? kElExplosionEmpty : it->second);
    } else {
        // Blocked, roll for new direction
        std::vector<Direction> open_dirs;
        for (int dir_index = 0; dir_index < kNumActions; ++dir_index) {
            const auto dir = static_cast<Direction>(dir_index);
            if (dir == Direction::kNoop || !InBounds(index, dir)) {
                continue;
            }
            if (IsType(index, kElEmpty, dir)) {
                open_dirs.push_back(dir);
            }
        }
        // Roll available directions
        if (!open_dirs.empty()) {
            const Direction new_dir = open_dirs[xorshift64(local_state.random_state) % open_dirs.size()];
            // NOLINTNEXTLINE(*-bounds-constant-array-index)
            SetItem(index, kDirectionToOrange[static_cast<std::size_t>(new_dir)], -1);
        }
    }
}

void RNDGameState::UpdateMagicWall(std::size_t index) noexcept {
    // Dorminant, active, then expired once time runs out
    if (local_state.magic_active) {
        SetItem(index, kElWallMagicOn, -1);
    } else if (local_state.magic_wall_steps > 0) {
        SetItem(index, kElWallMagicDormant, -1);
    } else {
        SetItem(index, kElWallMagicExpired, -1);
    }
}

constexpr int BASE_CHANCE = 256;

void RNDGameState::UpdateBlob(std::size_t index) noexcept {
    // Replace blobs if swap element set
    if (local_state.blob_swap != kNullElement.cell_type) {
        // NOLINTNEXTLINE(*-bounds-constant-array-index)
        SetItem(index, kCellTypeToElement[static_cast<std::size_t>(local_state.blob_swap) + 1], -1);
        AddIndexID(index);
        return;
    }
    ++local_state.blob_size;
    // Check if at least one tile blob can grow to
    if (IsTypeAdjacent(index, kElEmpty) || IsTypeAdjacent(index, kElDirt)) {
        local_state.blob_enclosed = false;
    }
    // Roll if to grow and direction
    const bool will_grow = (xorshift64(local_state.random_state) % BASE_CHANCE) < shared_state_ptr->blob_chance;
    const auto grow_dir = static_cast<Direction>(xorshift64(local_state.random_state) % kNumActions);
    if (will_grow && (IsType(index, kElEmpty, grow_dir) || IsType(index, kElDirt, grow_dir))) {
        SetItem(index, kElBlob, -1, grow_dir);
        // TODO test this
        RemoveIndexID(IndexFromDirection(index, grow_dir));
    }
}

void RNDGameState::UpdateExplosions(std::size_t index) noexcept {
    SetItem(index, kExplosionToElement.at(GetItem(index)), -1);
    AddIndexID(index);
}

void RNDGameState::OpenGate(const Element &element) noexcept {
    for (std::size_t index = 0; index < board.grid.size(); ++index) {
        if (board.item(index) == element.cell_type) {
            SetItem(index, kGateOpenMap.at(GetItem(index)), -1);
        }
    }
}

// ---------------------------------------------------------------------------

void RNDGameState::StartScan() noexcept {
    if (local_state.steps_remaining > 0) {
        local_state.steps_remaining += -1;
    }
    local_state.current_reward = 0;
    local_state.blob_size = 0;
    local_state.blob_enclosed = true;
    local_state.reward_signal = 0;
    board.reset_updated();
}

void RNDGameState::EndScan() noexcept {
    if (local_state.blob_swap == kNullElement.cell_type) {
        if (local_state.blob_enclosed) {
            local_state.blob_swap = kElDiamond.cell_type;
        }
        if (local_state.blob_size > shared_state_ptr->blob_max_size) {
            local_state.blob_swap = kElStone.cell_type;
        }
    }
    if (local_state.magic_active) {
        local_state.magic_wall_steps =
            static_cast<decltype(local_state.magic_wall_steps)>(std::max(local_state.magic_wall_steps - 1, 0));
    }
    local_state.magic_active = local_state.magic_active && (local_state.magic_wall_steps > 0);
}

}    // namespace stonesngems
