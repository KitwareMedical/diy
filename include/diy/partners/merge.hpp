#ifndef DIY_PARTNERS_MERGE_HPP
#define DIY_PARTNERS_MERGE_HPP

#include "common.hpp"

namespace diy
{

class Master;

/**
 * \ingroup Communication
 * \brief Partners for merge-reduce
 *
 */
struct RegularMergePartners: public RegularPartners
{
  typedef       RegularPartners                                 Parent;

                // contiguous parameter indicates whether to match partners contiguously or in a round-robin fashion;
                // contiguous is useful when data needs to be united;
                // round-robin is useful for vector-"halving"
                RegularMergePartners(int dim,                 //!< dimensionality of regular block grid
                                     int nblocks,             //!< total number of blocks
                                     int k,                   //!< target k value
                                     bool contiguous = true   //!< distance halving (true) or doubling (false)
                    ):
                    Parent(dim, nblocks, k, contiguous)         {}
                RegularMergePartners(const DivisionVector&   divs, //!< explicit division vector
                                     const KVSVector&        kvs,  //!< explicit k vector
                                     bool  contiguous = true       //!< distance halving (true) or doubling (false)
                    ):
                    Parent(divs, kvs, contiguous)               {}

  inline bool   active(int round, int gid, const Master&) const;

  // incoming is only valid for an active gid; it will only be called with an active gid
  inline void   incoming(int round, int gid, std::vector<int>& partners, const Master&) const    { Parent::fill(round - 1, gid, partners); }
  // this is a lazy implementation of outgoing, but it reuses the existing code
  inline void   outgoing(int round, int gid, std::vector<int>& partners, const Master&) const    { std::vector<int> tmp; Parent::fill(round, gid, tmp); partners.push_back(tmp[0]); }
};

} // diy

bool
diy::RegularMergePartners::
active(int round, int gid, const Master&) const
{
  CoordVector   coords;
  Decomposer::gid_to_coords(gid, coords, divisions());

  for (int r = 0; r < round; ++r)
      if (Parent::group_position(r, coords[kvs()[r].dim], step(r)) != 0)
          return false;

  return true;
}

#endif

