#include "../interfaces/input.hpp"

user_cmd *get_user_cmd(void *me, int sequence_number) {
    return input->get_user_cmd(sequence_number);
}
