/* SPDX-License-Identifier: MIT */
#include "battleship/types.h"
#include "battleship/version.h"

const char *bs_version(void)
{
    return BS_VERSION_STRING;
}

const char *bs_status_str(bs_status status)
{
    switch (status) {
    case BS_OK:
        return "success";
    case BS_ERR_INVALID_ARGUMENT:
        return "invalid argument";
    case BS_ERR_OUT_OF_BOUNDS:
        return "position outside the board";
    case BS_ERR_COLLISION:
        return "ship overlaps or touches another ship";
    case BS_ERR_FLEET_FULL:
        return "fleet is already complete";
    case BS_ERR_ALREADY_SHOT:
        return "cell has already been fired at";
    case BS_ERR_WRONG_PHASE:
        return "action not allowed in the current phase";
    case BS_ERR_PLACEMENT_FAILED:
        return "could not place the fleet";
    }
    return "unknown status";
}
