#ifndef STONESNGEMS_BASE_H_
#define STONESNGEMS_BASE_H_

#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "definitions.h"

namespace stonesngems {

constexpr int SPRITE_WIDTH = 32;
constexpr int SPRITE_HEIGHT = 32;
constexpr int SPRITE_CHANNELS = 3;
constexpr int SPRITE_DATA_LEN_PER_ROW = SPRITE_WIDTH * SPRITE_CHANNELS;
constexpr int SPRITE_DATA_LEN = SPRITE_WIDTH * SPRITE_HEIGHT * SPRITE_CHANNELS;

// Game parameter can be boolean, integral or floating point
using GameParameter = std::variant<bool, int, float, std::string>;
using GameParameters = std::unordered_map<std::string, GameParameter>;

// Default game parameters
constexpr bool DEFAULT_SHOW_OBS_IDS = false;
constexpr int DEFAULT_MAGIC_WALL_STEPS = 140;
constexpr int DEFAULT_BLOB_CHANCE = 20;
constexpr float DEFAULT_BLOB_MAX_PERCENTAGE = static_cast<float>(0.16);
constexpr int DEFAULT_RNG_SEED = 0;
const std::string DEFAULT_GAME_BOARD_STR =
    "22|40|1280|12|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|"
    "19|19|19|19|19|19|19|19|02|02|02|02|02|02|01|02|02|05|02|03|01|02|02|02|02|02|03|02|03|02|02|02|02|02|02|02|01|02|"
    "02|02|02|03|02|02|02|02|19|19|01|03|00|03|02|02|02|02|02|02|01|02|02|02|02|02|02|02|02|02|03|05|02|02|03|02|02|02|"
    "02|01|02|02|02|02|02|01|02|02|19|19|02|02|02|02|02|02|02|02|02|02|01|02|02|03|02|02|02|02|02|03|02|03|02|02|03|02|"
    "02|02|02|02|02|02|02|03|02|02|02|02|19|19|03|02|03|03|02|02|02|02|02|02|02|02|02|03|02|02|02|02|02|02|03|02|02|03|"
    "02|02|02|02|03|02|02|02|03|02|02|02|02|02|19|19|03|02|01|03|02|02|02|02|02|02|02|02|02|01|03|02|02|03|02|02|02|02|"
    "02|02|02|02|03|02|02|02|02|02|02|03|02|03|03|02|19|19|02|02|02|01|02|02|03|02|02|02|02|02|02|02|02|03|02|02|02|02|"
    "02|03|02|01|03|02|02|02|02|02|02|02|02|03|02|03|03|02|19|19|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|"
    "18|18|18|18|18|18|18|18|18|18|18|18|02|02|02|03|02|02|03|02|19|19|02|01|02|02|02|03|02|02|05|02|01|02|02|03|02|03|"
    "02|02|02|02|02|02|02|02|02|02|05|02|03|05|02|02|02|02|02|02|01|02|19|19|02|02|05|02|02|02|02|02|03|02|02|02|02|02|"
    "01|02|02|02|02|02|02|02|02|03|03|01|03|02|02|03|02|02|02|02|03|02|02|02|19|19|02|02|02|03|02|02|03|02|03|02|02|02|"
    "02|02|02|02|02|02|02|02|02|02|02|03|01|02|03|02|02|03|02|02|02|02|02|02|02|02|19|19|02|03|02|02|02|02|02|03|02|02|"
    "02|02|02|02|02|02|03|03|03|02|02|02|02|02|02|02|03|02|02|01|02|05|02|02|02|02|03|02|19|19|02|05|02|02|01|02|02|03|"
    "02|01|01|02|02|02|02|02|03|02|03|05|02|02|05|02|02|02|02|03|02|02|02|03|02|02|05|02|01|02|19|19|02|01|03|02|02|02|"
    "02|02|02|02|02|02|02|02|02|02|02|03|01|03|02|02|03|02|02|02|02|02|02|02|02|05|02|02|02|02|02|03|19|19|02|02|02|02|"
    "02|02|02|02|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|19|19|01|03|"
    "02|02|02|02|02|02|02|02|02|03|02|02|02|05|02|02|02|02|03|02|02|02|02|02|03|02|02|02|03|02|02|02|02|02|02|02|19|19|"
    "01|03|02|02|02|02|02|02|02|02|02|01|03|02|02|03|02|02|02|02|02|02|02|02|03|02|02|02|02|02|02|03|02|03|03|02|02|07|"
    "19|19|02|01|02|02|03|02|02|02|02|02|02|02|02|03|02|02|02|02|02|03|02|01|01|02|02|02|02|05|02|02|02|03|02|03|03|02|"
    "02|02|19|19|02|02|02|02|03|05|02|02|03|02|02|02|02|02|02|02|02|03|02|02|02|02|02|02|03|02|03|05|02|02|02|02|02|02|"
    "03|02|02|02|19|19|02|02|02|01|02|02|03|02|01|02|02|03|02|03|03|02|02|02|02|02|02|02|02|02|03|02|03|05|02|02|02|02|"
    "02|02|01|02|02|03|19|19|02|05|02|02|02|02|01|02|02|02|02|02|01|02|02|02|02|02|02|02|02|02|01|02|03|02|02|03|02|02|"
    "02|02|03|02|02|02|03|02|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|19|"
    "19|19|19|19|19|19|19|19|19|19|19";
constexpr bool DEFAULT_GRAVITY = true;
constexpr int DEFAULT_BLOB_SWAP = -1;

static const GameParameters kDefaultGameParams{
    {"obs_show_ids",
     GameParameter(DEFAULT_SHOW_OBS_IDS)},    // Flag to show object ids in observation instead of binary channels
    {"magic_wall_steps", GameParameter(DEFAULT_MAGIC_WALL_STEPS)},    // Number of steps before magic wall expire
    {"blob_chance", GameParameter(DEFAULT_BLOB_CHANCE)},              // Chance to spawn another blob (out of 256)
    {"blob_max_percentage",
     GameParameter(
         DEFAULT_BLOB_MAX_PERCENTAGE)},    // Max number of blobs before they collapse (percentage of map size)
    {"rng_seed", GameParameter(DEFAULT_RNG_SEED)},                // Seed for anything that uses the rng
    {"game_board_str", GameParameter(DEFAULT_GAME_BOARD_STR)},    // Game board string
    {"gravity", GameParameter(DEFAULT_GRAVITY)},                  // Game board string
    {"blob_swap", GameParameter(DEFAULT_BLOB_SWAP)},              // Blob swap hidden element
};

// Shared global state information relevant to all states for the given game
struct SharedStateInfo {
    SharedStateInfo(GameParameters params_)
        : params(std::move(params_)),
          obs_show_ids(std::get<bool>(params.at("obs_show_ids"))),
          magic_wall_steps(std::get<int>(params.at("magic_wall_steps"))),
          blob_chance(static_cast<uint8_t>(std::get<int>(params.at("blob_chance")))),
          blob_max_percentage(std::get<float>(params.at("blob_max_percentage"))),
          rng_seed(std::get<int>(params.at("rng_seed"))),
          game_board_str(std::get<std::string>(params.at("game_board_str"))),
          gravity(std::get<bool>(params.at("gravity"))) {}
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    GameParameters params;                              // Copy of game parameters for state resetting
    bool obs_show_ids;                                  // Flag to show object IDs (currently not used)
    int magic_wall_steps;                               // Number of steps the magic wall stays active for
    uint8_t blob_chance;                                // Chance (out of 256) for blob to spawn
    int blob_max_size = 0;                              // Max blob size in terms of grid spaces
    float blob_max_percentage;                          // Max blob size as percentage of map size
    int rng_seed;                                       // Seed
    std::string game_board_str;                         // String representation of the starting state
    bool gravity;                                       // Flag if gravity is on, affects stones/gems
    std::unordered_map<std::size_t, uint64_t> zrbht;    // Zobrist hashing table
    std::vector<bool> in_bounds_board;                  // Fast check for single-step in bounds
    std::vector<std::size_t> board_to_inbounds;         // Indexing conversion for in bounds checking
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

// Information specific for the current game state
struct IndexId {
    std::size_t index;
    int id;
};
struct LocalState {
    bool operator==(const LocalState &other) const {
        return magic_wall_steps == other.magic_wall_steps && blob_size == other.blob_size &&
               gems_collected == other.gems_collected && magic_active == other.magic_active &&
               blob_enclosed == other.blob_enclosed;
    }
    using id_type = uint16_t;
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    std::vector<IndexId> index_id_mappings;
    uint64_t random_state = 1;                           // State of Xorshift rng
    uint64_t reward_signal = 0;                          // Signal for external information about events
    int steps_remaining = -1;                            // Number of steps remaining (if timeout set)
    int gems_collected = 0;                              // Number of gems collected
    int current_reward = 0;                              // Reward for the current game state
    int magic_wall_steps = 0;                            // Number of steps remaining for the magic wall
    int blob_size = 0;                                   // Current size of the blob
    int id_state = 0;                                    // Current ID state
    HiddenCellType blob_swap = HiddenCellType::kNull;    // Swap element when the blob vanishes
    bool magic_active = false;                           // Flag if magic wall is currently active
    bool blob_enclosed = true;                           // Flag if blob is enclosed
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

// Game state
class RNDGameState {
public:
    using Position = std::pair<std::size_t, std::size_t>;

