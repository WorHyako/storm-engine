#include "xdefines.h"

void to_json(storm::Data &data, const XYRECT &rect)
{
    data = storm::Data{
            {"left", rect.left},
            {"top", rect.top},
            {"right", rect.right},
            {"bottom", rect.bottom},
        };
}

void from_json(const storm::Data &data, XYRECT &rect)
{
    if (data.is_object()) {
        data.at("left").get_to(rect.left);
        data.at("top").get_to(rect.top);
        data.at("right").get_to(rect.right);
        data.at("bottom").get_to(rect.bottom);
    }
    else if (data.is_string()) {
        std::string str = data.get<std::string>();
    }
}