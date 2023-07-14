#include "reassembler.hh"

using namespace std;

void Reassembler::insert(uint64_t first_index, string data, bool is_last_substring, Writer &output) {
    if (is_last_substring)isFinish = true;
    if (data.size() == 0) {
        if (isFinish and totalBytes == 0)output.close();
        return;
    }
    uint64_t available_size = output.available_capacity(); // 剩余容量
    uint64_t end = pushtotalBytes - 1 + available_size; // 最后一个可容纳的最后一个字节索引
    uint64_t st = pushtotalBytes;       // 第一个可容纳的字节索引
    uint64_t l = first_index;
    uint64_t r = first_index + data.size() - 1;
    // [l,r]和[st,end]必须要有重叠
    if (l > end || r < st) {
        if (isFinish and totalBytes == 0)output.close();
        return;
    }
    if (l < st) {  // 若当前一部分数据已经被取走过了
        data = data.substr(st - l);
        l = st;
    }
    if (r > end) {    // 有一部分数据超出当前容量，截断
        r = end;
        data = data.substr(0, r - l + 1);
    }
    uint64_t lmin = l, rmax = r;
    for (auto &[ll, rr]: s) { // 遍历区间
        if ((ll <= rmax and rr >= rmax) or (rr >= lmin and ll <= lmin)) { //两个区间有重合
            lmin = min(lmin, ll);
            rmax = max(rmax, rr);
        }else if(ll>rmax)break;
    }
    string temp_s(rmax - lmin + 1, '0');
    uint64_t sum = rmax - lmin + 1;
    set<PII> temp = s;
    for (auto &[ll, rr]: temp) {
        if (ll >= lmin and rr <= rmax) {
            s.erase({ll, rr});
            mergeString(lmin, ll, rr, temp_s, mp[ll]);
            sum -= (rr - ll + 1);
            mp.erase(ll);
        }
    }
    s.insert({lmin, rmax});
    mergeString(lmin, l, r, temp_s, data);
    totalBytes += sum;
    mp[lmin] = temp_s;
    while (s.size() and (s.begin()->first == pushtotalBytes)) {
        st = pushtotalBytes;
        output.push(mp[st]);
        s.erase(s.begin());
        totalBytes -= mp[st].size();
        pushtotalBytes += mp[st].size();
        mp.erase(st);
    }
    if (isFinish and totalBytes == 0)output.close();
    return;
}

uint64_t Reassembler::bytes_pending() const {
    // Your code here.
    return totalBytes;
}

void Reassembler::mergeString(int lmin, int l, int r, std::string &dest, std::string &src) {
    for (int i = (l - lmin), j = 0; i <= (r - lmin); i++, j++) {
        dest[i] = src[j];
    }
}