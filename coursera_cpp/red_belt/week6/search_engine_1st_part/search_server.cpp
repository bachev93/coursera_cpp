#include "search_server.h"
#include "iterator_range.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>


vector<string_view> SplitIntoWords(string_view line) {
    vector<string_view> result;

    size_t pos = line.find_first_not_of(' ');
    line.remove_prefix(pos);
    while(pos != line.npos) {
        pos = line.find(' ');
        result.push_back(line.substr(0, pos));
        pos = line.find_first_not_of(' ', pos);
        line.remove_prefix(pos);
    }

    return result;
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
    vector<size_t> docid_count(index.getDocsSize());

    for (string current_query; getline(query_input, current_query); ) {
        for(const auto& word: SplitIntoWords(current_query)) {
            for(const auto& [docid, qty] : index.Lookup(word)) {
                docid_count[docid] += qty;
            }
        }

        vector<Entry> search_results(docid_count.size());
        for(size_t i = 0, I = search_results.size(); i < I; ++i) {
            search_results[i] = {i, docid_count[i]};
        }

        auto middleIt = Head(search_results, 5).end();
        partial_sort(begin(search_results), middleIt, end(search_results),
                     [](const Entry& lhs, const Entry& rhs) {
            int64_t lhs_docid = lhs.docID_;
            int64_t rhs_docid = rhs.docID_;
            return make_pair(lhs.hitcount_, -lhs_docid) > make_pair(rhs.hitcount_, -rhs_docid);});

        search_results_output << current_query << ':';
        for (const auto& [docid, hitcount] : Head(search_results, 5)) {
            if(hitcount == 0) {
                break;
            }

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

    map<string_view, size_t> words_count;

    for(const auto& word : SplitIntoWords(docs.back())) {
        ++words_count[word];
    }

    for(const auto& [word, hitcount] : words_count) {
        index_[word].push_back({docid, hitcount});
    }
}

const vector<Entry>& InvertedIndex::Lookup(const string_view& word) const {
    static const vector<Entry> empty;

    if(auto it = index_.find(word); it != index_.end()) {
        return it->second;
    }

    return empty;
}

size_t InvertedIndex::getDocsSize() const {
    return docs.size();
}
