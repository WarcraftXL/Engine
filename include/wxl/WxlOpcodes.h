// WarcraftXL custom opcode assignments carried by wxl.network.
// Copyright (C) 2026 WarcraftXL. GPLv3.

#ifndef WXL_OPCODES_H
#define WXL_OPCODES_H

#include <stdint.h>

enum WXL_Opcode
{
    WXL_CMSG_QUEST_MARKER_REQUEST   = 0x051F,
    WXL_SMSG_QUEST_MARKER_UPDATE    = 0x0520,
    WXL_CMSG_RADIAL_PING            = 0x0521,
    WXL_SMSG_RADIAL_PING            = 0x0522,
    WXL_CMSG_RETAIL_ITEM_VARIANTS   = 0x0523,
    WXL_SMSG_RETAIL_ITEM_VARIANTS   = 0x0524,
    WXL_CMSG_SPELL_CHARGES_REQUEST  = 0x0525,
    WXL_SMSG_SPELL_CHARGES_UPDATE   = 0x0526,
    WXL_CMSG_SKYRIDING              = 0x0527,
    WXL_SMSG_SKYRIDING              = 0x0528,
    WXL_CMSG_MOVE_ADD_IMPULSE_ACK   = 0x0529,
    WXL_SMSG_MOVE_ADD_IMPULSE       = 0x052A,
};

#endif // WXL_OPCODES_H