    RNDGameState(const GameParameters &params = kDefaultGameParams);

    auto operator==(const RNDGameState &other) const noexcept -> bool;
    auto operator!=(const RNDGameState &other) const noexcept -> bool;

    /**
     * Reset the environment to the state as given by the GameParameters
     */
    void reset();

    /**
     * Apply the action to the current state, and set the reward and signals.
     * @param action The action to apply, should be one of the legal actions
     */
    void apply_action(Action action) noexcept;

    /**
     * Check if the state is terminal, meaning either solution, timeout, or agent dies.
     * @return True if terminal, false otherwise
     */
    [[nodiscard]] auto is_terminal() const noexcept -> bool;

    /**
     * Check if the state is in the solution state (agent inside exit).
     * @return True if terminal, false otherwise
     */
    [[nodiscard]] auto is_solution() const noexcept -> bool;

    /**
     * Get the legal actions which can be applied in the state.
     * @return vector containing each actions available
     */
    [[nodiscard]] auto legal_actions() const noexcept -> std::vector<Action>;

    /**
     * Get the legal actions which can be applied in the state, and store in the given vector.
     * @note Use when wanting to reuse a pre-allocated vector
     * @param actions The vector to store the available actions in
     */
    void legal_actions(std::vector<Action> &actions) const noexcept;

