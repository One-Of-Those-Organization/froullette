#pragma once

#include "Items.hpp"

class ItemRevealer: public Items {
    public:
        ItemRevealer(): Items("Revealer") { this->callback = this->the_callback; };
        virtual ~ItemRevealer = default;

        void the_callback(Player *player, void *additional_data) {
            (void)player;
            (void)additional_data;
        }
};
