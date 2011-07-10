/** @file spelling_corrector.h
 * @brief Spelling word group corrector.
 */
/* Copyright (C) 2011 Nikita Smetanin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_SPELLING_CORRECTOR_H
#define XAPIAN_INCLUDED_SPELLING_CORRECTOR_H

#include <vector>
#include <string>
#include "database.h"
#include "spelling_base.h"

class SpellingCorrector : public SpellingBase {

    static const unsigned LIMIT_CORRECTIONS = 5;
    static const unsigned MAX_GAP = 1;

    //Key for states memorisation
    struct word_spelling_key {
	unsigned first_word_index;
	unsigned first_spelling_index;
	unsigned second_word_index;
	unsigned second_spelling_index;

	bool operator<(const word_spelling_key& other) const
	{
	    return first_word_index < other.first_word_index || (first_word_index == other.first_word_index && (
		   first_spelling_index < other.first_spelling_index || (first_spelling_index == other.first_spelling_index && (
		   second_word_index < other.second_word_index || (second_word_index == other.second_word_index && (
		   second_spelling_index < other.second_spelling_index))))));
	}
    };

    void get_top_spelling_corrections(const std::string& word, unsigned top, bool use_freq,
				      std::vector<std::string>& result);

    unsigned get_spelling_freq(const std::vector<std::vector<std::string> >& words,
			       const std::vector<unsigned>& word_spelling,
			       std::map<word_spelling_key, unsigned>& freq_map, unsigned first_index,
			       unsigned second_index);

    void recursive_spelling_corrections(const std::vector<std::vector<std::string> >& words, unsigned word_index,
					std::vector<unsigned>& word_spelling, unsigned word_freq, std::map<
						word_spelling_key, unsigned>& freq_map,
					std::vector<unsigned>& max_spelling_word, unsigned& max_word_freq);

    unsigned max_edit_distance;

public:
    SpellingCorrector(const std::vector<Xapian::Internal::RefCntPtr<Xapian::Database::Internal> >& internal_,
                      unsigned max_edit_distance_) : SpellingBase(internal_), max_edit_distance(max_edit_distance_)
    {
    }

    unsigned get_spelling(const std::string& words, std::string& result);

    unsigned get_spelling(const std::vector<std::string>& words, std::vector<std::string>& result);
};

#endif // XAPIAN_INCLUDED_SPELLING_CORRECTOR_H
