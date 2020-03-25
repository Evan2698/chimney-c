#ifndef FUCN_H_328E88QE88DJASDJFHASNGLCNGJA_OEUEUR
#define FUCN_H_328E88QE88DJASDJFHASNGLCNGJA_OEUEUR
#include <string>
#include <strstream>
/*template <typename T>
std::string ToHex(T begin, T end)
{
	static const char *chars = "0123456789ABCDEF";
	std::stringstream hex;
	for (auto it = begin; it != end; ++it)
	{
		short i = *it;
		hex << chars[(i >> 4) & 0x0F];
		hex << chars[i & 0x0F];
	}

	return hex.str();
}

*/
template <typename T>
std::string ToHexEX(T begin, T end)
{
	/*static const char *chars = "0123456789ABCDEF";
	std::stringstream hex;
	for (auto it = begin; it != end; ++it)
	{
		short i = *it;
		hex << chars[(i >> 4) & 0x0F];
		hex << chars[i & 0x0F];
        hex << " ";
	}

	return hex.str();*/
	return std::string("");
}

#endif