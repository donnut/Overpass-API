#ifndef TYPE_AREA_DEFINED
#define TYPE_AREA_DEFINED

#include <cstring>
#include <map>
#include <set>
#include <vector>

#include "basic_types.h"

using namespace std;

struct Aligned_Segment
{
  uint32 ll_upper_;
  uint64 ll_lower_a, ll_lower_b;
};

struct Area
{
//   uint32 id;
//   uint32 index;
//   vector< uint32 > nds;
//   vector< pair< string, string > > tags;
//   
//   Way() : id(0), index(0) {}
//   
//   Way(uint32 id_)
//   : id(id_), index(0)
//   {}
//   
//   Way(uint32 id_, uint32 index_, const vector< uint32 >& nds_)
//   : id(id_), index(index_), nds(nds_) {}
//   
//   static uint32 calc_index(const vector< uint32 >& nd_idxs)
//   {
//     if (nd_idxs.empty())
//       return 0;
//     
//     uint32 bitmask(0), value(nd_idxs[0]);
//     for (uint i(1); i < nd_idxs.size(); ++i)
//       bitmask |= (value ^ nd_idxs[i]);
//     if (bitmask & 0xff000000)
//       value = 0x80000040;
//     else if (bitmask & 0xffff0000)
//       value = (value & 0xff000000) | 0x80000030;
//     else if (bitmask & 0xffffff00)
//       value = (value & 0xffff0000) | 0x80000020;
//     else if (bitmask)
//       value = (value & 0xffffff00) | 0x80000010;
//     
//     return value;
//   }
// };
// 
// struct Way_Comparator_By_Id {
//   bool operator() (const Way& a, const Way& b)
//   {
//     return (a.id < b.id);
//   }
// };
// 
// struct Way_Equal_Id {
//   bool operator() (const Way& a, const Way& b)
//   {
//     return (a.id == b.id);
//   }
  static Aligned_Segment segment_from_ll_quad
      (uint32 from_lat, int32 from_lon, uint32 to_lat, int32 to_lon)
  {
    Aligned_Segment result;
    uint32 a_ll_upper(Node::ll_upper(from_lat, from_lon));
    uint32 b_ll_upper(Node::ll_upper(to_lat, to_lon));
    result.ll_upper_ = a_ll_upper & 0xffffff00;
    result.ll_lower_a = (uint64)Node::ll_lower(from_lat, from_lon) |
    (((uint64)a_ll_upper & 0xff)<<32);
    result.ll_lower_b = (uint64)Node::ll_lower(to_lat, to_lon) |
    (((uint64)b_ll_upper & 0xff)<<32);
    
    return result;
  }
  
  static int32 proportion(int32 clow, int32 cmid, int32 cup, int32 low, int32 up)
  {
    return ((int64)(up - low))*(cmid - clow)/(cup - clow) + low;
  }
  
  static void calc_horiz_aligned_segments
      (vector< Aligned_Segment >& aligned_segments,
       uint32 from_lat, uint32 from_lon, uint32 to_lat, uint32 to_lon)
  {
    if ((from_lat & 0xfff00000) == (to_lat & 0xfff00000))
    {
      aligned_segments.push_back(segment_from_ll_quad
      (from_lat, from_lon, to_lat, to_lon));
    }
    else if (from_lat < to_lat)
    {
      uint32 split_lat((from_lat & 0xfff00000) + 0x100000);
      aligned_segments.push_back(segment_from_ll_quad
          (from_lat, from_lon, split_lat - 1,
           proportion(from_lat, split_lat - 1, to_lat, from_lon, to_lon)));
      for (; split_lat < (to_lat & 0xfff00000); split_lat += 0x100000)
      {
	aligned_segments.push_back(segment_from_ll_quad
	    (split_lat, proportion(from_lat, split_lat, to_lat, from_lon, to_lon),
	     split_lat + 0xfffff,
	     proportion(from_lat, split_lat + 0xfffff, to_lat, from_lon, to_lon)));
      }
      aligned_segments.push_back(segment_from_ll_quad
          (split_lat, proportion(from_lat, split_lat, to_lat, from_lon, to_lon),
           to_lat, to_lon));
    }
    else
    {
      uint32 split_lat((to_lat & 0xfff00000) + 0x100000);
      aligned_segments.push_back(segment_from_ll_quad
          (to_lat, to_lon, split_lat - 1,
           proportion(to_lat, split_lat - 1, from_lat, to_lon, from_lon)));
      for (; split_lat < (to_lat & 0xfff00000); split_lat += 0x100000)
      {
	aligned_segments.push_back(segment_from_ll_quad
	    (split_lat, proportion(to_lat, split_lat, from_lat, to_lon, from_lon),
	     split_lat + 0xfffff,
	     proportion(to_lat, split_lat + 0xfffff, from_lat, to_lon, from_lon)));
      }
      aligned_segments.push_back(segment_from_ll_quad
          (split_lat, proportion(to_lat, split_lat, from_lat, to_lon, from_lon),
           from_lat, from_lon));
    }
  }
  
