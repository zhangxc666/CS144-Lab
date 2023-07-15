#include "wrapping_integers.hh"

using namespace std;
typedef long long LL;

// absolute seqno -> seqno
Wrap32 Wrap32::wrap(uint64_t n, Wrap32 zero_point) {
    return zero_point + uint32_t(n % (uint64_t(1UL) << 32)); // 自然溢出
}

// seqno → absolute seqno.
/*
恶心坏了这个题
想了个o1的复杂度+压行+边界情况，可以保证code在十行以内
思路如下:
1.  选出距离当前checkpoint最近的两端-> absolute seq（seq为0的位置）
2.  找到mid -> absolute seq
3.  若当前的checkpoint > mid,则此时两端不动
    若当前的checkpoint < mid,则此时两端-(1<<32)
4.  之后比较l+diff和r+diff与checkpoint的长度，长度较小的就是答案
5.  两个特判的点：
        1. 若当前checkpoint在[0,mid]之间，则此时l>r,r=0,l不合法，故答案范围r+diff
        2. 若当前checkpoint在[(1<<64)-(1<<31)+1,(1<<64)-1]之间，则此时l>r,r=0,r不合法,最终返回l+dirr

证明：为什么checkpoint > mid，两端不动？
答:  若两端不动，当diff无论取任何值，都能保证raw_value的abs seq与checkpoint的差值在(1ULL<<31)以内
     若两端右移，当diff>(1UL<<31)时，无法保证aw_value的abs seq与checkpoint的差值在(1ULL<<31)以内
     若两端左移，当diff很小时,同理

     对于checkpoint < mid,则此时两端-(1<<32)，道理类似，重点是始终保证最后的答案与checkpoint的差值在[0,1UL<<31]的范围


*/
uint64_t Wrap32::unwrap(Wrap32 zero_point, uint64_t checkpoint) const {
    uint64_t lpoint = (checkpoint & (UINT64_MAX - UINT32_MAX)), rpoint = lpoint + (1UL << 32),mid=lpoint+(1UL<<31);
    if (checkpoint<=mid)rpoint = lpoint,lpoint = lpoint - (1UL << 32);
    uint64_t diff = (uint64_t(raw_value_) + (1UL << 32) - zero_point.raw_value_) % (1UL << 32);
    if (lpoint>rpoint)return rpoint + diff;
    if (rpoint == 0)return lpoint + diff;
    uint64_t l = lpoint + diff, r = rpoint + diff;
    LL dl = abs(LL(checkpoint - l)), dr = abs(LL(r - checkpoint));
    return dl < dr ? l : r;
}
