#pragma once
#include "string"

class BitSet
{
public:
	struct BitSetInfo
	{
		const std::size_t max_count = 0;
		const int flag_on_count = 0;
		const double fragment_rate = 0.0;
	};

	BitSet(std::size_t size) 
		: m_size(size), m_bitset((size + 7) / 8, 0) 
	{
	}

	bool setBit(std::size_t pos, bool value)
	{
		if (pos >= m_size)
			return false;
		
		std::size_t byteIndex = pos / 8;
		std::size_t bitIndex = pos % 8;
		if (value)
		{
			m_bitset[byteIndex] |= (1 << bitIndex);
		}
		else
		{
			m_bitset[byteIndex] &= ~(1 << bitIndex);
		}

		return true;
	}

	bool setBit(std::size_t begin_pos, std::size_t end_pos, bool value)
	{
		if (begin_pos >= m_size || end_pos >= m_size || begin_pos > end_pos)
			return false;

		for (std::size_t pos = begin_pos; pos <= end_pos; ++pos)
		{
			setBit(pos, value);
		}

		return true;
	}

	bool getBit(std::size_t pos) const
	{
		if (pos >= m_size)
			return false;
		
		std::size_t byteIndex = pos / 8;
		std::size_t bitIndex = pos % 8;
		return (m_bitset[byteIndex] & (1 << bitIndex)) != 0;		
	}

	std::size_t getRelaySizePos(std::size_t size, bool value) const
	{
		std::size_t relay_bit_begin_pos = 0;
		std::size_t relay_bit_size = 0;
		for (std::size_t pos = 0; pos <= m_size; ++pos)
		{
			if (value != getBit(pos))
			{
				relay_bit_begin_pos = pos;
				relay_bit_size = 0;
			}	
			else
			{
				if (relay_bit_size == 0)
				{
					relay_bit_begin_pos = pos;
				}

				++relay_bit_size;
			}

			if (relay_bit_size == size)
			{
				return relay_bit_begin_pos;
			}
		}

		return -1;
	}

	BitSetInfo getBitSetInfo() const
	{
		if (m_size == 0)
			return {};

		int segment_count = 0;
		int flag_on_count = 0;
		bool current_bit = getBit(0);

		for (std::size_t i = 1; i < m_size; ++i)
		{
			bool bit = getBit(i);
			if (bit != current_bit)
			{
				++segment_count;
				current_bit = !current_bit;
			}

			if (bit == true)
			{
				++flag_on_count;
			}
		}

		++segment_count;

		return { m_size, flag_on_count, (static_cast<double>(segment_count) / m_size) };
	}

private:	
	const std::size_t m_size;
	std::vector<BYTE> m_bitset;
};