  static void calc_vert_aligned_segments
      (vector< Aligned_Segment >& aligned_segments,
       uint32 from_lat, uint32 from_lon, uint32 to_lat, uint32 to_lon)
  {
    if ((from_lon & 0xfff00000) == (to_lon & 0xfff00000))
    {
      calc_horiz_aligned_segments
      (aligned_segments, from_lat, from_lon, to_lat, to_lon);
      return;
    }
    else if (from_lon < to_lon)
    {
      int32 split_lon((from_lon & 0xfff00000) + 0x100000);
      calc_horiz_aligned_segments
          (aligned_segments, from_lat, from_lon,
           proportion(from_lon, split_lon - 1, to_lon, from_lat, to_lat), split_lon - 1);
      for (; split_lon < (to_lon & 0xfff00000); split_lon += 0x100000)
	calc_horiz_aligned_segments
	    (aligned_segments,
	     proportion(from_lon, split_lon, to_lon, from_lat, to_lat), split_lon,
	     proportion(from_lon, split_lon + 0xfffff, to_lon, from_lat, to_lat),
	     split_lon + 0xfffff);
      calc_horiz_aligned_segments
	  (aligned_segments,
	   proportion(from_lon, split_lon, to_lon, from_lat, to_lat), split_lon,
	   to_lat, to_lon);
    }
    else
    {
      int32 split_lon((to_lon & 0xfff00000) + 0x100000);
      calc_horiz_aligned_segments
          (aligned_segments, to_lat, to_lon,
           proportion(to_lon, split_lon - 1, from_lon, to_lat, from_lat), split_lon - 1);
      for (; split_lon < (from_lon & 0xfff00000); split_lon += 0x100000)
	calc_horiz_aligned_segments
	    (aligned_segments,
	     proportion(to_lon, split_lon, from_lon, to_lat, from_lat), split_lon,
	     proportion(to_lon, split_lon + 0xfffff, from_lon, to_lat, from_lat),
	     split_lon + 0xfffff);
      calc_horiz_aligned_segments
	  (aligned_segments,
	   proportion(to_lon, split_lon, from_lon, to_lat, from_lat), split_lon,
	   from_lat, from_lon);
    }
  }
  
  static void calc_aligned_segments
      (vector< Aligned_Segment >& aligned_segments,
       uint64 from, uint64 to)
  {
    uint32 from_lat(0), to_lat(0);
    int32 from_lon(0), to_lon(0);
    
    for (uint32 i(0); i < 16; i+=1)
    {
      from_lat |= (((uint64)0x1<<(31-2*i))&(from>>32))<<i;
      from_lat |= (((uint64)0x1<<(31-2*i))&from)>>(16-i);
    }
    for (uint32 i(0); i < 16; i+=1)
    {
      to_lat |= (((uint64)0x1<<(31-2*i))&(to>>32))<<i;
      to_lat |= (((uint64)0x1<<(31-2*i))&to)>>(16-i);
    }
    for (uint32 i(0); i < 16; i+=1)
    {
      from_lon |= (((uint64)0x1<<(30-2*i))&(from>>32))<<(i+1);
      from_lon |= (((uint64)0x1<<(30-2*i))&from)>>(15-i);
    }
    for (uint32 i(0); i < 16; i+=1)
    {
      to_lon |= (((uint64)0x1<<(30-2*i))&(to>>32))<<(i+1);
      to_lon |= (((uint64)0x1<<(30-2*i))&to)>>(15-i);
    }
    
    if ((from_lon < -900000000) && (to_lon > 900000000))
    {
      calc_vert_aligned_segments
          (aligned_segments, to_lat, to_lon,
           proportion(to_lon, 1800000000, (uint64)from_lon + 3600000000ul,
		      to_lat, from_lat), 1800000000);
      calc_vert_aligned_segments
          (aligned_segments,
           proportion((uint64)to_lon - 3600000000ul, -1800000000, from_lon,
		      to_lat, from_lat), -1800000000, from_lat, from_lon);
    }
    else if ((to_lon < -900000000) && (from_lon > 900000000))
    {
      calc_vert_aligned_segments
          (aligned_segments, from_lat, from_lon,
	   proportion(from_lon, 1800000000, (uint64)to_lon + 3600000000ul,
		      from_lat, to_lat), 1800000000);
      calc_vert_aligned_segments
          (aligned_segments,
	   proportion((uint64)from_lon - 3600000000ul, -1800000000, to_lon,
		      from_lat, to_lat), -1800000000, to_lat, to_lon);
    }
    else
      calc_vert_aligned_segments
        (aligned_segments, from_lat, from_lon, to_lat, to_lon);
  }
};

struct Area_Block
{
  uint32 id;
  vector< uint64 > coors;
  
  Area_Block() {}
  
  Area_Block(void* data)
  {
    id = *(uint32*)data;
    coors.resize(*((uint16*)data + 2));
    for (int i(0); i < *((uint16*)data + 2); ++i)
      coors[i] = (*(uint64*)((uint8*)data + 6 + 5*i)) & (uint64)0xffffffffffull;
  }
  
  Area_Block(uint32 id_, const vector< uint64 >& coors_)
  : id(id_), coors(coors_) {}
  
  uint32 size_of() const
  {
    return 6 + 5*coors.size();
  }
  
  static uint32 size_of(void* data)
  {
    return (6 + 5 * *((uint16*)data + 2));
  }
  
  void to_data(void* data) const
  {
    *(uint32*)data = id;
    *((uint16*)data + 2) = coors.size();
    for (uint i(0); i < coors.size(); ++i)
    {
      *(uint32*)((uint8*)data + 6 + 5*i) = coors[i];
      *((uint8*)data + 10 + 5*i) = (coors[i])>>32;
    }
  }
  
  bool operator<(const Area_Block& a) const
  {
    if (this->id < a.id)
      return true;
    else if (this->id > a.id)
      return false;
    return (this->coors < a.coors);
  }
  
  bool operator==(const Area_Block& a) const
  {
    return ((this->id == a.id) && (this->coors == a.coors));
  }
};

#endif