#include <rnd/stonesngems.h>

using namespace stonesngems;

void test_serialization() {
    const std::string board_str =
        "14|14|-1|1|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18|07|01|01|18|01|01|01|01|18|02|02|05|18|18|02|01|01|18|"
        "02|02|02|02|18|02|32|01|18|18|01|01|02|36|02|02|02|01|18|01|01|02|18|18|18|18|18|18|01|01|01|01|18|34|18|18|"
        "18|18|01|02|02|01|01|02|02|02|01|02|02|02|18|18|02|02|02|35|02|01|02|02|02|02|01|01|18|18|01|01|02|02|01|02|"
        "02|01|02|02|01|01|18|18|02|02|02|01|02|01|01|02|01|01|02|02|18|18|18|18|18|18|00|02|01|01|18|18|18|18|18|18|"
        "01|01|29|18|02|01|02|02|18|02|01|02|18|18|02|01|02|18|02|01|02|02|18|02|02|01|18|18|01|01|01|31|01|01|02|01|"
        "28|01|38|02|18|18|18|18|18|18|18|18|18|18|18|18|18|18|18";
    GameParameters params = kDefaultGameParams;
    params["game_board_str"] = GameParameter(board_str);

    RNDGameState state(params);
    state.apply_action(Action(1));

    std::vector<uint8_t> bytes = state.serialize();

    RNDGameState state_copy(bytes);

    if (state != state_copy) {
        std::cout << "serialization error." << std::endl;
    }
    std::cout << state << std::endl;
    std::cout << state.get_hash() << std::endl;

    if (state.get_hash() != state_copy.get_hash()) {
        std::cout << "serialization error." << std::endl;
    }
    std::cout << state_copy << std::endl;
    std::cout << state_copy.get_hash() << std::endl;
}

int main() {
    test_serialization();
}