    /**
     * Get the number of possible actions
     * @return Count of possible actions
     */
    [[nodiscard]] constexpr static auto action_space_size() noexcept -> std::size_t {
        return kNumActions;
    }

    /**
     * Get the shape the observations should be viewed as.
     * @return vector indicating observation CHW
     */
    [[nodiscard]] auto observation_shape() const noexcept -> std::array<std::size_t, 3>;

    /**
     * Get a flat representation of the current state observation.
     * The observation should be viewed as the shape given by observation_shape().
     * @return vector where 1 represents object at position
     */
    [[nodiscard]] auto get_observation() const noexcept -> std::vector<float>;

    /**
     * Get a flat representation of the current state observation, and store in the given vector.
     * @note Use when wanting to reuse a pre-allocated vector
     * The observation should be viewed as the shape given by observation_shape(), where 1 represents the element at the
     * given position.
     * @param obs Vector to store the observation in
     */
    void get_observation(std::vector<float> &obs) const noexcept;

    /**
     * Get a flat representation of the current state observation.
     * The observation should be viewed as the shape given by observation_shape().
     * @return vector where 1 represents object at position
     */
    [[nodiscard]] auto get_observation(const std::vector<VisibleCellType> &filter_elements) const noexcept
        -> std::vector<float>;

    /**
     * Get the index corresponding to the given position
     * @return the flat index
     */
    [[nodiscard]] auto position_to_index(const Position &position) const noexcept -> std::size_t;

    /**
     * Get the position corresponding to the given index
     * @return The position
     */
    [[nodiscard]] auto index_to_position(std::size_t index) const noexcept -> Position;

    /**
     * Get the shape the image should be viewed as.
     * @return array indicating observation HWC
     */
    [[nodiscard]] auto image_shape() const noexcept -> std::array<std::size_t, 3>;

    /**
     * Get the flat (HWC) image representation of the current state
     * @return flattened byte vector represending RGB values (HWC)
     */
    [[nodiscard]] auto to_image() const noexcept -> std::vector<uint8_t>;

    /**
     * Get the string representation of the underlying board
     */
    [[nodiscard]] auto board_to_str() const noexcept -> std::string;

    /**
     * Get the current reward signal as a result of the previous action taken.
     * @return bit field representing events that occured
     */
    [[nodiscard]] auto get_reward_signal() const noexcept -> uint64_t;

    /**
     * Get the hash representation for the current state.
     * @return hash value
     */
    [[nodiscard]] auto get_hash() const noexcept -> uint64_t;

    /**
     * Get all positions for a given element type
     * @param element The hidden cell type of the element to search for
     * @return pair of positions for each instance of element
     */
    [[nodiscard]] auto get_positions(HiddenCellType element) const noexcept -> std::vector<Position>;

    /**
     * Get all indices for a given element type
     * @param element The hidden cell type of the element to search for
     * @return flat indices for each instance of element
     */
    [[nodiscard]] auto get_indices(HiddenCellType element) const noexcept -> std::vector<std::size_t>;

    /**
     * Check if a given position is in bounds
     * @param position The position to check
     * @return True if the position is in bounds, false otherwise
     */
    [[nodiscard]] auto is_pos_in_bounds(const Position &position) const noexcept -> bool;

