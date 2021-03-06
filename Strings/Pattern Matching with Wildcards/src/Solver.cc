//
// Created by whoami on 12/1/16.
//

#include <iostream>
#include <vector>
#include "Solver.h"

Solver::Solver(const std::string &pattern, const std::string &text)
    : pattern_(pattern),
      text_(text) {

  // First character that is different from '?'
  int left = 0;
  while (left < pattern.length()) {
    if (pattern_[left] != '?') {
      break;
    }
    left += 1;
  }

  // Last character that is different from '?'
  int right = static_cast<int>(pattern_.length()) - 1;
  while (right >= left) {
    if (pattern_[right] != '?') {
      break;
    }
    right -= 1;
  }

  pattern_begin_ = left;
  pattern_end_ = right;

  text_begin_ = left;
  text_end_ = right + static_cast<int>(text.length() - pattern.length());

  indices_ = GetIndices();

  for (int i = 0; i < indices_.size(); ++i) {
    dfa.Put(pattern_.substr(static_cast<size_t>(indices_[i].first),
                            static_cast<size_t>(indices_[i].second - indices_[i].first + 1)), i);
  }

  dfa.BuildSuffixLinks();
}

std::vector<int> Solver::GetEntries() {
  std::vector<int> result;

  // Pattern is empty
  if (pattern_end_ < pattern_begin_) {
    for (int i = 0; i <= text_end_ - text_begin_ + 1; ++i) {
      result.push_back(i);
    }

    return result;
  }

  // Text length must be always greater or equal to pattern length
  if (pattern_.length() > text_.length()) {
    return result;
  }

  // Auxiliary array to restore all entries
  std::vector<int> count(text_.length());

  for (int i = text_begin_; i <= text_end_; ++i) {

    while (dfa.GetTraverser() != dfa.GetRoot() ||
        dfa.GetTraverser()->children.find(text_[i]) != dfa.GetTraverser()->children.end()) {

      if (dfa.GetTraverser()->children.find(text_[i]) == dfa.GetTraverser()->children.end()) {
        dfa.MoveOnSuffixLink();
        continue;
      }

      dfa.MoveOnLetter(text_[i]);

      std::shared_ptr<Node> visitor = dfa.GetTraverser();
      while (visitor != dfa.GetRoot()) {

        for (auto &id : visitor->ids) {

          if (i - text_begin_ - indices_[id].second >= 0) {
            count[i - text_begin_ - indices_[id].second] += 1;
          }
        }

        visitor = visitor->compressed_suffix_link;
      }

      break;
    }
  }

  for (auto& elem : count) {
    if (elem == indices_.size()) {
      result.push_back(static_cast<int>(&elem - &count[0]));
    }
  }

  return result;
}

std::vector<std::pair<int, int>> Solver::GetIndices() {
  std::vector<std::pair<int, int>> result;

  bool previous_was_question = true;
  int last_question = 0;

  for (int i = pattern_begin_; i <= pattern_end_; ++i) {
    if (pattern_[i] != '?') {
      if (previous_was_question) {
        last_question = i;
      }
      previous_was_question = false;
    } else {
      if (!previous_was_question) {
        result.push_back(std::make_pair(last_question, i - 1));
      }
      previous_was_question = true;
    }
  }

  if (!previous_was_question) {
    result.push_back(std::make_pair(last_question, pattern_end_));
  }

  return result;
}