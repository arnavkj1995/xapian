/** @file spelling_corrector.cc
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

#include <config.h>
#include <limits>
#include <vector>
#include <map>
#include <algorithm>
#include <xapian/unicode.h>

#include "database.h"
#include "spelling_corrector.h"
#include "autoptr.h"
#include "ortermlist.h"
#include "editdistance.h"
#include "extended_edit_distance.h"

using namespace std;
using namespace Xapian;

SpellingCorrector::SpellingCorrector(const std::vector<
	Xapian::Internal::RefCntPtr<Database::Internal> >& internal_) :
    internal(internal_)
{
}

unsigned SpellingCorrector::request_internal(const string& first_word,
					     const string& second_word)
{
    unsigned freq = 0;

    for (size_t i = 0; i < internal.size(); ++i)
	freq += internal[i]->get_spellings_frequency(first_word, second_word);

    if (freq > 0)
	return freq * 2;

    for (size_t i = 0; i < internal.size(); ++i)
	freq += internal[i]->get_spelling_frequency(first_word)
		+ internal[i]->get_spelling_frequency(second_word);

    if (freq > 0)
	return max(freq / 32, 1u);
    return 0;
}

void SpellingCorrector::get_top_spelling_corrections(
						     const string& word,
						     unsigned max_edit_distance,
						     unsigned top, vector<
							     string>& result)
{
    AutoPtr<TermList> merger;
    for (size_t i = 0; i < internal.size(); ++i)
    {
	TermList* term_list = internal[i]->open_spelling_termlist_max(word, max_edit_distance);
	if (term_list != 0)
	{
	    if (merger.get() != 0)
	    {
		merger.reset(new OrTermList(merger.release(), term_list));
	    }
	    else merger.reset(term_list);
	}
    }

    if (merger.get() == 0)
    return;

    vector<unsigned> word_utf((Utf8Iterator(word)), Utf8Iterator());
    vector<unsigned> term_utf;
    multimap<double, string> top_spelling;

    ExtendedEditDistance edit_distance;

    while (true)
    {
	TermList* return_term_list = merger->next();
	if (return_term_list != 0)
	merger.reset(return_term_list);
	if (merger->at_end())
	break;

	string term = merger->get_termname();
	term_utf.assign((Utf8Iterator(term)), Utf8Iterator());

	if (unsigned(abs(long(term_utf.size()) - long(word_utf.size()))) > max_edit_distance)
	    continue;

	unsigned distance = edit_distance_unsigned(&term_utf[0], int(term_utf.size()), &word_utf[0],
		int(word_utf.size()), max_edit_distance);

	if (distance <= max_edit_distance)
	{
	    double distance_precise = edit_distance.edit_distance(&term_utf[0], term_utf.size(), &word_utf[0],
		    word_utf.size(), distance);

	    top_spelling.insert(make_pair(distance_precise, term));

	    if (top_spelling.size() > top)
	    top_spelling.erase(--top_spelling.end());
	}
    }

    result.clear();
    result.reserve(top_spelling.size());
    for (multimap<double, string>::const_iterator it = top_spelling.begin(); it != top_spelling.end(); ++it)
    result.push_back(it->second);

    if (result.empty())
    result.push_back(word);
}

unsigned SpellingCorrector::get_spelling_freq(
					      const vector<vector<string> >& words,
					      const vector<unsigned>& word_spelling,
					      map<word_spelling_key, unsigned>& freq_map,
					      unsigned first_index,
					      unsigned second_index)
{
    word_spelling_key key;
    key.first_word_index = first_index;
    key.first_spelling_index = word_spelling[first_index];
    key.second_word_index = second_index;
    key.second_spelling_index = word_spelling[second_index];

    map<word_spelling_key, unsigned>::const_iterator it = freq_map.find(key);
    if (it == freq_map.end()) {
	unsigned
		freq =
			request_internal(
					 words[first_index][word_spelling[first_index]],
					 words[second_index][word_spelling[second_index]]);
	it = freq_map.insert(make_pair(key, freq)).first;
    }
    return it->second;
}

void SpellingCorrector::recursive_spelling_corrections(
						       const vector<vector<
							       string> >& words,
						       unsigned word_index,
						       vector<unsigned>& word_spelling,
						       unsigned word_freq,
						       map<word_spelling_key,
							       unsigned>& freq_map,
						       vector<unsigned>& max_spelling_word,
						       unsigned& max_word_freq)
{
    if (word_index < words.size()) {
	for (unsigned spelling_index = 0; spelling_index
		< words[word_index].size(); ++spelling_index) {
	    word_spelling[word_index] = spelling_index;

	    if (word_index > 0) {
		word_freq += get_spelling_freq(words, word_spelling, freq_map,
					       word_index - 1, word_index);
		if (word_index > 1)
		    word_freq += get_spelling_freq(words, word_spelling,
						   freq_map, word_index - 2,
						   word_index);
	    }
	    recursive_spelling_corrections(words, word_index + 1,
					   word_spelling, word_freq, freq_map,
					   max_spelling_word, max_word_freq);
	}
    } else if (word_freq > max_word_freq) {
	max_word_freq = word_freq;
	max_spelling_word = word_spelling;
    }
}

unsigned SpellingCorrector::get_spelling_corrected(const vector<string>& words,
						   unsigned max_edit_distance,
						   vector<string>& result)
{
    vector<vector<string> > word_corrections(words.size());
    for (unsigned i = 0; i < words.size(); ++i)
	get_top_spelling_corrections(words[i], max_edit_distance,
				     LIMIT_CORRECTIONS, word_corrections[i]);

    unsigned max_word_freq = 0;
    vector<unsigned> max_spelling_word(words.size(), 0);

    vector<unsigned> word_spellings(words.size(), 0);
    map<word_spelling_key, unsigned> freq_map;

    recursive_spelling_corrections(word_corrections, 0, word_spellings, 0,
				   freq_map, max_spelling_word, max_word_freq);

    result.clear();
    result.reserve(word_corrections.size());
    for (unsigned i = 0; i < word_corrections.size(); ++i)
	result.push_back(word_corrections[i][max_spelling_word[i]]);
    return max_word_freq;
}