    /**
     * Get the ID of the element at the given flat index
     * @param index The index to query
     * @return the ID of the element if trackable, else -1
     */
    [[nodiscard]] auto get_index_id(std::size_t index) const noexcept -> int;

    /**
     * Get the index of the element for the given ID
     * @param id The id to query
     * @return the flat index of the element if trackable, else numeric_limits<std::size_t>::max()
     */
    [[nodiscard]] auto get_id_index(int id) const noexcept -> std::size_t;

    /**
     * Get all possible reward codes from the current state
     * @return )set of possible reward codes from the state
     */
    [[nodiscard]] auto get_valid_rewards() const noexcept -> std::unordered_set<RewardCodes>;

    /**
     * Get the agent index position, or code if in exit (solution) or dead (failure)
     * @return Agent index, kAgentPosExit if in exit (solution), or kAgentPosDead if dead
     */
    [[nodiscard]] auto get_agent_pos() const noexcept -> std::size_t;

    /**
     * Get the agent index position, even if in exit
     * @return Agent index
     */
    [[nodiscard]] auto get_agent_index() const noexcept -> std::size_t;

    /**
     * Get the hidden cell item at the given index
     */
    [[nodiscard]] auto get_hidden_item(std::size_t index) const noexcept -> HiddenCellType;

    // All possible actions
    static const std::vector<Action> ALL_ACTIONS;

    friend auto operator<<(std::ostream &os, const RNDGameState &state) -> std::ostream &;

private:
    [[nodiscard]] auto IndexFromDirection(std::size_t index, Direction direction) const noexcept -> std::size_t;
    [[nodiscard]] auto BoundsIndexFromDirection(std::size_t index, Direction direction) const noexcept -> std::size_t;
    [[nodiscard]] auto InBounds(std::size_t index, Direction direction = Direction::kNoop) const noexcept -> bool;
    [[nodiscard]] auto IsType(std::size_t index, const Element &element,
                              Direction direction = Direction::kNoop) const noexcept -> bool;
    [[nodiscard]] auto HasProperty(std::size_t index, int property,
                                   Direction direction = Direction::kNoop) const noexcept -> bool;
    void UpdateIDIndex(std::size_t index_old, std::size_t index_new) noexcept;
    void UpdateIndexID(std::size_t index) noexcept;
    void AddIndexID(std::size_t index) noexcept;
    void RemoveIndexID(std::size_t index) noexcept;
    void MoveItem(std::size_t index, Direction direction) noexcept;
    void SetItem(std::size_t index, const Element &element, int id, Direction direction = Direction::kNoop) noexcept;
    [[nodiscard]] auto GetItem(std::size_t index,
                               Direction direction = Direction::kNoop) const noexcept -> const Element &;
    [[nodiscard]] auto IsTypeAdjacent(std::size_t index, const Element &element) const noexcept -> bool;

    [[nodiscard]] auto CanRollLeft(std::size_t index) const noexcept -> bool;
    [[nodiscard]] auto CanRollRight(std::size_t index) const noexcept -> bool;
    void RollLeft(std::size_t index, const Element &element) noexcept;
    void RollRight(std::size_t index, const Element &element) noexcept;
    void Push(std::size_t index, const Element &stationary, const Element &falling, Direction direction) noexcept;
    void MoveThroughMagic(std::size_t index, const Element &element) noexcept;
    void Explode(std::size_t index, const Element &element, Direction direction = Direction::kNoop) noexcept;

    void UpdateStone(std::size_t index) noexcept;
    void UpdateStoneFalling(std::size_t index) noexcept;
    void UpdateDiamond(std::size_t index) noexcept;
    void UpdateDiamondFalling(std::size_t index) noexcept;
    void UpdateNut(std::size_t index) noexcept;
    void UpdateNutFalling(std::size_t index) noexcept;
    void UpdateBomb(std::size_t index) noexcept;
    void UpdateBombFalling(std::size_t index) noexcept;
    void UpdateExit(std::size_t index) noexcept;
    void UpdateAgent(std::size_t index, Direction direction) noexcept;
    void UpdateFirefly(std::size_t index, Direction direction) noexcept;
    void UpdateButterfly(std::size_t index, Direction direction) noexcept;
    void UpdateOrange(std::size_t index, Direction direction) noexcept;
    void UpdateMagicWall(std::size_t index) noexcept;
    void UpdateBlob(std::size_t index) noexcept;
    void UpdateExplosions(std::size_t index) noexcept;
    void OpenGate(const Element &element) noexcept;

    void StartScan() noexcept;
    void EndScan() noexcept;

    std::shared_ptr<SharedStateInfo> shared_state_ptr;
    Board board;
    LocalState local_state;
};

}    // namespace stonesngems

#endif    // STONESNGEMS_BASE_H_
