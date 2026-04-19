/**
 * @brief World-owned score snapshot for HUD / completion summaries.
 * @note Each WorldBase subclass builds this; the UI reads it from the active world.
 **/
#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace cse498 {

struct WorldScoreDisplay {
  /// Optional headline (e.g. completion banner).
  std::optional<std::string> headline;
  /// Rows for the score panel: (label, value). Empty label means value is a full line.
  std::vector<std::pair<std::string, std::string>> lines;
  /// When set, show a numeric score in the HUD (StubWorld: in progress + completed).
  std::optional<int> numeric_score;
  /// If true, label the numeric line "Final score"; otherwise "Score".
  bool numeric_score_is_final = false;
};

}  // namespace cse498
