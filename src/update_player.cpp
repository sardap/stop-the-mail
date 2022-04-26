#include <update_player.hpp>

namespace sm {

void update_player(Player& player, BulletinBoard& board) {
    if (board.mail_delivered > 0) {
        // TODO add sound
        player.life.currentHp -= Fixed(board.mail_delivered);
    }
}

}  // namespace sm