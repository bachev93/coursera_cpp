#include "search_server.h"
#include "iterator_range.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>

vector<string> SplitIntoWords(const string& line) {
    istringstream words_input(line);
    return {istream_iterator<string>(words_input), istream_iterator<string>()};
}

SearchServer::SearchServer(istream& document_input) {
    UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(istream& document_input) {
    InvertedIndex new_index;

    for (string current_document; getline(document_input, current_document); ) {
        new_index.Add(move(current_document));
    }

    index = move(new_index);
}

void SearchServer::AddQueriesStream(
        istream& query_input, ostream& search_results_output) {
    const auto max_docs = 50000;
    vector<size_t> docid_count(max_docs);
    for (string current_query; getline(query_input, current_query); ) {
        size_t maxValidDocid{0};

        for(const auto& word: SplitIntoWords(current_query)) {
            for(const auto& [docid, qty] : index.Lookup(word)) {
                docid_count[docid] += qty;
                maxValidDocid = max(docid, maxValidDocid);
            }
        }

        vector<pair<size_t, size_t>> search_results;
        for(size_t i {0}, I {maxValidDocid + 1}; i < I; ++i) {
            if(docid_count[i] > 0) {
                search_results.emplace_back(i, docid_count[i]);
            }
        }

        auto middleIt = min(begin(search_results) + 5, end(search_results));
        partial_sort(begin(search_results), middleIt, end(search_results),
                     [](pair<size_t, size_t> lhs, pair<size_t, size_t> rhs) {
            int64_t lhs_docid = lhs.first;
            auto lhs_hit_count = lhs.second;
            int64_t rhs_docid = rhs.first;
            auto rhs_hit_count = rhs.second;
            return make_pair(lhs_hit_count, -lhs_docid) > make_pair(rhs_hit_count, -rhs_docid);
        }
        );

        search_results_output << current_query << ':';
        for (const auto& [docid, hitcount] : Head(search_results, 5)) {
            search_results_output << " {"
                                  << "docid: " << docid << ", "
                                  << "hitcount: " << hitcount << '}';
        }
        search_results_output << '\n';

        fill(begin(docid_count), end(docid_count), 0);
    }
}

void InvertedIndex::Add(string&& document) {
    docs.push_back(move(document));

    const size_t docid = docs.size() - 1;

    map<string, size_t> words_count;

    for(const auto& word : SplitIntoWords(docs.back())) {
        ++words_count[word];
    }

    for(const auto& [word, count] : words_count) {
        map_index[word].emplace_back(docid, count);
    }
}

const vector<pair<size_t, size_t>> InvertedIndex::Lookup(const string& word) const {
    if(map_index.count(word)) {
        return map_index.at(word);
    }
    return {};
}
