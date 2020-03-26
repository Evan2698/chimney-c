#ifndef FUCN_H_328E88QE88DJASDJFHASNGLCNGJA_OEUEUR
#define FUCN_H_328E88QE88DJASDJFHASNGLCNGJA_OEUEUR
#include <string>
template <typename T>
std::string ToHexEX(T begin, T end)
{
	static const char *chars = "0123456789ABCDEF";
	std::string hex;
	for (auto it = begin; it != end; ++it)
	{
		unsigned char i = *it;
		hex.push_back(chars[(i >> 4) & 0x0F]);
		hex.push_back(chars[i & 0x0F]);	
        hex.append(" ");
	}
	return hex;
}

#endif