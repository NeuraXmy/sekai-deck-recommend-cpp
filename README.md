# Sekai Deck Recommendation C++

A C++ optimized version of [sekai-calculator](https://github.com/xfl03/sekai-calculator) with Python bindings, providing both the original brute-force search algorithm and some new randomized algorithms.

## Install from PyPI

```
pip install sekai-deck-recommend-cpp
```

## Install from source

### Prerequisites

- CMake ≥ 3.15
- C++20 compatible compiler (GCC/Clang/MSVC)
- Python 3.10+ with development headers

### Steps

```bash
# Clone with submodules
git clone --recursive https://github.com/NeuraXmy/sekai-deck-recommend-cpp.git
cd sekai-deck-recommend-cpp

# Install via pip
pip install -e . -v
```

## Usage

```python
from sekai_deck_recommend_cpp import (
    SekaiDeckRecommend, 
    DeckRecommendOptions,
    DeckRecommendCardConfig
)
   
sekai_deck_recommend = SekaiDeckRecommend()

sekai_deck_recommend.update_masterdata("base/dir/of/masterdata", "jp")
sekai_deck_recommend.update_musicmetas("file/path/of/musicmetas.json", "jp")

options = DeckRecommendOptions()

# optimizing target in ["score", "power", "skill", "bonus"], default is "score"
options.target = "score"

# "ga" for genetic algorithm, "dfs" for brute-force search
# default is "ga"
options.algorithm = "ga"   

options.region = "jp"
options.user_data_file_path = "user/data/file/path.json"
options.live_type = "multi"
options.music_id = 74
options.music_diff = "expert"
options.event_id = 160

result = sekai_deck_recommend.recommend(options)
```

For more details of options, please refer the docstring of `sekai_deck_recommend.DeckRecommendOptions`

## Acknowledgments
- Original implementation by [xfl03/sekai-calculator](https://github.com/xfl03/sekai-calculator)
- JSON parsing by [nlohmann/json](https://github.com/nlohmann/json)
- Python bindings powered by [pybind11](https://github.com/pybind/pybind11)
