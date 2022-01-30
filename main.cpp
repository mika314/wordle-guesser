#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

auto score(const auto &word, const auto &charsState, const auto &charsCount)
{
  auto s = [&]() {
    auto ret = std::unordered_set<char>{};
    for (auto ch : word)
      ret.insert(ch);
    return ret;
  }();

  return std::accumulate(std::begin(s), std::end(s), 0, [&](auto sum, auto ch) {
    const auto stateIt = charsState.find(ch);
    if (stateIt != std::end(charsState))
      return sum - 1000;
    const auto countIt = charsCount.find(ch);
    if (countIt == std::end(charsCount))
      return sum;
    return sum + countIt->second;
  });
}

enum class State { grey, yellow, green };

auto updateCharsState(auto &charsState, const auto &randWord, const auto &guess)
{
  auto idx = -1;
  for (const auto ch : guess)
  {
    ++idx;
    if (ch == randWord[idx])
      charsState[ch] = std::pair{State::green, idx};
    else if (std::find(std::begin(randWord), std::end(randWord), ch) != std::end(randWord))
    {
      if (charsState.find(ch) == std::end(charsState))
        charsState[ch] = std::pair{State::yellow, idx};
    }
    else
      charsState[ch] = std::pair{State::grey, -1};
  }
}
auto updateCharsStateIo(auto &charsState, const auto &guess)
{
  for (;;)
  {
    std::string ans;
    std::cout << "\n>";
    std::cin >> ans;
    if (ans == "pass")
      return;
    if (ans.size() < 5)
      continue;
    auto idx = -1;
    for (const auto ch : ans)
    {
      ++idx;
      switch (ch)
      {
      case 'x': charsState[guess[idx]] = std::pair{State::grey, -1}; break;
      case 'o': charsState[guess[idx]] = std::pair{State::green, idx}; break;
      case '.':
        if (charsState.find(guess[idx]) == std::end(charsState))
          charsState[guess[idx]] = std::pair{State::yellow, idx};
        break;
      default: continue;
      }
    }
    break;
  }
}

auto isMatch(const auto &charsState, const auto &w)
{
  for (auto s : charsState)
  {
    const auto [ch, statePos] = s;
    const auto [state, pos] = statePos;
    switch (state)
    {
    case State::grey:
      if (std::find(std::begin(w), std::end(w), ch) != std::end(w))
        return false;
      break;
    case State::yellow:
      if (std::find(std::begin(w), std::end(w), ch) == std::end(w))
        return false;
      else
      {
        if (w[pos] == ch)
          return false;
      }
      break;
    case State::green:
      if (w[pos] != ch)
        return false;
      break;
    }
  }
  return true;
}

auto main() -> int
{
  const auto words = [&]() {
    auto f = std::ifstream{"words_alpha.txt"};
    auto line = std::string{};
    auto ret = std::vector<std::string>{};
    while (std::getline(f, line))
    {
      if (line.size() != 5)
        continue;
      ret.push_back(line);
    }
    return ret;
  }();

  const auto charsCount = [&]() {
    auto ret = std::unordered_map<char, int>{};
    for (const auto &w : words)
      for (auto ch : w)
        ++ret[ch];
    return ret;
  }();

  srand(time(nullptr));
  const auto randWord = words[rand() % words.size()];
  std::cout << "random word: " << randWord << std::endl;

  auto charsState = std::unordered_map<char, std::pair<State, int>>{};
  auto guessedWords = std::unordered_set<std::string>{};

  // charsState.emplace('s', std::pair{State::grey, -1});
  // charsState.emplace('a', std::pair{State::grey, -1});
  // charsState.emplace('e', std::pair{State::yellow, 2});
  // charsState.emplace('r', std::pair{State::green, 1});
  // charsState.emplace('o', std::pair{State::green, 0});
  //
  // isMatch(charsState, "orlet");

  for (;;)
  {
    if (false)
    {
      std::cout << "chars state\n";
      for (auto s : charsState)
      {
        const auto [ch, statePos] = s;
        const auto [state, pos] = statePos;
        std::cout << ch << ": ";
        switch (state)
        {
        case State::grey: std::cout << "grey"; break;
        case State::yellow: std::cout << "yellow " << pos; break;
        case State::green: std::cout << "green " << pos; break;
        }
        std::cout << std::endl;
      }
    }
    const auto possibleWords = [&]() {
      auto ret = std::vector<std::string>{};
      for (const auto &w : words)
        if (guessedWords.find(w) == std::end(guessedWords) && isMatch(charsState, w))
          ret.push_back(w);
      return ret;
    }();
    std::cout << "Possible words size: " << possibleWords.size() << std::endl;

    if (possibleWords.size() <= 1)
    {
      if (!possibleWords.empty())
      {
        std::cout << "#" << guessedWords.size() + 1 << " final guess: " << possibleWords.front() << " which is ";
        if (possibleWords.front() == randWord)
          std::cout << "correct\n";
        else
          std::cout << "incorrect\n";
      }
      else
        std::cout << "we run out of guesses\n";
      break;
    }

    auto allowedWords = words;
    std::sort(std::begin(allowedWords), std::end(allowedWords), [&](const auto &w1, const auto &w2) {
      const auto score1 = score(w1, charsState, charsCount);
      const auto score2 = score(w2, charsState, charsCount);
      return score1 < score2;
    });
    const auto guess = [&]() {
      auto ret = allowedWords.back();
      while (guessedWords.find(ret) != std::end(guessedWords))
      {
        allowedWords.pop_back();
        ret = allowedWords.back();
      }
      if (score(ret, charsState, charsCount) < 5000 || guessedWords.size() >= 5 || possibleWords.size() + guessedWords.size() <= 6)
        return possibleWords.front();
      return ret;
    }();
    guessedWords.insert(guess);

    std::cout << "#" << guessedWords.size() << " Guess: " << guess /* << " score: " << score(guess, charsState, charsCount) */ << std::endl;
    if (guess == randWord)
      break;
    // updateCharsState(charsState, randWord, guess);
    updateCharsStateIo(charsState, guess);
  }
}
