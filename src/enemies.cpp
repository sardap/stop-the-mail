#include <stdio.h>

#include <enemies.hpp>

namespace sm {

#pragma GCC push_options
#pragma GCC optimize("O0")
void step_mail_collsion(Mail& mail) {
    mail.collsion.rect.x = mail.postion.x;
    mail.collsion.rect.w = Fixed(16);
    mail.collsion.rect.y = mail.postion.y;
    mail.collsion.rect.h = Fixed(16);

    for (const auto event : mail.collsion.collisions) {
        if (!event) continue;

        switch (event->object.type) {
            case Identity::Type::INVALID:
                break;
            case Identity::Type::MAIL:
                break;
            case Identity::Type::TOWER:
                break;
        }
    }
}
#pragma GCC pop_options

}  // namespace sm
