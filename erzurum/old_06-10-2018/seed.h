#ifndef _ERZURUM_SEED_H_
#define _ERZURUM_SEED_H_

#include "tablebase.h"

/**
 * @brief Generate all the possible terminal positions for K vs. K endgames
 * @return
 *
 * Upper Bound: 7680 positions
 * --------------------------
 * 64 positions for white king
 * 60 positions for black king
 * 2 sides to move
 */
TableBase GenerateKings();

/**
 * @brief Generate all the possible terminal positions for KR vs. K endgames
 * @return
 * 
 * Upper Bound: 460800 positions
 * -----------------------------
 * 64 positions for checkmated king
 * 60 positions for checkmating king
 * 60 positions for checkmating rook
 * 2 sides to move
 * 
 * Subsets: K vs. K
 */
TableBase GenerateRookAndKing();

/**
 * @brief Add KR vs. K positions that were impossible to reach by frontier
 * @param tb
 */
void PatchRookAndKing(TableBase & tb);

/**
 * @brief Generate all the possible terminal positions for KBB vs. K endgames
 * @return
 * 
 * Upper Bound: 7864320 positions
 * ------------------------------
 * 64 positions for checkmated king
 * 60 positions for checkmating king
 * 32 positions for checkmating light-squared bishop
 * 32 positions for checkmating dark-squared bishop
 * 2 sides to move
 * 
 * Subsets: K vs. K, KB vs. K
 */
TableBase GenerateTwoBishopsAndKing();

#endif