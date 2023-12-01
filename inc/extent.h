#pragma once

#define EXTENTS 30 // number of extents in inode

// represents a contiguous block on disk of data
struct extent {
  uint startblkno; // start block number
  uint nblocks;    // n blocks following the start block
};
